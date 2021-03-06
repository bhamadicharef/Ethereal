/*
  Ethereal is a UCI chess playing engine authored by Andrew Grant.
  <https://github.com/AndyGrant/Ethereal>     <andrew@grantnet.us>
  
  Ethereal is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.
  
  Ethereal is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
  
  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdint.h>
#include <assert.h>

#include "bitboards.h"
#include "bitutils.h"
#include "castle.h"
#include "magics.h"
#include "move.h"
#include "movegen.h"
#include "piece.h"
#include "types.h"

/**
 * Generate all of the legal moves for a given board. The moves will
 * be stored in a pointer passed to the function, and an integer 
 * pointer passed to the function will be updated to reflect the
 * total number of psuedo legal moves found.
 *
 * @param   board   Board pointer with the current position
 * @param   moves   Destination for the found legal moves
 * @param   size    Pointer to keep track of the number of moves
 */
void genAllLegalMoves(Board * board, uint16_t * moves, int * size){
    
    Undo undo[1];
    int i, psuedoSize = 0;    
    uint16_t psuedoMoves[MAX_MOVES];
    
    genAllMoves(board, psuedoMoves, &psuedoSize);
    
    // Copy over moves from psuedoMoves that are proven to be legal
    for (i = 0; i < psuedoSize; i++){
        applyMove(board, psuedoMoves[i], undo);
        if (isNotInCheck(board, !board->turn))
            moves[(*size)++] = psuedoMoves[i];
        revertMove(board, psuedoMoves[i], undo);
    }
}

/**
 * Generate all of the psudeo legal moves for a given board.
 * The moves will be stored in a pointer passed to the function,
 * and an integer pointer passed to the function will be updated
 * to reflect the total number of psuedo legal moves found.
 *
 * @param   board   Board pointer with the current position
 * @param   moves   Destination for the found psuedo legal moves
 * @param   size    Pointer to keep track of the number of moves
 */
