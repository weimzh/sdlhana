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

#include "main.h"

CBot::CBot()
{
}

CBot::~CBot()
{
}

int CBot::SelectCard()
{
   AnalyzeHand();
   AnalyzeMoves();

   // if we have no possible move, then discard a card
   if (m_iNumPossibleMove <= 0) {
      return DiscardCard();
   }

   int i, j, index = -1, handtype = HAND_MAXNUM + 1, prev = -1;

   for (i = 0; i < m_iNumPossibleMove; i++) {
      for (j = handtype; j < HAND_COUNT; j++) {
         if (m_PossibleMoves[i].hand[j] > m_rgHandPercent[j] &&
            m_PossibleMoves[i].hand[j] > prev)
         {
            handtype = j;
            prev = m_PossibleMoves[i].hand[j];
            index = i; // select this card
         }
      }
   }

   assert(index != -1);

   // check if we can make a winning hand immediately
   if (prev > 100) {
      m_iWantedMove = index;
      return m_PossibleMoves[index].handindex;
   }

   int goal = AnalyzeGoal();
   index = -1;
   handtype = HAND_MAXNUM + 1;
   prev = -1;

   for (i = 0; i < m_iNumPossibleMove; i++) {
      for (j = handtype; j < HAND_COUNT; j++) {
         int score = 0;
         if (j == goal || goal == -1) {
            score += 80;
         }
         score += gpGame->GetDeskCard(m_PossibleMoves[i].deskindex).GetType() * 15;
         score += m_HandCards[m_PossibleMoves[i].handindex].GetType() * 10;
         score += m_PossibleMoves[i].hand[j] - m_rgHandPercent[j];
         if (m_rgOpnHandPercent[HAND_MAX] >= 60 || m_rgOpnHandPercent[HAND_SAKECUP] > 0) {
            // Opponent is about to win. Try to stop him.
            score += (m_rgOpnHandPercent[HAND_MAX] - m_PossibleMoves[i].opnhand[HAND_MAX]) * 2;
            score += CardIsDangerous(gpGame->GetDeskCard(m_PossibleMoves[i].deskindex)) * m_rgOpnHandPercent[HAND_MAX] / 2;
         }
         for (int k = HAND_MAXNUM + 1; k < HAND_COUNT; k++) {
            score += (m_PossibleMoves[i].hand[k] - m_rgHandPercent[k]) / 5;
         }
         if (score > prev) {
            prev = score;
            index = i; // select this card
         }
      }
   }

   assert(index != -1);

   m_iWantedMove = index;
   return m_PossibleMoves[index].handindex;
}

int CBot::DiscardCard()
{
   int i, index = -1, max = -1;
   for (i = 0; i < m_iNumHandCard; i++) {
      CCard c = m_HandCards[i];

      int score = 5000 - c.GetType() * 30;
      score += (CardIsSafe(c) ? 300 : 0);
      score -= CardIsDangerous(c) * 150;
      score += NumMonthInHand(c.GetMonth()) * 50;
      score += NumMonthCaptured(c.GetMonth()) * 60;

      if (score > max) {
         index = i;
         max = score;
      }
   }

   assert(index != -1);
   return index;
}

