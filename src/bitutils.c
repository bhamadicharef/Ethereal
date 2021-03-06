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

const int LsbTable[64] = {
    0, 47,  1, 56, 48, 27,  2, 60,
   57, 49, 41, 37, 28, 16,  3, 61,
   54, 58, 35, 52, 50, 42, 21, 44,
   38, 32, 29, 23, 17, 11,  4, 62,
   46, 55, 26, 59, 40, 36, 15, 53,
   34, 51, 20, 43, 31, 22, 10, 45,
   25, 39, 14, 33, 19, 30,  9, 24,
   13, 18,  8, 12,  7,  6,  5, 63
};

int BitCounts[0x10000];

/**
 * Count the number of set bits in a given 64-bit Integer
 *
 * @param   bb  BitBoard to count set bits in
 * @return      Count of all set bits in bb
 */
int countSetBits(uint64_t bb){
    
    int count = 0;
    
    while (bb){
        bb ^= 1ull << getLSB(bb);
        count += 1;
    }
    
    return count;
}

/**
 * Count the number of set bits in a given 64-bit Integer.
 * Faster than countSetBits, but requires BitCounts first
 * be initialized using countsetBits.
 *
 * @param   bb  BitBoard to count set bits in
 * @return      Count of all set bits in bb
 */
int popcount(uint64_t bb){
    return  BitCounts[bb >>  0 & 0xFFFF]
          + BitCounts[bb >> 16 & 0xFFFF]
          + BitCounts[bb >> 32 & 0xFFFF]
          + BitCounts[bb >> 48 & 0xFFFF];
}

/**
 * Fill an array with the bit indexes of all set bits
 * in a given 64-bit Integer. Set the array index after
 * the last bit index location to -1 to indicate that
 * all bit indexes have been traversed
 *
 * @param   bb  BitBoard to get set bits in
 * @param   arr Integer Array to fill with bit indexes
 */
void getSetBits(uint64_t bb, int * arr){
    
    int lsb, count = 0;
    
    while (bb){
        lsb = getLSB(bb);
        arr[count++] = lsb;
        bb ^= 1ull << lsb;
    }
    
    arr[count] = -1;
}