void genAllMoves(Board * board, uint16_t * moves, int * size){
     
    uint64_t pawnForwardOne;
    uint64_t pawnForwardTwo;
    uint64_t pawnLeft;
    uint64_t pawnRight;
    
    uint64_t pawnPromoForward;
    uint64_t pawnPromoLeft;
    uint64_t pawnPromoRight;
    
    int bit, lsb;
    int forwardShift, leftShift, rightShift;
    int epSquare = board->epSquare;
    
    uint64_t friendly = board->colours[board->turn];
    uint64_t enemy = board->colours[!board->turn];
    
    uint64_t empty = ~(friendly | enemy);
    uint64_t notEmpty = ~empty;
    uint64_t notFriendly = ~friendly;
    uint64_t attackable;
    
    uint64_t myPawns   = friendly & board->pieces[PAWN];
    uint64_t myKnights = friendly & board->pieces[KNIGHT];
    uint64_t myBishops = friendly & board->pieces[BISHOP];
    uint64_t myRooks   = friendly & board->pieces[ROOK];
    uint64_t myQueens  = friendly & board->pieces[QUEEN];
    uint64_t myKings   = friendly & board->pieces[KING];
    
    // Generate the queens' moves as if they were rooks and bishops
    myBishops |= myQueens; myRooks |= myQueens;
    
    // DEFINE PAWN BITBOARDS AND FIND ENPASS MOVES
    if (board->turn == WHITE){
        forwardShift = -8;
        leftShift = -7;
        rightShift = -9;
        
        pawnForwardOne = (myPawns << 8) & empty;
        pawnForwardTwo = ((pawnForwardOne & RANK_3) << 8) & empty;
        pawnLeft = ((myPawns << 7) & (~FILE_H)) & enemy;
        pawnRight = ((myPawns << 9) & (~FILE_A)) & enemy;
        
        pawnPromoForward = pawnForwardOne & RANK_8;
        pawnPromoLeft = pawnLeft & RANK_8;
        pawnPromoRight = pawnRight & RANK_8;
        
        pawnForwardOne &= ~RANK_8;
        pawnLeft &= ~RANK_8;
        pawnRight &= ~RANK_8;
        
        if(epSquare != -1){
            if (board->squares[epSquare - 7] == WHITE_PAWN && epSquare != 47)
                moves[(*size)++] = MoveMake(epSquare-7, epSquare, ENPASS_MOVE);
            
            if (board->squares[epSquare - 9] == WHITE_PAWN && epSquare != 40)
                moves[(*size)++] = MoveMake(epSquare-9, epSquare, ENPASS_MOVE);
        }
        
    } else {
        forwardShift = 8;
        leftShift = 7;
        rightShift = 9;
        
        pawnForwardOne = (myPawns >> 8) & empty;
        pawnForwardTwo = ((pawnForwardOne & RANK_6) >> 8) & empty;
        pawnLeft = ((myPawns >> 7) & (~FILE_A)) & enemy;
        pawnRight = ((myPawns >> 9) & (~FILE_H)) & enemy;
        
        pawnPromoForward = pawnForwardOne & RANK_1;
        pawnPromoLeft = pawnLeft & RANK_1;
        pawnPromoRight = pawnRight & RANK_1;
        
        pawnForwardOne &= ~RANK_1;
        pawnLeft &= ~RANK_1;
        pawnRight &= ~RANK_1;
        
        if(epSquare != -1){
            if (board->squares[epSquare + 7] == BLACK_PAWN && epSquare != 16)
                moves[(*size)++] = MoveMake(epSquare+7, epSquare, ENPASS_MOVE);
            
            if (board->squares[epSquare + 9] == BLACK_PAWN && epSquare != 23)
                moves[(*size)++] = MoveMake(epSquare+9, epSquare, ENPASS_MOVE);
        }
    }
    
    // Generate all Pawn moves aside from Promotions and Enpass
    buildPawnMoves(moves, size, pawnForwardOne, forwardShift);
    buildPawnMoves(moves, size, pawnForwardTwo, (2*forwardShift));
    buildPawnMoves(moves, size, pawnLeft, leftShift);
    buildPawnMoves(moves, size, pawnRight, rightShift);
    
    // Generate all Pawn promotion moves
    buildPawnPromotions(moves, size, pawnPromoForward, forwardShift);
    buildPawnPromotions(moves, size, pawnPromoLeft, leftShift);
    buildPawnPromotions(moves, size, pawnPromoRight, rightShift);
    
    // Generate all moves for all non pawns aside from Castles
    buildKnightMoves(moves, size, myKnights, notFriendly);
    buildBishopAndQueenMoves(moves, size, myBishops, notEmpty, notFriendly);
    buildRookAndQueenMoves(moves, size, myRooks, notEmpty, notFriendly);
    buildKingMoves(moves, size, myKings, notFriendly);
    
    // GENERATE CASTLES
    if (isNotInCheck(board,board->turn)){
        if (board->turn == WHITE){
            
            // KING SIDE
            if ((notEmpty & WHITE_CASTLE_KING_SIDE_MAP) == 0)
                if (board->castleRights & WHITE_KING_RIGHTS)
                    if (!squareIsAttacked(board, WHITE, 5))
                        moves[(*size)++] = MoveMake(4, 6, CASTLE_MOVE);
                        
            // QUEEN SIDE
            if ((notEmpty & WHITE_CASTLE_QUEEN_SIDE_MAP) == 0)
                if (board->castleRights & WHITE_QUEEN_RIGHTS)
                    if (!squareIsAttacked(board, WHITE, 3))
                        moves[(*size)++] = MoveMake(4, 2, CASTLE_MOVE);
        }
        
        else {
            
            // KING SIDE
            if ((notEmpty & BLACK_CASTLE_KING_SIDE_MAP) == 0)
                if (board->castleRights & BLACK_KING_RIGHTS)
                    if (!squareIsAttacked(board, BLACK, 61))
                        moves[(*size)++] = MoveMake(60, 62, CASTLE_MOVE);
                        
            // QUEEN SIDE
            if ((notEmpty & BLACK_CASTLE_QUEEN_SIDE_MAP) == 0)
                if (board->castleRights & BLACK_QUEEN_RIGHTS)
                    if (!squareIsAttacked(board, BLACK, 59))
                        moves[(*size)++] = MoveMake(60, 58, CASTLE_MOVE);
        }
    }
}