bool CBot::WantToContinue()
{
   // FIXME: This is still stupid
   if (gpGame->GetGameMode() == GAMEMODE_BET) {
      return false;
   }

   AnalyzeHand();
   AnalyzeMoves();

   int i, j, score = 50;

   for (i = HAND_MAXNUM + 1; i < HAND_COUNT; i++) {
      score += (m_rgHandPercent[i] - m_rgOpnHandPercent[i]) * i / 80;
   }

   for (i = 0; i < m_iNumPossibleMove; i++) {
      for (j = HAND_MAXNUM + 1; j < HAND_COUNT; j++) {
         if (m_PossibleMoves[i].hand[j] > m_rgHandPercent[j] && m_PossibleMoves[i].hand[j] >= 100) {
            score += (m_PossibleMoves[i].hand[j] - m_rgHandPercent[j]) * (j - HAND_MAXNUM) / 2;
         }
         score += (m_PossibleMoves[i].hand[j] - m_rgHandPercent[j]) * (j - HAND_MAXNUM) / 15;
         score += (m_rgOpnHandPercent[j] - m_PossibleMoves[i].opnhand[j]) * (j - HAND_MAXNUM) / 30;
      }
   }

   if (gpGame->GetGameMode() != GAMEMODE_KOREAN) {
      int hand[HAND_COUNT], opnhand[HAND_COUNT];
      j = m_iNumCapturedCard;
      for (i = 0; i < m_iNumHandCard; i++) {
         AddCapturedCard(m_HandCards[i]);
      }
      for (i = 0; i < gpGame->GetNumDeskCard(); i++) {
         AddCapturedCard(gpGame->GetDeskCard(i));
      }
      AnalyzeHand(hand, opnhand);
      m_iNumCapturedCard = j;
      for (j = HAND_MAXNUM + 1; j < HAND_COUNT; j++) {
         score += (hand[j] - m_rgHandPercent[j]) * (j - HAND_MAXNUM) / 25;
      }
   }

   for (i = 0; i < m_iNumHandCard; i++) {
      if (CardIsSafe(m_HandCards[i])) {
         score += 10;
      } else {
         score -= CardIsDangerous(m_HandCards[i]) * 2;
      }
   }

   for (i = 0; i < gpGame->GetNumDeskCard(); i++) {
      if (!gpGame->GetDeskCard(i).IsValid()) {
         continue;
      }
      if (CardIsSafe(gpGame->GetDeskCard(i))) {
         score += 5;
      } else {
         score -= (CardIsDangerous(gpGame->GetDeskCard(i)) - 1) * 8;
      }
   }

   return (RandomLong(1, 300) < score);
}

int CBot::SelectCardOnDesk(int month, const CCard &drawn)
{
   if (month == m_PossibleMoves[m_iWantedMove].month) {
      return m_PossibleMoves[m_iWantedMove].deskindex;
   }

   AnalyzeHand();

   int i, index[2] = {-1, -1}, count = 0, hand[2][HAND_COUNT], opnhand[2][HAND_COUNT];
   for (i = 0; i < gpGame->GetNumDeskCard(); i++) {
      if (gpGame->GetDeskCard(i).GetMonth() == month) {
         if (count >= 2) {
            TerminateOnError("CBot::SelectCardOnDesk(): count >= 2");
         }
         index[count++] = i;
      }
   }

   assert(count == 2);
   m_CapturedCard[m_iNumCapturedCard++] = drawn;
   m_CapturedCard[m_iNumCapturedCard++] = gpGame->GetDeskCard(index[0]);
   AnalyzeHand(hand[0], opnhand[0]);
   m_CapturedCard[m_iNumCapturedCard - 1] = gpGame->GetDeskCard(index[1]);
   AnalyzeHand(hand[1], opnhand[1]);
   m_iNumCapturedCard -= 2;

   int goal = AnalyzeGoal(), chosen = -1, max = -1, handtype = HAND_MAXNUM + 1;
   for (i = 0; i < 2; i++) {
      for (int j = handtype; j < HAND_COUNT; j++) {
         int score = 0;
         if (j == goal || goal == -1) {
            score += 80;
         }
         score += gpGame->GetDeskCard(index[i]).GetType() * 15;
         score += hand[i][j] - m_rgHandPercent[j];
         if (hand[i][j] - m_rgHandPercent[j] > 0 && hand[i][j] >= 100) {
            score += 500; // picking this card will result in immediate winning hand
         }
         if (m_rgOpnHandPercent[HAND_MAX] >= 60 || m_rgOpnHandPercent[HAND_SAKECUP] > 0) {
            // Opponent is about to win. Try to stop him.
            score += (m_rgOpnHandPercent[HAND_MAX] - opnhand[i][HAND_MAX]) * 2;
            score += CardIsDangerous(gpGame->GetDeskCard(index[i])) * m_rgOpnHandPercent[HAND_MAX] / 2;
         }
         for (int k = HAND_MAXNUM + 1; k < HAND_COUNT; k++) {
            score += (hand[i][k] - m_rgHandPercent[k]) / 5;
         }
         if (score > max) {
            max = score;
            chosen = i; // select this card
         }
      }
   }

   assert(chosen != -1);
   return index[chosen];
}

