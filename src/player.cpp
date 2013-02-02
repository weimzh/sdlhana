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

CBasePlayer *CBasePlayer::m_pDealer = NULL;
int CBasePlayer::m_iMaxHandCards = 8;

CBasePlayer::CBasePlayer()
{
}

CBasePlayer::~CBasePlayer()
{
}

void CBasePlayer::NewRound()
{
   int i;

   for (i = 0; i < m_iMaxHandCards; i++) {
      m_HandCards[i] = CCard::GetRandomCard();
   }

   m_iNumHandCard = m_iMaxHandCards;
   m_iNumCapturedCard = 0;
   m_iNumContinue = 0;
   m_iNumLeaveThree = 0;

   // Don't allow 4 same cards or 4 pairs in
   // hand! Yes, this is cheating :)
   bool allpairs = true;
   for (i = 0; i < m_iNumHandCard; i++) {
      int count = 0, j;
      for (j = 0; j < m_iNumHandCard; j++) {
         if (m_HandCards[i] == m_HandCards[j]) {
            count++;
         }
      }
      if (count != 2) {
         allpairs = false;
      }
      if (count >= 4) {
         int count2 = 999, k;
         while (count2 >= 4) {
            CCard::PutBackToPile(m_HandCards[i]);
            m_HandCards[i] = CCard::GetRandomCard();
            count2 = 0;
            for (k = 0; k < m_iNumHandCard; k++) {
               if (m_HandCards[i] == m_HandCards[k]) {
                  count2++;
               }
            }
         }
      }
   }

   if (allpairs) {
      int index = RandomLong(0, m_iNumHandCard - 1), k;
      int count = 999;
      while (count >= 2) {
         CCard::PutBackToPile(m_HandCards[index]);
         m_HandCards[index] = CCard::GetRandomCard();
         count = 0;
         for (k = 0; k < m_iNumHandCard; k++) {
            if (m_HandCards[index] == m_HandCards[k]) {
               count++;
            }
         }
      }
   }

   memset(&m_Result, 0, sizeof(m_Result));
   memset(&m_PrevResult, 0, sizeof(m_PrevResult));
}

