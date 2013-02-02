//
// Copyright (c) 2005, 2006 Wei Mingzhi <whistler@openoffice.org>
// All Rights Reserved.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 2 of
// the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
// 02110-1301, USA
//

#ifndef BOT_H
#define BOT_H

#include "player.h"

#define MAX_MOVES 160

enum
{
   HAND_MAX = 0,
   HAND_MAXNUM,
   HAND_CARDS,
   HAND_RIBBONS,
   HAND_ANIMALS,
   HAND_RED_RIBBONS,
   HAND_BLUE_RIBBONS,
   HAND_NORMAL_RIBBONS,
   HAND_SAKECUP,
   HAND_BOAR,
   HAND_BIRD,
   HAND_LIGHTS,

   HAND_COUNT,
};

typedef struct move_s
{
   int handindex;
   int deskindex;
   int hand[HAND_COUNT];
   int opnhand[HAND_COUNT];
   int month;
} move_t;

class CBot : public CBasePlayer
{
public:
   CBot();
   ~CBot();

   bool      IsBot()               { return true; }
   int       SelectCard();
   bool      WantToContinue();
   int       SelectCardOnDesk(int month, const CCard &drawn);

private:
   void      AnalyzeMoves();
   void      AnalyzeHand(int *hand = NULL, int *opnhand = NULL);
   int       AnalyzeGoal();
   int       DiscardCard();

   int       NumMonthInHand(int month);
   int       NumMonthExposed(int month);
   int       NumMonthCaptured(int month);
   int       NumMonthInvisible(int month);

   bool      CardIsSafe(const CCard &c);
   int       CardIsDangerous(const CCard &c);

   move_t    m_PossibleMoves[MAX_MOVES];
   int       m_iNumPossibleMove;

   int       m_rgHandPercent[HAND_COUNT];
   int       m_rgOpnHandPercent[HAND_COUNT];

   int       m_iWantedMove;
};

#endif