void CBot::AnalyzeMoves()
{
   int i, j;
   m_iNumPossibleMove = 0;
   for (i = 0; i < gpGame->GetNumDeskCard(); i++) {
      for (j = 0; j < m_iNumHandCard; j++) {
         if (gpGame->GetDeskCard(i) == m_HandCards[j]) {
            m_PossibleMoves[m_iNumPossibleMove].deskindex = i;
            m_PossibleMoves[m_iNumPossibleMove].handindex = j;
            m_PossibleMoves[m_iNumPossibleMove].month = m_HandCards[j].GetMonth();

            // now try what can be archived after this move
            m_CapturedCard[m_iNumCapturedCard++] = m_HandCards[j];
            m_CapturedCard[m_iNumCapturedCard++] = gpGame->GetDeskCard(i);
            AnalyzeHand(m_PossibleMoves[m_iNumPossibleMove].hand,
               m_PossibleMoves[m_iNumPossibleMove].opnhand);
            m_iNumCapturedCard -= 2;

            m_iNumPossibleMove++;
         }
      }
   }
}

void CBot::AnalyzeHand(int *hand, int *opnhand)
{
   if (hand == NULL) {
      hand = m_rgHandPercent;
   }

   if (opnhand == NULL) {
      opnhand = m_rgOpnHandPercent;
   }

   // number of each kind of cards
   int num_lights = 0, num_animals = 0, num_ribbons = 0;
   int num_red = 0, num_blue = 0, num_grass = 0;
   int num_cards = 0, num_birds = 0, num_boar = 0;

   bool has_rain = false;

   bool has_sakecup = false, has_moon = false, has_flower = false;

   int i;

   for (i = 0; i < m_iNumCapturedCard; i++) {
      CCard &c = m_CapturedCard[i];
      int type = c.GetType();

      if (type == CARD_LIGHT) {
         // This is a light card
         num_lights++;
         if (c.IsRain()) {
            has_rain = true;
         } else if (c.IsMoon()) {
            has_moon = true;
         } else if (c.IsFlower()) {
            has_flower = true;
         }
      } else if (type == CARD_RIBBON_RED) {
         // This is a red ribbon card
         num_red++;
         num_ribbons++;
      } else if (type == CARD_RIBBON_BLUE) {
         // This is a blue ribbon card
         num_blue++;
         num_ribbons++;
      } else if (type == CARD_RIBBON) {
         // This is a normal ribbon card
         num_ribbons++;
         if (c.GetMonth() != 11) {
            num_grass++;
         }
      } else if (type == CARD_ANIMAL) {
         // This is an animal card
         num_animals++;
         if (c.IsSakeCup()) {
            num_cards += ((gpGame->GetGameMode() == GAMEMODE_KOREAN) ? 2 : 1);
            has_sakecup = true;
         } else if (c.IsDeer() || c.IsBoar() || c.IsButterfly()) {
            num_boar++;
         } else if (c.IsBird()) {
            num_birds++;
         }
      } else {
         // This is a normal card
         num_cards++;
         if (gpGame->GetGameMode() == GAMEMODE_KOREAN) {
            if (c.GetValue() == 43 || c.GetValue() == 45) {
               num_cards++; // these 2 cards counts as 2 normal cards each
            }
         }
      }
   }

   hand[HAND_CARDS] = num_cards * 10;
   hand[HAND_ANIMALS] = num_animals * 20;
   hand[HAND_RIBBONS] = num_ribbons * 20;
   hand[HAND_LIGHTS] = num_lights * 34 - (has_rain ? 17 : 0);
   hand[HAND_RED_RIBBONS] = num_red * 34;
   hand[HAND_BLUE_RIBBONS] = num_blue * 34;
   if (gpGame->GetGameMode() == GAMEMODE_KOREAN) {
      hand[HAND_NORMAL_RIBBONS] = num_grass * 34;
      hand[HAND_BIRD] = num_birds * 34;
      hand[HAND_BOAR] = -1;
      hand[HAND_SAKECUP] = -1;
   } else {
      hand[HAND_BOAR] = num_boar * 34;
      if (gpGame->GetGameMode() != GAMEMODE_BET) {
         hand[HAND_SAKECUP] = (has_sakecup ? 60 : 0) +
            (has_moon ? 40 : 0) + (has_flower ? 40 : 0);
      } else {
         hand[HAND_SAKECUP] = -1;
      }
      hand[HAND_NORMAL_RIBBONS] = -1;
      hand[HAND_BIRD] = -1;
   }

   // now check the opponent's hand
   num_lights = num_animals = num_ribbons = 0;
   num_red = num_blue = num_grass = 0;
   num_cards = num_birds = num_boar = 0;

   has_rain = false;
   has_sakecup = false, has_moon = false, has_flower = false;

   for (i = 0; i < GetOpponent()->GetNumCapturedCard(); i++) {
      CCard c = GetOpponent()->GetCapturedCard(i);
      int type = c.GetType();

      if (type == CARD_LIGHT) {
         // This is a light card
         num_lights++;
         if (c.IsRain()) {
            has_rain = true;
         } else if (c.IsMoon()) {
            has_moon = true;
         } else if (c.IsFlower()) {
            has_flower = true;
         }
      } else if (type == CARD_RIBBON_RED) {
         // This is a red ribbon card
         num_red++;
         num_ribbons++;
      } else if (type == CARD_RIBBON_BLUE) {
         // This is a blue ribbon card
         num_blue++;
         num_ribbons++;
      } else if (type == CARD_RIBBON) {
         // This is a normal ribbon card
         num_ribbons++;
         if (c.GetMonth() != 11) {
            num_grass++;
         }
      } else if (type == CARD_ANIMAL) {
         // This is an animal card
         num_animals++;
         if (c.IsSakeCup()) {
            num_cards += ((gpGame->GetGameMode() == GAMEMODE_KOREAN) ? 2 : 1);
            has_sakecup = true;
         } else if (c.IsDeer() || c.IsBoar() || c.IsButterfly()) {
            num_boar++;
         } else if (c.IsBird()) {
            num_birds++;
         }
      } else {
         // This is a normal card
         num_cards++;
         if (gpGame->GetGameMode() == GAMEMODE_KOREAN) {
            if (c.GetValue() == 43 || c.GetValue() == 45) {
               num_cards++; // these 2 cards counts as 2 normal cards each
            }
         }
      }
   }

   opnhand[HAND_CARDS] = num_cards * 10;
   opnhand[HAND_ANIMALS] = num_animals * 20;
   opnhand[HAND_RIBBONS] = num_ribbons * 20;
   opnhand[HAND_LIGHTS] = num_lights * 34 - (has_rain ? 17 : 0);
   opnhand[HAND_RED_RIBBONS] = num_red * 34;
   opnhand[HAND_BLUE_RIBBONS] = num_blue * 34;
   if (gpGame->GetGameMode() == GAMEMODE_KOREAN) {
      opnhand[HAND_NORMAL_RIBBONS] = num_grass * 34;
      opnhand[HAND_BIRD] = num_birds * 34;
      opnhand[HAND_BOAR] = -1;
      opnhand[HAND_SAKECUP] = -1;
   } else {
      opnhand[HAND_BOAR] = num_boar * 34;
      if (gpGame->GetGameMode() != GAMEMODE_BET) {
         opnhand[HAND_SAKECUP] = (has_sakecup ? 60 : 0) +
            (has_moon ? 40 : 0) + (has_flower ? 40 : 0);
      } else {
         opnhand[HAND_SAKECUP] = -1;
      }
      opnhand[HAND_NORMAL_RIBBONS] = -1;
      opnhand[HAND_BIRD] = -1;
   }

   if (hand[HAND_RED_RIBBONS] > 0 &&
      opnhand[HAND_RED_RIBBONS] > 0)
   {
      hand[HAND_RED_RIBBONS] = 0;
      opnhand[HAND_RED_RIBBONS] = 0;
   }

   if (hand[HAND_BLUE_RIBBONS] > 0 &&
      opnhand[HAND_BLUE_RIBBONS] > 0)
   {
      hand[HAND_BLUE_RIBBONS] = 0;
      opnhand[HAND_BLUE_RIBBONS] = 0;
   }

   if (hand[HAND_NORMAL_RIBBONS] > 0 &&
      opnhand[HAND_NORMAL_RIBBONS] > 0)
   {
      hand[HAND_NORMAL_RIBBONS] = 0;
      opnhand[HAND_NORMAL_RIBBONS] = 0;
   }

   if (hand[HAND_BOAR] > 0 &&
      opnhand[HAND_BOAR] > 0)
   {
      hand[HAND_BOAR] = 0;
      opnhand[HAND_BOAR] = 0;
   }

   if (hand[HAND_BIRD] > 0 &&
      opnhand[HAND_BIRD] > 0)
   {
      hand[HAND_BIRD] = 0;
      opnhand[HAND_BIRD] = 0;
   }

   if (hand[HAND_LIGHTS] >= 60) {
      opnhand[HAND_LIGHTS] = 0;
   }

   if (opnhand[HAND_LIGHTS] >= 60) {
      hand[HAND_LIGHTS] = 0;
   }

   if (hand[HAND_SAKECUP] >= 60) {
      opnhand[HAND_SAKECUP] = 0;
   }

   if (opnhand[HAND_SAKECUP] >= 60) {
      hand[HAND_SAKECUP] = 0;
   }

   opnhand[HAND_MAX] = 0;
   opnhand[HAND_MAXNUM] = -1;
   for (i = HAND_MAXNUM + 1; i < HAND_COUNT; i++) {
      if (opnhand[i] >= opnhand[HAND_MAX]) {
         opnhand[HAND_MAX] = opnhand[i];
         opnhand[HAND_MAXNUM] = i;
      }
   }
   hand[HAND_MAX] = 0;
   hand[HAND_MAXNUM] = -1;
   for (i = HAND_MAXNUM + 1; i < HAND_COUNT; i++) {
      if (hand[i] >= hand[HAND_MAX]) {
         hand[HAND_MAX] = hand[i];
         hand[HAND_MAXNUM] = i;
      }
   }
}