void CBasePlayer::CalcResult()
{
   memset(&m_Result, 0, sizeof(m_Result));

   // number of each kind of cards
   int num_lights = 0, num_animals = 0, num_ribbons = 0;
   int num_red = 0, num_blue = 0, num_grass = 0;
   int num_cards = 0, num_birds = 0;

   bool has_rain = false;
   bool has_boar = false, has_deer = false, has_butterfly = false;
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
         } else if (c.IsDeer()) {
            has_deer = true;
         } else if (c.IsBoar()) {
            has_boar = true;
         } else if (c.IsButterfly()) {
            has_butterfly = true;
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

   // check for light cards
   if (num_lights >= 5) {
      m_Result.five_lights = 1;
   } else if (num_lights >= 4) {
      if (has_rain) {
         m_Result.rain_four_lights = 1;
      } else {
         m_Result.four_lights = 1;
      }
   } else if (num_lights >= 3) {
      if (!has_rain) {
         m_Result.three_lights = 1;
      } else if (gpGame->GetGameMode() == GAMEMODE_KOREAN) {
         m_Result.rain_three_lights = 1;
      }
   }

   // check for red ribbons
   if (num_red >= 3) {
      m_Result.red_ribbons = 1;
   }

   // check for blue ribbons
   if (num_blue >= 3) {
      m_Result.blue_ribbons = 1;
   }

   if (gpGame->GetGameMode() != GAMEMODE_KOREAN) {
      // check for boar, deer and butterfly
      if (has_boar && has_deer && has_butterfly) {
         m_Result.boar_deer_butterfly = 1;
      }

      // check for sake cup
      if (has_sakecup && gpGame->GetGameMode() != GAMEMODE_BET) {
         if (has_moon) {
            m_Result.moon_meets_sakecup = 1;
         }
         if (has_flower) {
            m_Result.flower_meets_sakecup = 1;
         }
      }
   } else {
      // check for five birds
      if (num_birds >= 3) {
         m_Result.five_birds = 1;
      }
      // check for normal ribbons
      if (num_grass >= 3) {
         m_Result.normal_ribbons = 1;
      }
   }

   // check for normal cards
   if (num_cards >= 10) {
      m_Result.cards = num_cards - 9;
   }

   // check for animal cards
   if (num_animals >= 5) {
      m_Result.animals = num_animals - 4;
   }

   if (gpGame->GetGameMode() == GAMEMODE_KOREAN && has_sakecup) {
      // In Korean game the Sake Cup cannot be count as both an
      // animal card and a normal card
      if (m_Result.cards - 2 > m_Result.animals) {
         m_Result.animals--;
         if (m_Result.animals < 0) {
            m_Result.animals = 0;
         }
      } else {
         m_Result.cards -= 2;
         if (m_Result.cards < 0) {
            m_Result.cards = 0;
         }
      }
   }

   // check for ribbon cards
   if (num_ribbons >= 5) {
      m_Result.ribbons = num_ribbons - 4;
   }

   // calculate score
   if (gpGame->GetGameMode() == GAMEMODE_KOREAN) {
      // Korean mode
      m_Result.score += m_Result.five_lights * 15;
      m_Result.score += m_Result.four_lights * 4;
      m_Result.score += m_Result.rain_four_lights * 4;
      m_Result.score += m_Result.three_lights * 3;
      m_Result.score += m_Result.rain_three_lights * 2;

      m_Result.score += m_Result.red_ribbons * 3;
      m_Result.score += m_Result.blue_ribbons * 3;
      m_Result.score += m_Result.normal_ribbons * 3;
      m_Result.score += m_Result.ribbons;

      m_Result.score += m_Result.five_birds * 5;
      m_Result.score += m_Result.animals;

      m_Result.score += m_Result.cards;
   } else {
      // Japanese mode
      m_Result.score += m_Result.five_lights * 15;
      m_Result.score += m_Result.four_lights * 10;
      m_Result.score += m_Result.rain_four_lights * 8;
      m_Result.score += m_Result.three_lights * 6;

      m_Result.score += m_Result.red_ribbons * 6;
      m_Result.score += m_Result.blue_ribbons * 6;
      m_Result.score += m_Result.ribbons;

      m_Result.score += m_Result.boar_deer_butterfly * 5;
      m_Result.score += m_Result.animals;

      m_Result.score += m_Result.flower_meets_sakecup * 3;
      m_Result.score += m_Result.moon_meets_sakecup * 3;

      m_Result.score += m_Result.cards;

      if (m_Result.score == 0 && m_iNumHandCard == 0 &&
         GetOpponent()->GetNumHandCard() == 0)
      {
         // The game is a draw. Dealer gets 6 points.
         if (IsDealer() && GetOpponent()->m_Result.score == 0) {
            m_Result.dealer = 1;
            m_Result.score = 6;
         }
      }
   }
}

void CBasePlayer::CalcAddResult()
{
   int i;

   m_Result.score += m_iNumContinue;

   if (m_iNumContinue >= 4) {
      m_Result.score *= 4;
   } else if (m_iNumContinue == 3) {
      m_Result.score *= 2;
   }

   if (m_Result.animals >= 3) {
      m_Result.score *= 2;
   }

   if (m_Result.five_lights || m_Result.four_lights || m_Result.rain_four_lights ||
      m_Result.three_lights || m_Result.rain_three_lights)
   {
      for (i = 0; i < GetOpponent()->GetNumCapturedCard(); i++) {
         if (GetOpponent()->GetCapturedCard(i).GetType() == CARD_LIGHT) {
            break;
         }
      }
      if (i >= GetOpponent()->GetNumCapturedCard()) {
         // Opponent has no lights. Double the score
         m_Result.score *= 2;
      }
   }

   if (m_Result.cards) {
      int opn_num_cards = 0;
      for (i = 0; i < GetOpponent()->GetNumCapturedCard(); i++) {
         if (GetOpponent()->GetCapturedCard(i).GetType() == CARD_NONE) {
            opn_num_cards++;
            if (GetOpponent()->GetCapturedCard(i).GetValue() == 43 ||
               GetOpponent()->GetCapturedCard(i).GetValue() == 45)
            {
               opn_num_cards++;
            }
         } else if (GetOpponent()->GetCapturedCard(i).IsSakeCup()) {
            opn_num_cards += 2;
         }
      }
      if (opn_num_cards <= 5) {
         m_Result.score *= 2;
      }
   }
}

