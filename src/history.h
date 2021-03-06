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

#ifndef _HISTORY_H
#define _HISTORY_H

#define HISTORY_GOOD    (0)
#define HISTORY_TOTAL   (1)

#define HISTORY_MAX     (0x7FFF)

void clearHistory(HistoryTable history);

void updateHistory(HistoryTable history, uint16_t move, int colour, int isGood,
                                                                    int delta);

int getHistoryScore(HistoryTable history, uint16_t move, int colour,
                                                        int factor);

#endif