int CBot::AnalyzeGoal()
{
   int goalvalue[HAND_COUNT], i, j, max = 0, maxindex = -1;
   for (i = 0; i < HAND_COUNT; i++) {
      goalvalue[i] = 0;
   }

   for (i = 0; i < m_iNumPossibleMove; i++) {
      for (j = HAND_MAX + 1; j < HAND_COUNT; j++) {
         if (m_PossibleMoves[i].hand[j] > m_rgHandPercent[j]) {
            goalvalue[j] += m_PossibleMoves[i].hand[j] * (j - HAND_MAX);
         }
      }
   }

   for (i = 0; i < m_iNumPossibleMove; i++) {
      int m = m_PossibleMoves[i].hand[HAND_MAXNUM];
      if (goalvalue[m] > 0) {
         goalvalue[m] += m_PossibleMoves[i].hand[HAND_MAX] * (m - HAND_MAX) / 2;
      }
   }

   for (i = 0; i < m_iNumHandCard; i++) {
      CCard &c = m_HandCards[i];
      if (c.GetType() == CARD_LIGHT) {
         if (m_rgHandPercent[HAND_LIGHTS] >= 0) {
            if (goalvalue[HAND_LIGHTS] > 0) {
               goalvalue[HAND_LIGHTS] += 20;
            }
         }
         if ((c.IsMoon() || c.IsFlower()) && m_rgHandPercent[HAND_SAKECUP] >= 0) {
            if (goalvalue[HAND_SAKECUP] > 0) {
               goalvalue[HAND_SAKECUP] += 30;
            }
         }
      } else if (c.GetType() == CARD_ANIMAL) {
         if (goalvalue[HAND_ANIMALS] > 0) {
            goalvalue[HAND_ANIMALS] += 10;
         }
         if (c.IsBird() && m_rgHandPercent[HAND_BIRD] >= 0) {
            if (goalvalue[HAND_BIRD] > 0) {
               goalvalue[HAND_BIRD] += 15;
            }
         }
         if (c.IsBoar() || c.IsDeer() || c.IsButterfly()) {
            if (m_rgHandPercent[HAND_BOAR] >= 0) {
               if (goalvalue[HAND_BOAR] > 0) {
                  goalvalue[HAND_BOAR] += 15;
               }
            }
         }
      } else if (c.GetType() == CARD_RIBBON_RED) {
         if (m_rgHandPercent[HAND_RED_RIBBONS] >= 0) {
            if (goalvalue[HAND_RED_RIBBONS] > 0) {
               goalvalue[HAND_RED_RIBBONS] += 15;
            }
         }
         if (goalvalue[HAND_RIBBONS] > 0) {
            goalvalue[HAND_RIBBONS] += 10;
         }
      } else if (c.GetType() == CARD_RIBBON_BLUE) {
         if (m_rgHandPercent[HAND_BLUE_RIBBONS] >= 0) {
            if (goalvalue[HAND_BLUE_RIBBONS] > 0) {
               goalvalue[HAND_BLUE_RIBBONS] += 15;
            }
         }
         if (goalvalue[HAND_RIBBONS] > 0) {
            goalvalue[HAND_RIBBONS] += 10;
         }
      } else if (c.GetType() == CARD_RIBBON) {
         if (c.GetMonth() != 11 && m_rgHandPercent[HAND_NORMAL_RIBBONS] >= 0) {
            if (goalvalue[HAND_NORMAL_RIBBONS] > 0) {
               goalvalue[HAND_NORMAL_RIBBONS] += 15;
            }
         }
         if (goalvalue[HAND_RIBBONS] > 0) {
            goalvalue[HAND_RIBBONS] += 10;
         }
      } else {
         if (goalvalue[HAND_CARDS] > 0) {
            goalvalue[HAND_CARDS] += 1;
         }
      }
   }

   for (i = 0; i < gpGame->GetNumDeskCard(); i++) {
      CCard c = gpGame->GetDeskCard(i);
      if (c.GetType() == CARD_LIGHT) {
         if (m_rgHandPercent[HAND_LIGHTS] >= 0) {
            if (goalvalue[HAND_LIGHTS] > 0) {
               goalvalue[HAND_LIGHTS] += 10;
            }
         }
         if ((c.IsMoon() || c.IsFlower()) && m_rgHandPercent[HAND_SAKECUP] > 0) {
            if (goalvalue[HAND_SAKECUP] > 0) {
               goalvalue[HAND_SAKECUP] += 15;
            }
         }
      } else if (c.GetType() == CARD_ANIMAL) {
         if (goalvalue[HAND_ANIMALS] > 0) {
            goalvalue[HAND_ANIMALS] += 5;
         }
         if (c.IsBird() && m_rgHandPercent[HAND_BIRD] >= 0) {
            if (goalvalue[HAND_BIRD] > 0) {
               goalvalue[HAND_BIRD] += 8;
            }
         }
         if (c.IsBoar() || c.IsDeer() || c.IsButterfly()) {
            if (m_rgHandPercent[HAND_BOAR] >= 0) {
               if (goalvalue[HAND_BOAR] > 0) {
                  goalvalue[HAND_BOAR] += 8;
               }
            }
         }
      } else if (c.GetType() == CARD_RIBBON_RED) {
         if (m_rgHandPercent[HAND_RED_RIBBONS] >= 0) {
            if (goalvalue[HAND_RED_RIBBONS] > 0) {
               goalvalue[HAND_RED_RIBBONS] += 8;
            }
         }
         if (goalvalue[HAND_RIBBONS] > 0) {
            goalvalue[HAND_RIBBONS] += 5;
         }
      } else if (c.GetType() == CARD_RIBBON_BLUE) {
         if (m_rgHandPercent[HAND_BLUE_RIBBONS] >= 0) {
            if (goalvalue[HAND_BLUE_RIBBONS] > 0) {
               goalvalue[HAND_BLUE_RIBBONS] += 8;
            }
         }
         if (goalvalue[HAND_RIBBONS] > 0) {
            goalvalue[HAND_RIBBONS] += 5;
         }
      } else if (c.GetType() == CARD_RIBBON) {
         if (c.GetMonth() != 11 && m_rgHandPercent[HAND_NORMAL_RIBBONS] >= 0) {
            if (goalvalue[HAND_NORMAL_RIBBONS] > 0) {
               goalvalue[HAND_NORMAL_RIBBONS] += 8;
            }
         }
         if (goalvalue[HAND_RIBBONS] > 0) {
            goalvalue[HAND_RIBBONS] += 5;
         }
      } else {
         if (goalvalue[HAND_CARDS] > 0) {
            goalvalue[HAND_CARDS] += 1;
         }
      }
   }

   for (i = 0; i < HAND_COUNT; i++) {
      if (goalvalue[i] > max) {
         maxindex = i;
         max = goalvalue[i];
      }
   }

   return maxindex;
}