/**
 * Generate all of the psudeo legal moves which are considered
 * noisy, IE captures, promotions, enpassant, for a given board
 * The moves will be stored in a pointer passed to the function,
 * and an integer pointer passed to the function will be updated
 * to reflect the total number of psuedo legal noisy moves found.
 *
 * @param   board   Board pointer with the current position
 * @param   moves   Destination for the found psuedo legal moves
 * @param   size    Pointer to keep track of the number of moves
 */
void genAllNoisyMoves(Board * board, uint16_t * moves, int * size){
    
    uint64_t pawnLeft;
    uint64_t pawnRight;
    
    uint64_t pawnPromoForward;
    uint64_t pawnPromoLeft;
    uint64_t pawnPromoRight;
    
    int bit, lsb;
    int forwardShift, leftShift, rightShift;
    int epSquare = board->epSquare;
    
    uint64_t friendly = board->colours[board->turn];
    uint64_t enemy = board->colours[!board->turn];
    
    uint64_t empty = ~(friendly | enemy);
    uint64_t notEmpty = ~empty;
    uint64_t attackable;
    
    uint64_t myPawns   = friendly & board->pieces[PAWN];
    uint64_t myKnights = friendly & board->pieces[KNIGHT];
    uint64_t myBishops = friendly & board->pieces[BISHOP];
    uint64_t myRooks   = friendly & board->pieces[ROOK];
    uint64_t myQueens  = friendly & board->pieces[QUEEN];
    uint64_t myKings   = friendly & board->pieces[KING];
    
    // Generate queen moves as if they were rooks and bishops
    myBishops |= myQueens; myRooks |= myQueens;
    
    // Generate Pawn BitBoards and Generate Enpass Moves
    if (board->turn == WHITE){
        forwardShift = -8;
        leftShift = -7;
        rightShift = -9;
        
        pawnLeft = ((myPawns << 7) & ~FILE_H) & enemy;
        pawnRight = ((myPawns << 9) & ~FILE_A) & enemy;
        
        pawnPromoForward = (myPawns << 8) & empty & RANK_8;
        pawnPromoLeft = pawnLeft & RANK_8;
        pawnPromoRight = pawnRight & RANK_8;
        
        pawnLeft &= ~RANK_8;
        pawnRight &= ~RANK_8;
        
        if(epSquare != -1){
            if (board->squares[epSquare-7] == WHITE_PAWN && epSquare != 47)
                moves[(*size)++] = MoveMake(epSquare-7, epSquare, ENPASS_MOVE);
            
            if (board->squares[epSquare-9] == WHITE_PAWN && epSquare != 40)
                moves[(*size)++] = MoveMake(epSquare-9, epSquare, ENPASS_MOVE);
        }
        
    } else {
        forwardShift = 8;
        leftShift = 7;
        rightShift = 9;
        
        pawnLeft = ((myPawns >> 7) & ~FILE_A) & enemy;
        pawnRight = ((myPawns >> 9) & ~FILE_H) & enemy;
        
        pawnPromoForward = (myPawns >> 8) & empty & RANK_1;
        pawnPromoLeft = pawnLeft & RANK_1;
        pawnPromoRight = pawnRight & RANK_1;
        
        pawnLeft &= ~RANK_1;
        pawnRight &= ~RANK_1;
        
        if(epSquare != -1){
            if (board->squares[epSquare+7] == BLACK_PAWN && epSquare != 16)
                moves[(*size)++] = MoveMake(epSquare+7, epSquare, ENPASS_MOVE);
            
            if (board->squares[epSquare+9] == BLACK_PAWN && epSquare != 23)
                moves[(*size)++] = MoveMake(epSquare+9, epSquare, ENPASS_MOVE);
        }
    }
    
    // Generate all pawn captures that are not promotions
    buildPawnMoves(moves, size, pawnLeft, leftShift);
    buildPawnMoves(moves, size, pawnRight, rightShift);
    
    // Generate all pawn promotions
    buildPawnPromotions(moves, size, pawnPromoForward, forwardShift);
    buildPawnPromotions(moves, size, pawnPromoLeft, leftShift);
    buildPawnPromotions(moves, size, pawnPromoRight, rightShift);
    
    // Generate attacks for all non pawn pieces
    buildKnightMoves(moves, size, myKnights, enemy);
    buildBishopAndQueenMoves(moves, size, myBishops, notEmpty, enemy);
    buildRookAndQueenMoves(moves, size, myRooks, notEmpty, enemy);
    buildKingMoves(moves, size, myKings, enemy);
}