void CBasePlayer::DrawCurResult()
{
   result_t cur_result = m_Result;

   cur_result.five_lights -= m_PrevResult.five_lights;
   cur_result.rain_four_lights -= m_PrevResult.rain_four_lights;
   cur_result.four_lights -= m_PrevResult.four_lights;
   cur_result.three_lights -= m_PrevResult.three_lights;
   cur_result.rain_three_lights -= m_PrevResult.rain_three_lights;
   cur_result.red_ribbons -= m_PrevResult.red_ribbons;
   cur_result.blue_ribbons -= m_PrevResult.blue_ribbons;
   cur_result.normal_ribbons -= m_PrevResult.normal_ribbons;
   cur_result.boar_deer_butterfly -= m_PrevResult.boar_deer_butterfly;
   cur_result.five_birds -= m_PrevResult.five_birds;
   cur_result.flower_meets_sakecup -= m_PrevResult.flower_meets_sakecup;
   cur_result.moon_meets_sakecup -= m_PrevResult.moon_meets_sakecup;
   cur_result.dealer -= m_PrevResult.dealer;

   cur_result.ribbons = ((m_PrevResult.ribbons == cur_result.ribbons) ? 0 : cur_result.ribbons);
   cur_result.animals = ((m_PrevResult.animals == cur_result.animals) ? 0 : cur_result.animals);
   cur_result.cards = ((m_PrevResult.cards == cur_result.cards) ? 0 : cur_result.cards);

#define DRAW_BOX_TEXT(text)                               \
   {                                                      \
      CBox *box = new CBox(20, 270, 595, 33, 155, 40, 185); \
      gpGeneral->PlaySound(SOUND_HINT);                   \
      gpGeneral->DrawText(text, 30, 270, 255, 255, 24);   \
      UTIL_Delay(2000);                                   \
      delete box;                                         \
   }

#define CLEAR_EFFECT                                      \
   {                                                      \
      int i;                                              \
      for (i = 0; i < m_iNumCapturedCard; i++) {          \
         m_CapturedCard[i].m_iRenderEffect = 0;           \
      }                                                   \
   }

#define DRAW_RESULT(r, cards, s)                               \
   {                                                           \
      if (cur_result.r > 0) {                                  \
         int i, j;                                             \
         for (i = 0; i < m_iNumCapturedCard; i++) {            \
            j = 0;                                             \
            while (cards[j] != 255) {                          \
               if (m_CapturedCard[i].GetValue() == cards[j]) { \
                  m_CapturedCard[i].m_iRenderEffect |= EF_BOX; \
                  break;                                       \
               }                                               \
               j++;                                            \
            }                                                  \
         }                                                     \
         DrawCaptured();                                       \
         DRAW_BOX_TEXT(va("%s [%d %s]", msg(#r), m_Result.r * s,      \
            ((s * m_Result.r <= 1) ? msg("point") : msg("points")))); \
         CLEAR_EFFECT;                                         \
         DrawCaptured();                                       \
      }                                                        \
   }

   unsigned char r_lights[] = {0, 8, 28, 40, 44, 255};
   unsigned char r_red_ribbons[] = {1, 5, 9, 255};
   unsigned char r_blue_ribbons[] = {21, 33, 37, 255};
   unsigned char r_normal_ribbons[] = {13, 17, 25, 255};
   unsigned char r_ribbons[] = {13, 17, 25, 42, 1, 5, 9, 21, 33, 37, 255};
   unsigned char r_bdb[] = {20, 24, 36, 255};
   unsigned char r_birds[] = {4, 12, 29, 255};
   unsigned char r_animals[] = {4, 12, 16, 20, 24, 29, 32, 36, 41, 255};
   unsigned char r_fms[] = {32, 8, 255};
   unsigned char r_mms[] = {32, 28, 255};
   unsigned char r_cards[] = {2, 3, 6, 7, 10, 11, 14, 15, 18, 19, 22, 23,
      26, 27, 30, 31, 32, 34, 35, 38, 39, 43, 45, 46, 47, 255};
   unsigned char r_fake[] = {255};

   if (gpGame->GetGameMode() == GAMEMODE_KOREAN) {
      DRAW_RESULT(five_lights, r_lights, 15);
      DRAW_RESULT(four_lights, r_lights, 4);
      DRAW_RESULT(rain_four_lights, r_lights, 4);
      DRAW_RESULT(three_lights, r_lights, 3);
      DRAW_RESULT(rain_three_lights, r_lights, 2);
      DRAW_RESULT(red_ribbons, r_red_ribbons, 3);
      DRAW_RESULT(blue_ribbons, r_blue_ribbons, 3);
      DRAW_RESULT(normal_ribbons, r_normal_ribbons, 3);
      DRAW_RESULT(ribbons, r_ribbons, 1);
      DRAW_RESULT(five_birds, r_birds, 5);
      DRAW_RESULT(animals, r_animals, 1);
      DRAW_RESULT(cards, r_cards, 1);
   } else {
      DRAW_RESULT(five_lights, r_lights, 15);
      DRAW_RESULT(four_lights, r_lights, 10);
      DRAW_RESULT(rain_four_lights, r_lights, 8);
      DRAW_RESULT(three_lights, r_lights, 6);
      DRAW_RESULT(red_ribbons, r_red_ribbons, 6);
      DRAW_RESULT(blue_ribbons, r_blue_ribbons, 6);
      DRAW_RESULT(ribbons, r_ribbons, 1);
      DRAW_RESULT(boar_deer_butterfly, r_bdb, 5);
      DRAW_RESULT(animals, r_animals, 1);
      DRAW_RESULT(flower_meets_sakecup, r_fms, 3);
      DRAW_RESULT(moon_meets_sakecup, r_mms, 3);
      DRAW_RESULT(cards, r_cards, 1);
      DRAW_RESULT(dealer, r_fake, 6);
   }
}

void CBasePlayer::DrawAllResult()
{
   int y = (IsBot() ? 90 : 10);
   CBox bigbox(20, y, 400, 300, 150, 50, 120, 64, true);
   UTIL_Delay(800);

   y += 10;

#define DRAW_RESULT_A(r, s)                                              \
   {                                                                     \
      if (m_Result.r > 0) {                                              \
         gpGeneral->DrawText(msg(#r), 30, y, 255, 255, 0, 24);           \
         gpGeneral->DrawText(va("%d %s", s * m_Result.r,                 \
            ((s * m_Result.r <= 1) ? msg("point") : msg("points"))),     \
            300, y, 255, 255, 255, 24);                                  \
         gpGeneral->PlaySound(SOUND_HINT);                               \
         UTIL_Delay(1000);                                               \
         y += 26;                                                        \
      }                                                                  \
   }

   if (gpGame->GetGameMode() == GAMEMODE_KOREAN) {
      DRAW_RESULT_A(five_lights, 15);
      DRAW_RESULT_A(four_lights, 4);
      DRAW_RESULT_A(rain_four_lights, 4);
      DRAW_RESULT_A(three_lights, 3);
      DRAW_RESULT_A(rain_three_lights, 2);
      DRAW_RESULT_A(red_ribbons, 3);
      DRAW_RESULT_A(blue_ribbons, 3);
      DRAW_RESULT_A(normal_ribbons, 3);
      DRAW_RESULT_A(ribbons, 1);
      DRAW_RESULT_A(five_birds, 5);
      DRAW_RESULT_A(animals, 1);
      DRAW_RESULT_A(cards, 1);
   } else {
      DRAW_RESULT_A(five_lights, 15);
      DRAW_RESULT_A(four_lights, 10);
      DRAW_RESULT_A(rain_four_lights, 8);
      DRAW_RESULT_A(three_lights, 6);
      DRAW_RESULT_A(red_ribbons, 6);
      DRAW_RESULT_A(blue_ribbons, 6);
      DRAW_RESULT_A(ribbons, 1);
      DRAW_RESULT_A(boar_deer_butterfly, 5);
      DRAW_RESULT_A(animals, 1);
      DRAW_RESULT_A(flower_meets_sakecup, 3);
      DRAW_RESULT_A(moon_meets_sakecup, 3);
      DRAW_RESULT_A(cards, 1);
      DRAW_RESULT_A(dealer, 6);
   }

   y = (IsBot() ? 330 : 240);

   gpGeneral->DrawText(va("%s %d %s", msg("total"), m_Result.score,
      ((m_Result.score <= 1) ? msg("point") : msg("points"))),
      30, y, 255, 255, 255, 24);

   y += 26;
   gpGeneral->DrawText(IsBot() ? msg("computerwin") : msg("youwin"),
      30, y, 255, 255, 0, 24);
}

bool CBasePlayer::HasCaptured(const CCard &c) const
{
   int i;
   for (i = 0; i < m_iNumCapturedCard; i++) {
      if (m_CapturedCard[i].GetValue() == c.GetValue()) {
         return true;
      }
   }
   return false;
}

void CBasePlayer::DeleteCard(int index)
{
   int i;
   for (i = index; i < m_iNumHandCard - 1; i++) {
      m_HandCards[i] = m_HandCards[i + 1];
   }
   m_iNumHandCard--;
}

void CBasePlayer::AddCapturedCard(const CCard &c)
{
   m_CapturedCard[m_iNumCapturedCard++] = c;
}

void CBasePlayer::DeleteCapturedCard(int index)
{
   int i;
   for (i = index; i < m_iNumCapturedCard - 1; i++) {
      m_CapturedCard[i] = m_CapturedCard[i + 1];
   }
   m_iNumCapturedCard--;
}

void CBasePlayer::DrawHand()
{
   SDL_Rect dstrect;
   dstrect.x = 10;
   dstrect.y = (IsBot() ? 10 : 400);
   dstrect.w = 48;
   dstrect.h = 78;

   int i;

   UTIL_FillRect(gpScreen, dstrect.x, dstrect.y, dstrect.w * 8,
      dstrect.h, 30, 130, 100);

   for (i = 0; i < m_iNumHandCard; i++) {
      gpGeneral->DrawCard(IsBot() ? CCard(255) : m_HandCards[i],
         dstrect.x, dstrect.y, dstrect.w, dstrect.h, false);
      dstrect.x += dstrect.w;
   }

   gpGeneral->UpdateScreen(10, dstrect.y, dstrect.w * 8, dstrect.h);
}

void CBasePlayer::DrawCaptured()
{
   CCard spec[48], norm[48];
   int num_spec = 0, num_norm = 0, i, j;

   for (i = 0; i < m_iNumCapturedCard; i++) {
      if (m_CapturedCard[i].GetType() == 0) {
         norm[num_norm++] = m_CapturedCard[i];
      } else {
         spec[num_spec++] = m_CapturedCard[i];
      }
   }

   // sort out the special cards
   for (i = 0; i < num_spec; i++) {
      bool changed = false;
      for (j = 0; j < num_spec - i - 1; j++) {
         if (spec[j].GetOrder() > spec[j + 1].GetOrder()) {
            CCard t = spec[j];
            spec[j] = spec[j + 1];
            spec[j + 1] = t;
            changed = true;
         }
      }
      if (!changed) {
         break;
      }
   }

   if (IsBot()) {
      UTIL_FillRect(gpScreen, 10 + 48 * m_iNumHandCard, 10,
         640 - (10 + 48 * m_iNumHandCard), 78, 30, 130, 100);

      // Draw normal cards
      int x = 588;
      for (i = 0; i < 9; i++) {
         if (i >= num_norm) {
            break;
         }
         x -= 8;
         gpGeneral->DrawCard(norm[i], x, 10, 48, 78, false);
      }
      while (i < num_norm) {
         gpGeneral->DrawCard(norm[i], 570 - 3 * (i - 9), 10, 48, 78, false);
         i++;
      }

      x = 500 - 48;

      if (num_spec > 0) {
         // Draw special cards
         int width = x - 20 - 48 * m_iNumHandCard;
         int per_width = width / num_spec;
         if (per_width > 48) {
            per_width = 48;
         }

         for (i = 0; i < num_spec; i++) {
            gpGeneral->DrawCard(spec[i], x, 10, 48, 78, false);
            x -= per_width;
         }

         x += per_width;
      }
      gpGeneral->UpdateScreen(x, 10, 640 - x, 78);
   } else {
      UTIL_FillRect(gpScreen, 10 + 48 * m_iNumHandCard, 400,
         640 - (10 + 48 * m_iNumHandCard), 78, 30, 130, 100);

      // Draw normal cards
      for (i = 0; i < 9; i++) {
         if (i >= num_norm) {
            break;
         }
         gpGeneral->DrawCard(norm[i], 580 - 8 * i, 400, 48, 78, false);
      }
      while (i < num_norm) {
         gpGeneral->DrawCard(norm[i], 570 - 3 * (i - 9), 400, 48, 78, false);
         i++;
      }
      gpGeneral->UpdateScreen(500, 400, 140, 78);

      // Draw special cards
      if (num_spec > 0) {
         int width = 620;
         int x = 580;
         int per_width = width / num_spec;
         if (per_width > 48) {
            per_width = 48;
         }

         for (i = 0; i < num_spec; i++) {
            gpGeneral->DrawCard(spec[i], x, 320, 48, 78, false);
            x -= per_width;
         }

         x += per_width;
         gpGeneral->UpdateScreen(x, 320, 640 - x, 78);
      }
   }
}

//------------------------------------------------------------------

CPlayer::CPlayer()
{
}

CPlayer::~CPlayer()
{
}

int CPlayer::SelectCard()
{
   CBox hint(20, 270, 595, 33, 40, 55, 85);
   gpGeneral->DrawText(msg("discardcardselect"), 30, 270, 255, 255, 24);

   int i, sel = -1;
   CButton *b[8] = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};

   for (i = 0; i < m_iNumHandCard; i++) {
      b[i] = new CButton(i + 1, 10 + i * 48, 400, 48, 78, 0, 0, 0);
   }

   while (1) {
      int k = gpGeneral->ReadKey();
      if (k > SDLK_LAST) {
         sel = k - SDLK_LAST - 1;
         break;
      } else {
         // maybe implement some other features here
      }
   }

   for (i = 0; i < 8; i++) {
      if (b[i] != NULL) {
         delete b[i];
      }
   }

   return sel;
}

bool CPlayer::WantToContinue()
{
   CBox *mainbox = new CBox(20, 270, 595, 70, 80, 55, 85);
   CButton *yesbtn = new CButton(1, 30, 305, 100, 30, 128, 128, 128);
   CButton *nobtn = new CButton(2, 145, 305, 100, 30, 128, 128, 128);
   bool ret = true;

   gpGeneral->DrawText(msg("koikoiyesorno"), 30, 270, 255, 255, 0, 32);
   gpGeneral->DrawText(msg("go"), 35, 305, 255, 0, 255, 30);
   gpGeneral->DrawText(msg("stop"), 150, 305, 0, 255, 255, 30);

   while (1) {
      int k = gpGeneral->ReadKey();
      if (k > SDLK_LAST) {
         if (k == SDLK_LAST + 2) {
            // the "stop" button is clicked
            ret = false;
         }
         break;
      }
   }

   delete yesbtn;
   delete nobtn;
   delete mainbox;

   return ret;
}

int CPlayer::SelectCardOnDesk(int month, const CCard &c)
{
   return gpGame->SelectCardOnDesk(month);
}