// some utility functions
int CBot::NumMonthInHand(int month)
{
   int i, count = 0;
   for (i = 0; i < m_iNumHandCard; i++) {
      if (m_HandCards[i].GetMonth() == month) {
         count++;
      }
   }
   return count;
}

int CBot::NumMonthExposed(int month)
{
   int i, count = NumMonthCaptured(month);
   for (i = 0; i < gpGame->GetNumDeskCard(); i++) {
      if (gpGame->GetDeskCard(i).GetMonth() == month) {
         count++;
      }
   }
   return count;
}

int CBot::NumMonthCaptured(int month)
{
   int i, count = 0;
   for (i = 0; i < m_iNumCapturedCard; i++) {
      if (m_CapturedCard[i].GetMonth() == month) {
         count++;
      }
   }
   for (i = 0; i < GetOpponent()->GetNumCapturedCard(); i++) {
      if (GetOpponent()->GetCapturedCard(i).GetMonth() == month) {
         count++;
      }
   }
   return count;
}

int CBot::NumMonthInvisible(int month)
{
   return 4 - NumMonthExposed(month) - NumMonthInHand(month);
}

bool CBot::CardIsSafe(const CCard &c)
{
   // Discarding a card is totally safe if all of the same month are
   // visible
   return (NumMonthInvisible(c.GetMonth()) <= 0);
}