/**
 * Generate all of the psudeo legal moves which are considered
 * quiet, IE not captures, promotions, or enpassant, for a given board
 * The moves will be stored in a pointer passed to the function,
 * and an integer pointer passed to the function will be updated
 * to reflect the total number of psuedo legal quiet moves found.
 *
 * @param   board   Board pointer with the current position
 * @param   moves   Destination for the found psuedo legal moves
 * @param   size    Pointer to keep track of the number of moves
 */
void genAllQuietMoves(Board * board, uint16_t * moves, int * size){
     
    int bit, lsb;
    
    uint64_t pawnForwardOne;
    uint64_t pawnForwardTwo;
    
    uint64_t friendly = board->colours[board->turn];
    uint64_t enemy = board->colours[!board->turn];
    
    uint64_t empty = ~(friendly | enemy);
    uint64_t notEmpty = ~empty;
    uint64_t attackable;
    
    uint64_t myPawns   = friendly & board->pieces[PAWN];
    uint64_t myKnights = friendly & board->pieces[KNIGHT];
    uint64_t myBishops = friendly & board->pieces[BISHOP];
    uint64_t myRooks   = friendly & board->pieces[ROOK];
    uint64_t myQueens  = friendly & board->pieces[QUEEN];
    uint64_t myKings   = friendly & board->pieces[KING];
    
    // Generate the queens' moves as if they were rooks and bishops
    myBishops |= myQueens; myRooks |= myQueens;
    
    // Generate the pawn advances
    if (board->turn == WHITE){
        pawnForwardOne = (myPawns << 8) & empty & ~RANK_8;
        pawnForwardTwo = ((pawnForwardOne & RANK_3) << 8) & empty;
        buildPawnMoves(moves, size, pawnForwardOne, -8);
        buildPawnMoves(moves, size, pawnForwardTwo, -16);
    } 
    
    else {
        pawnForwardOne = (myPawns >> 8) & empty & ~RANK_1;
        pawnForwardTwo = ((pawnForwardOne & RANK_6) >> 8) & empty;
        buildPawnMoves(moves, size, pawnForwardOne, 8);
        buildPawnMoves(moves, size, pawnForwardTwo, 16);
    }
    
    // Generate all moves for all non pawns aside from Castles
    buildKnightMoves(moves, size, myKnights, empty);
    buildBishopAndQueenMoves(moves, size, myBishops, notEmpty, empty);
    buildRookAndQueenMoves(moves, size, myRooks, notEmpty, empty);
    buildKingMoves(moves, size, myKings, empty);
    
    // GENERATE CASTLES
    if (isNotInCheck(board,board->turn)){
        if (board->turn == WHITE){
            
            // KING SIDE
            if ((notEmpty & WHITE_CASTLE_KING_SIDE_MAP) == 0)
                if (board->castleRights & WHITE_KING_RIGHTS)
                    if (!squareIsAttacked(board, WHITE, 5))
                        moves[(*size)++] = MoveMake(4, 6, CASTLE_MOVE);
                        
            // QUEEN SIDE
            if ((notEmpty & WHITE_CASTLE_QUEEN_SIDE_MAP) == 0)
                if (board->castleRights & WHITE_QUEEN_RIGHTS)
                    if (!squareIsAttacked(board, WHITE, 3))
                        moves[(*size)++] = MoveMake(4, 2, CASTLE_MOVE);
        }
        
        else {
            
            // KING SIDE
            if ((notEmpty & BLACK_CASTLE_KING_SIDE_MAP) == 0)
                if (board->castleRights & BLACK_KING_RIGHTS)
                    if (!squareIsAttacked(board, BLACK, 61))
                        moves[(*size)++] = MoveMake(60, 62, CASTLE_MOVE);
                        
            // QUEEN SIDE
            if ((notEmpty & BLACK_CASTLE_QUEEN_SIDE_MAP) == 0)
                if (board->castleRights & BLACK_QUEEN_RIGHTS)
                    if (!squareIsAttacked(board, BLACK, 59))
                        moves[(*size)++] = MoveMake(60, 58, CASTLE_MOVE);
        }
    }
}

