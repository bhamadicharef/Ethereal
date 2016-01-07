#ifndef _BITBOARDS_H
#define _BITBOARDS_H

#include <stdint.h>

uint64_t WhiteCastleKingSide  = (1ull <<  5) + (1ull <<  6);
uint64_t BlackCastleKingSide  = (1ull << 61) + (1ull << 62);
uint64_t WhiteCastleQueenSide = (1ull <<  1) + (1ull <<  2) + (1ull << 3);
uint64_t BlackCastleQueenSide = (1ull << 57) + (1ull << 58) + (1ull << 59);

uint64_t RANK_8 = 0xFF00000000000000;
uint64_t RANK_7 = 0x00FF000000000000;
uint64_t RANK_6 = 0x0000FF0000000000;
uint64_t RANK_5 = 0x000000FF00000000;
uint64_t RANK_4 = 0x00000000FF000000;
uint64_t RANK_3 = 0x0000000000FF0000;
uint64_t RANK_2 = 0x000000000000FF00;
uint64_t RANK_1 = 0x00000000000000FF;

uint64_t FILE_A = 9259542123273814144ull;
uint64_t FILE_B = 4629771061636907072ull;
uint64_t FILE_C = 2314885530818453536ull;
uint64_t FILE_D = 1157442765409226768ull;
uint64_t FILE_E = 578721382704613384ull;
uint64_t FILE_F = 289360691352306692ull;
uint64_t FILE_G = 144680345676153346ull;
uint64_t FILE_H = 72340172838076673ull;

#endif