int CBot::CardIsDangerous(const CCard &c)
{
   // Discarding a card is dangerous if the card is of the same month
   // as the opponent is waiting for.

   if (CardIsSafe(c)) {
      return 0; // card is safe, so it isn't dangerous
   }

   int i, j, k;
   CCard c1;

   for (i = HAND_COUNT - 1; i > HAND_MAXNUM; i--) {
      if (m_rgOpnHandPercent[i] >= ((i == HAND_SAKECUP) ? 30 : 60)) {
         switch (i) {
            case HAND_CARDS:
               return HAND_CARDS;

            case HAND_RIBBONS:
               if (c.GetType() == CARD_RIBBON || c.GetType() == CARD_RIBBON_RED ||
                  c.GetType() == CARD_RIBBON_BLUE)
               {
                  return HAND_RIBBONS;
               }
               j = c.GetMonth() - 1;
               if (j == 7 || j == 11) {
                  break;
               }
               k = ((j == 10) ? 3 : 2);
               c1 = j * 12 + k;
               if (HasCaptured(c1) || GetOpponent()->HasCaptured(c1)) {
                  break;
               }
               return HAND_RIBBONS;

            case HAND_ANIMALS:
               if (c.GetType() == CARD_ANIMAL) {
                  return HAND_ANIMALS;
               }
               j = c.GetMonth() - 1;
               if (j == 0 || j == 2 || j == 11) {
                  break;
               }
               k = ((j == 10 || j == 7) ? 2 : 1);
               c1 = j * 12 + k;
               if (HasCaptured(c1) || GetOpponent()->HasCaptured(c1)) {
                  break;
               }
               return HAND_ANIMALS;

            case HAND_SAKECUP:
               if ((c.GetMonth() == 3 && !HasCaptured(CCard(8)) && !GetOpponent()->HasCaptured(CCard(8))) ||
                  (c.GetMonth() == 8 && !HasCaptured(CCard(28)) && !GetOpponent()->HasCaptured(CCard(28))) ||
                  (c.GetMonth() == 9 && !HasCaptured(CCard(32)) && !GetOpponent()->HasCaptured(CCard(32))))
               {
                  return HAND_SAKECUP;
               }
               break;

            case HAND_RED_RIBBONS:
               if ((c.GetMonth() == 1 && !HasCaptured(CCard(1)) && !GetOpponent()->HasCaptured(CCard(1))) ||
                  (c.GetMonth() == 2 && !HasCaptured(CCard(5)) && !GetOpponent()->HasCaptured(CCard(5))) ||
                  (c.GetMonth() == 3 && !HasCaptured(CCard(9)) && !GetOpponent()->HasCaptured(CCard(9))))
               {
                  return HAND_RED_RIBBONS;
               }
               break;

            case HAND_BLUE_RIBBONS:
               if ((c.GetMonth() == 6 && !HasCaptured(CCard(21)) && !GetOpponent()->HasCaptured(CCard(21))) ||
                  (c.GetMonth() == 9 && !HasCaptured(CCard(33)) && !GetOpponent()->HasCaptured(CCard(33))) ||
                  (c.GetMonth() == 10 && !HasCaptured(CCard(37)) && !GetOpponent()->HasCaptured(CCard(37))))
               {
                  return HAND_BLUE_RIBBONS;
               }
               break;

            case HAND_NORMAL_RIBBONS:
               if ((c.GetMonth() == 4 && !HasCaptured(CCard(13)) && !GetOpponent()->HasCaptured(CCard(13))) ||
                  (c.GetMonth() == 5 && !HasCaptured(CCard(17)) && !GetOpponent()->HasCaptured(CCard(17))) ||
                  (c.GetMonth() == 7 && !HasCaptured(CCard(25)) && !GetOpponent()->HasCaptured(CCard(25))))
               {
                  return HAND_NORMAL_RIBBONS;
               }
               break;

            case HAND_BOAR:
               if ((c.GetMonth() == 6 && !HasCaptured(CCard(20)) && !GetOpponent()->HasCaptured(CCard(20))) ||
                  (c.GetMonth() == 7 && !HasCaptured(CCard(24)) && !GetOpponent()->HasCaptured(CCard(24))) ||
                  (c.GetMonth() == 10 && !HasCaptured(CCard(36)) && !GetOpponent()->HasCaptured(CCard(36))))
               {
                  return HAND_BOAR;
               }
               break;

            case HAND_BIRD:
               if ((c.GetMonth() == 2 && !HasCaptured(CCard(4)) && !GetOpponent()->HasCaptured(CCard(4))) ||
                  (c.GetMonth() == 4 && !HasCaptured(CCard(12)) && !GetOpponent()->HasCaptured(CCard(12))) ||
                  (c.GetMonth() == 8 && !HasCaptured(CCard(29)) && !GetOpponent()->HasCaptured(CCard(29))))
               {
                  return HAND_BIRD;
               }
               break;

            case HAND_LIGHTS:
               if ((c.GetMonth() == 1 && !HasCaptured(CCard(0)) && !GetOpponent()->HasCaptured(CCard(0))) ||
                  (c.GetMonth() == 3 && !HasCaptured(CCard(8)) && !GetOpponent()->HasCaptured(CCard(8))) ||
                  (c.GetMonth() == 8 && !HasCaptured(CCard(28)) && !GetOpponent()->HasCaptured(CCard(28))) ||
                  (c.GetMonth() == 11 && !HasCaptured(CCard(40)) && !GetOpponent()->HasCaptured(CCard(40))) ||
                  (c.GetMonth() == 12 && !HasCaptured(CCard(44)) && !GetOpponent()->HasCaptured(CCard(44))))
               {
                  return HAND_LIGHTS;
               }
               break;
         }
      }
   }

   return 0; // the card isn't dangerous
}