/**
 * Determine if the king of a certain colour is in check
 * on the board passed to the function
 *
 * @param   board   Board pointer to current position
 * @param   turn    Colour of the king to determine check status for
 *
 * @return          1 for not in check, 0 for in check
 */
int isNotInCheck(Board * board, int turn){
    int kingsq = getLSB(board->colours[turn] & board->pieces[KING]);
    assert(board->squares[kingsq] == WHITE_KING + turn);
    return !squareIsAttacked(board, turn, kingsq);
}

/**
 * Determine if the enemy pieces are able to attacked
 * a given square. This is used by isNotInCheck.
 *
 * @param   board   Board pointer for current position
 * @param   turn    Colour of friendly side
 * @param   sq      Square to be attacked
 *
 * @return          1 if can be attacked, 0 if cannot
 */
int squareIsAttacked(Board * board, int turn, int sq){
    
    uint64_t square;
    
    uint64_t friendly = board->colours[turn];
    uint64_t enemy = board->colours[!turn];
    uint64_t notEmpty = friendly | enemy;
    
    uint64_t enemyPawns   = enemy & board->pieces[PAWN];
    uint64_t enemyKnights = enemy & board->pieces[KNIGHT];
    uint64_t enemyBishops = enemy & board->pieces[BISHOP];
    uint64_t enemyRooks   = enemy & board->pieces[ROOK];
    uint64_t enemyQueens  = enemy & board->pieces[QUEEN];
    uint64_t enemyKings   = enemy & board->pieces[KING];
    
    enemyBishops |= enemyQueens;
    enemyRooks |= enemyQueens;
    square = (1ull << sq);
    
    // Pawns
    if (turn == WHITE){
        if (((square << 7 & ~FILE_H) | (square << 9 & ~FILE_A)) & enemyPawns)
            return 1;
    } else {
        if (((square >> 7 & ~FILE_A) | (square >> 9 & ~FILE_H)) & enemyPawns)
            return 1;
    }
    
    // Knights
    if (enemyKnights && KnightAttacks(sq, enemyKnights)) return 1;
    
    // Bishops and Queens
    if (enemyBishops && BishopAttacks(sq, notEmpty, enemyBishops)) return 1;
    
    // Rooks and Queens
    if (enemyRooks && RookAttacks(sq, notEmpty, enemyRooks)) return 1;
    
    // King
    if (KingAttacks(sq, enemyKings)) return 1;
    
    return 0;
}