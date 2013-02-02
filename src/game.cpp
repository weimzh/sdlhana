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

CGame *gpGame = NULL;

CGame::CGame()
{
   m_iGameMode = atoi(cfg.Get("GAME", "GameMode", "0"));
   if (m_iGameMode == GAMEMODE_BET) {
      CBasePlayer::m_iMaxHandCards = 6; // only 6 cards in bet mode
   }

   m_iScore = atoi(cfg.Get("GAME", "Score", "0"));
   m_flAnimDuration = atof(cfg.Get("OPTIONS", "AnimSpeed", "180"));
   m_pPlayers[0] = m_pPlayers[1] = NULL;

   // don't allow more than 99999 points or less than -99999 points
   if (abs(m_iScore) > 99999) {
      m_iScore = 99999 * (m_iScore / abs(m_iScore));
   }
}

CGame::~CGame()
{
   if (m_pPlayers[0] != NULL) {
      delete m_pPlayers[0];
   }
   m_pPlayers[0] = NULL;
   if (m_pPlayers[1] != NULL) {
      delete m_pPlayers[1];
   }
   m_pPlayers[1] = NULL;
}

void CGame::MainMenu()
{
   int choice = -1;
   CBox *mainbox;
   CButton *bNewGame, *bOption, *bQuit;

start:
   gpGeneral->ClearScreen(true, false, true);

   mainbox = new CBox(140, 250, 350, 190, 58, 110, 165);
   bNewGame = new CButton(1, 150, 260, 330, 50, 58, 110, 165);
   bOption = new CButton(2, 150, 320, 330, 50, 58, 110, 165);
   bQuit = new CButton(3, 150, 380, 330, 50, 58, 110, 165);

   gpGeneral->DrawText(msg("play"), 155, 268, 255, 255, 0, 32);
   gpGeneral->DrawText(msg("settings"), 155, 328, 255, 255, 0, 32);
   gpGeneral->DrawText(msg("quit"), 155, 388, 255, 255, 0, 32);

   while (1) {
      int k = gpGeneral->ReadKey();
      if (k > SDLK_LAST) {
         choice = k - SDLK_LAST;
         break;
      }
   }

   delete bNewGame;
   delete bOption;
   delete bQuit;
   delete mainbox;

   switch (choice) {
      // single player game
      case 1:
         RunGame();
         break;

      // settings
      case 2:
         Settings();
         goto start;

      // quit game
      case 3:
         UserQuit();
         break;
   }
}

void CGame::Settings()
{
   gpGeneral->ClearScreen();
   gpGeneral->DrawText(msg("fullscreen"), 20, 20, 255, 255, 0, 24);
   gpGeneral->DrawText(msg("enablesound"), 20, 50, 255, 255, 0, 24);
   gpGeneral->DrawText(msg("gamemode"), 20, 80, 255, 255, 0, 24);
   gpGeneral->DrawText(msg("language"), 20, 110, 255, 255, 0, 24);
   gpGeneral->DrawText(msg("animspeed"), 20, 140, 255, 255, 0, 24);

   CButton fs(1, 230, 20, 150, 24, 0, 0, 0);
   CButton snd(2, 230, 50, 150, 24, 0, 0, 0);
   CButton gm(3, 230, 80, 150, 24, 0, 0, 0);
   CButton l(4, 230, 110, 150, 24, 0, 0, 0);
   CButton as(5, 230, 140, 150, 24, 0, 0, 0);
   CButton ok(6, 20, 180, 150, 34, 58, 110, 165);

   gpGeneral->DrawText("OK", 25, 185, 255, 255, 255, 24);

   int curgm = 0, curas = 3;

   char lngs[256], *curlng;
   strcpy(lngs, cfg.Get("OPTIONS", "AllLanguage", "eng"));
   curlng = strtok(lngs, ",");
   const char *strgm[3] = {msg("gamemode0"), msg("gamemode1"), msg("gamemode2")};
   const char *stras[5] = {msg("veryslow"), msg("slow"), msg("middle"), msg("fast"), msg("veryfast")};
   int valueas[5] = {800, 500, 300, 180, 50};

   cfg.Set("OPTIONS", "AnimSpeed", "180");

   while (1) {
      UTIL_FillRect(gpScreen, 230, 20, 12 * 8, 24, 30, 130, 100);
      if (atoi(cfg.Get("OPTIONS", "FullScreen", "0"))) {
         gpGeneral->DrawText(msg("Enabled"), 230, 20, 255, 255, 255, 24);
      } else {
         gpGeneral->DrawText(msg("Disabled"), 230, 20, 255, 255, 255, 24);
      }

      UTIL_FillRect(gpScreen, 230, 50, 12 * 8, 24, 30, 130, 100);
      if (!atoi(cfg.Get("OPTIONS", "NoSound", "0"))) {
         gpGeneral->DrawText(msg("Enabled"), 230, 50, 255, 255, 255, 24);
      } else {
         gpGeneral->DrawText(msg("Disabled"), 230, 50, 255, 255, 255, 24);
      }
      UTIL_FillRect(gpScreen, 230, 80, 640 - 230, 24, 30, 130, 100);
      gpGeneral->DrawText(strgm[atoi(cfg.Get("GAME", "GameMode", "0"))], 230, 80, 255, 255, 255, 24);
      gpGeneral->UpdateScreen(230, 80, 640 - 230, 24);
      UTIL_FillRect(gpScreen, 230, 110, 640 - 230, 24, 30, 130, 100);
      gpGeneral->DrawText(msg(cfg.Get("OPTIONS", "Language", "eng")), 230, 110, 255, 255, 255, 24);
      gpGeneral->UpdateScreen(230, 110, 640 - 230, 24);
      UTIL_FillRect(gpScreen, 230, 140, 640 - 230, 24, 30, 130, 100);
      gpGeneral->DrawText(stras[curas], 230, 140, 255, 255, 255, 24);
      gpGeneral->UpdateScreen(230, 140, 640 - 230, 24);
      int k = gpGeneral->ReadKey();
      if (k > SDLK_LAST) {
         switch (k - SDLK_LAST) {
            case 1:
               if (atoi(cfg.Get("OPTIONS", "FullScreen", "0"))) {
                  cfg.Set("OPTIONS", "FullScreen", "0");
               } else {
                  cfg.Set("OPTIONS", "FullScreen", "1");
               }
               break;

            case 2:
               if (atoi(cfg.Get("OPTIONS", "NoSound", "0"))) {
                  cfg.Set("OPTIONS", "NoSound", "0");
               } else {
                  cfg.Set("OPTIONS", "NoSound", "1");
               }
               break;

            case 3:
               cfg.Set("GAME", "GameMode", va("%d", curgm));
               m_iGameMode = curgm;
               if (m_iGameMode == GAMEMODE_BET) {
                  CBasePlayer::m_iMaxHandCards = 6; // only 6 cards in bet mode
               } else {
                  CBasePlayer::m_iMaxHandCards = 8;
               }
               if (++curgm >= 3) {
                  curgm = 0;
               }
               break;

            case 4:
               cfg.Set("OPTIONS", "Language", curlng);
               curlng = strtok(NULL, ",");
               if (curlng == NULL) {
                  strcpy(lngs, cfg.Get("OPTIONS", "AllLanguage", "eng"));
                  curlng = strtok(lngs, ",");
               }
               break;

            case 5:
               if (++curas >= 5) {
                  curas = 0;
               }
               cfg.Set("OPTIONS", "AnimSpeed", va("%d", valueas[curas]));
               m_flAnimDuration = (float)valueas[curas];
               break;

            case 6:
               InitTextMessage();
               gpGeneral->LoadFonts();

               if (atoi(cfg.Get("OPTIONS", "NoSound", "0"))) {
                  g_fNoSound = true;
               } else {
                  g_fNoSound = false;
                  // see if the audio device is already opened
                  int frequency, channels;
                  unsigned short format;
                  extern bool g_fAudioOpened;
                  if (!g_fAudioOpened) {
                     // audio device is not opened. Try to open the audio device
                     if (SOUND_OpenAudio(22050, AUDIO_S16, 1, 1024)) {
                        fprintf(stderr, "WARNING: Couldn't open audio: %s\n", SDL_GetError());
                        g_fNoSound = true;
                     } else {
                        g_fAudioOpened = true;
                        gpGeneral->LoadSound();
                     }
                  } else {
                     gpGeneral->LoadSound();
                  }
               }

               bool fs = (atoi(cfg.Get("OPTIONS", "FullScreen", "0")) > 0);
               if (!fs && (gpScreen->flags & SDL_FULLSCREEN) ||
                  fs && !(gpScreen->flags & SDL_FULLSCREEN))
                  UTIL_ToggleFullScreen();
               return; // exit settings
         }
      }
   }
}

void CGame::RunGame()
{
   m_pPlayers[0] = new CPlayer;
   m_pPlayers[1] = new CBot;
   if (m_pPlayers[0] == NULL || m_pPlayers[1] == NULL) {
      TerminateOnError("Memory allocation error!");
   }

   m_pPlayers[0]->SetOpponent(m_pPlayers[1]);
   m_pPlayers[1]->SetOpponent(m_pPlayers[0]);

   InitGame();

   while (1) {
      NewRound();
      PlayRound();
      // save the score
      cfg.Set("GAME", "Score", va("%d", m_iScore));
   }
}

void CGame::InitGame()
{
   if (GetGameMode() != GAMEMODE_BET) {
      // set random player as the dealer
      m_pPlayers[RandomLong(0, 1)]->SetAsDealer();
   } else {
      m_pPlayers[1]->SetAsDealer();
   }

   // Play a nice animation :P
   gpGeneral->ClearScreen(true, true, false);

   for (int i = 0; i < 48; i++) {
      CCard c(i);
      gpGeneral->DrawCard(c, RandomLong(0, 640 - 48), RandomLong(0, 480 - 78),
         48, 78, true);
      UTIL_Delay(5);
   }
}

void CGame::NewRound()
{
   CCard::NewRound();
   m_iNumDeskCard = 0;
   if (m_iGameMode != GAMEMODE_BET) {
      CBasePlayer::GetDealer()->GetOpponent()->SetAsDealer();
   }

   m_pPlayers[0]->NewRound();
   m_pPlayers[1]->NewRound();
}

void CGame::PlayRound()
{
   int i;

   // Deal 8 cards to the table
   for (i = 0; i < 8; i++) {
      m_DeskCards[i] = CCard::GetRandomCard();
   }
   m_iNumDeskCard = 8;

   // Don't allow 3 or more same card or 4 pairs on the
   // table! Instead of wasting time redealing, just
   // do a small cheat here :)
   bool allpairs = true;
   for (i = 0; i < m_iNumDeskCard; i++) {
      int count = 0, j;
      for (j = 0; j < m_iNumDeskCard; j++) {
         if (m_DeskCards[i] == m_DeskCards[j]) {
            count++;
         }
      }
      if (count != 2) {
         allpairs = false;
      }
      if (count >= 3) {
         int count2 = 999, k;
         while (count2 >= 3) {
            CCard::PutBackToPile(m_DeskCards[i]);
            m_DeskCards[i] = CCard::GetRandomCard();
            count2 = 0;
            for (k = 0; k < m_iNumDeskCard; k++) {
               if (m_DeskCards[i] == m_DeskCards[k]) {
                  count2++;
               }
            }
         }
      }
   }

   if (allpairs) {
      int index = RandomLong(0, m_iNumDeskCard - 1), k;
      int count = 999;
      while (count >= 2) {
         CCard::PutBackToPile(m_DeskCards[index]);
         m_DeskCards[index] = CCard::GetRandomCard();
         count = 0;
         for (k = 0; k < m_iNumDeskCard; k++) {
            if (m_DeskCards[index] == m_DeskCards[k]) {
               count++;
            }
         }
      }
   }

   if (m_iGameMode == GAMEMODE_BET) {
      m_iScore -= 1;
   }

   // don't allow more than 99999 points or less than -99999 points
   if (abs(m_iScore) > 99999) {
      m_iScore = 99999 * (m_iScore / abs(m_iScore));
   }

   InitScreen();
   AnimDeal();

   // playing the game
   CBasePlayer *current = CBasePlayer::GetDealer(), *winner = NULL;

   while (1) {
      if (current->GetNumHandCard() <= 0) {
         // No card is left
         if (GetGameMode() != GAMEMODE_KOREAN) {
            current->CalcResult();
            if (current->m_Result.score > 0 &&
               current->GetOpponent()->m_Result.score <= 0)
            {
               winner = current;
            }
         } else {
            winner = NULL;
         }
         break;
      }

      int s = current->SelectCard();

      int x = 10 + s * 48;
      int y = current->IsBot() ? 10 : 400;
      if (current->IsBot()) {
         UTIL_Delay(100);
         gpGeneral->DrawCard(current->GetHandCard(s), x, y, 48, 78);
         UTIL_Delay(200);
      }

      CardDiscarded(current->GetHandCard(s), current, 10 + 48 * s,
         (current->IsBot() ? 10 : 400));

      current->DeleteCard(s);
      current->DrawHand();
      current->DrawCaptured();
      DrawDeskCard();
      current->CalcResult();
      // Korean game requires at least 3 points
      if (current->m_Result.score - current->GetPrevScore() > 0 &&
         (GetGameMode() != GAMEMODE_KOREAN || current->m_Result.score >= 3)) {
         winner = current;
         current->DrawCurResult();
         if (current->GetNumHandCard() > 0 && current->WantToContinue())
         {
            current->ShiftResult();
            gpGeneral->PlaySound(SOUND_GO);
            current->m_iNumContinue++;
            UTIL_Delay(1000);
         } else {
            break;
         }
      }

      if (GetGameMode() == GAMEMODE_KOREAN && current->m_iNumLeaveThree >= 3) {
         CBox box(20, 300, 595, 33, 40, 55, 85);
         if (current->IsBot()) {
            gpGeneral->DrawText(msg("comget5pts"), 30, 300, 255, 255, 24);
            m_iScore -= 3;
            gpGeneral->PlaySound(SOUND_LOSE);
         } else {
            gpGeneral->DrawText(msg("youget5pts"), 30, 300, 255, 255, 24);
            m_iScore += 3;
            gpGeneral->PlaySound(SOUND_WIN);
         }
         DrawScore();
         UTIL_Delay(2000);
         break;
      }

      current = current->GetOpponent();
   }

   if (winner != NULL) {
      if (GetGameMode() == GAMEMODE_KOREAN) {
         winner->CalcAddResult();
      }
      winner->DrawAllResult();
      if (winner->IsBot()) {
         if (m_iGameMode != GAMEMODE_BET) {
            m_iScore -= winner->m_Result.score;
         }
         gpGeneral->PlaySound(SOUND_LOSE);
         UTIL_Delay(2500);
      } else {
         gpGeneral->PlaySound(SOUND_WIN);
         UTIL_Delay(2500);

         if (m_iGameMode == GAMEMODE_BET) {
            while (DoubleUp(winner)) {
               // Do nothing here
            }
         }

         m_iScore += winner->m_Result.score;
      }
   }

   return;
}

// FIXME: The dirtiest code here!!!
void CGame::CardDiscarded(const CCard &c, CBasePlayer *current, int sx, int sy)
{
   // draw one card from the desk
   CCard drawn = CCard::GetRandomCard();

   SDL_Surface *save1 = NULL, *save2 = NULL;
   int x1, y1, x2, y2, getfour_month = -1;
   bool leavethree = false;

   // see if the discarded card can match any one on the desk
   int index[3] = {-1, -1, -1}, count = 0, i, slot;
   for (i = 0; i < m_iNumDeskCard; i++) {
      if (c == m_DeskCards[i]) {
         if (count >= 3) {
            TerminateOnError("CGame::CardDiscarded(): count >= 3");
         }
         index[count++] = i;
      }
   }

   if (count <= 0) {
      // No card matches the discarded one. Just throw the
      // discarded one to the desk.
      slot = FindFreeDeskCardSlot();
      int dx = 140 + 48 * (slot / 2);
      int dy = 100 + 78 * (slot & 1);

      AnimCardMove(sx, sy, dx, dy);
      gpGeneral->PlaySound(SOUND_MOVECARD);

      m_DeskCards[slot] = c;
      if (slot >= m_iNumDeskCard) {
         m_iNumDeskCard = slot + 1;
      }

      UTIL_Delay(200);
      slot = 999;

      if (GetGameMode() == GAMEMODE_KOREAN) {
         getfour_month = c.GetMonth();
      }
   } else if (count == 1) {
      // Only one card matches the discarded one
      slot = index[0];

      x1 = 140 + 48 * (slot / 2) + 10;
      y1 = 100 + 78 * (slot & 1) + 10;

      save1 = AnimCardMove(sx, sy, x1, y1, 48, 78, NULL, true);
      gpGeneral->PlaySound(SOUND_PICKCARD);
      UTIL_Delay(200);

      if (GetGameMode() == GAMEMODE_KOREAN && c == drawn &&
         current->GetNumHandCard() > 1)
      {
         SDL_FreeSurface(save1);
         save1 = NULL;
         int slot1 = FindFreeDeskCardSlot();
         m_DeskCards[slot1] = c;
         if (slot1 >= m_iNumDeskCard) {
            m_iNumDeskCard = slot1 + 1;
         }
         leavethree = true;
         slot = 999;
      } else {
         current->AddCapturedCard(c);
         current->AddCapturedCard(m_DeskCards[slot]);
         m_DeskCards[slot].Destroy();
      }
   } else if (GetGameMode() == GAMEMODE_KOREAN && count >= 3) {
      // Three cards match the discarded one. Pick all these three cards
      x1 = 140 + 48 * (index[0] / 2) + 10;
      y1 = 100 + 78 * (index[0] & 1) + 10;

      save1 = AnimCardMove(sx, sy, x1, y1, 48, 78, NULL, true);
      gpGeneral->PlaySound(SOUND_PICKCARD);
      UTIL_Delay(200);

      AnimCardMove(x1, y1, 575, current->IsBot() ? 10 : 400, 48, 78, save1);
      UTIL_Delay(50);

      for (int k = 0; k < 3; k++) {
         x1 = 140 + 48 * (index[k] / 2);
         y1 = 100 + 78 * (index[k] & 1);

         gpGeneral->PlaySound(SOUND_MOVECARD);
         AnimCardMove(x1, y1, 575, current->IsBot() ? 10 : 400);
         UTIL_Delay(50);
      }

      current->AddCapturedCard(c);
      current->AddCapturedCard(m_DeskCards[index[0]]);
      current->AddCapturedCard(m_DeskCards[index[1]]);
      current->AddCapturedCard(m_DeskCards[index[2]]);

      m_DeskCards[index[0]].Destroy();
      m_DeskCards[index[1]].Destroy();
      m_DeskCards[index[2]].Destroy();

      save1 = NULL;
      slot = 999;

      if (current->GetNumHandCard() > 1 && current->GetOpponent()->GetNumHandCard() > 0) {
         GetOneCardFromOpponent(current);
      }
   } else if (count == 2) {
      // Two cards match the discarded one
      if (m_DeskCards[index[0]].GetType() == m_DeskCards[index[1]].GetType()) {
         if (m_DeskCards[index[1]].GetValue() == 43 || m_DeskCards[index[1]].GetValue() == 45) {
            // These two cards counts as 2 normal cards each in Korean game,
            // so always pick these ones
            slot = index[1];
         } else {
            slot = index[0];
         }
      } else {
         slot = current->SelectCardOnDesk(m_DeskCards[index[0]].GetMonth(), c);
      }

      x1 = 140 + 48 * (slot / 2) + 10;
      y1 = 100 + 78 * (slot & 1) + 10;

      save1 = AnimCardMove(sx, sy, x1, y1, 48, 78, NULL, true);
      gpGeneral->PlaySound(SOUND_PICKCARD);
      UTIL_Delay(200);

      current->AddCapturedCard(c);
      current->AddCapturedCard(m_DeskCards[slot]);
      m_DeskCards[slot].Destroy();

      if (GetGameMode() == GAMEMODE_KOREAN) {
         getfour_month = c.GetMonth();
      }
   }

   // Draw the drawn card
   save2 = SDL_CreateRGBSurface(gpScreen->flags & (~SDL_HWSURFACE),
      48, 78, gpScreen->format->BitsPerPixel, gpScreen->format->Rmask,
      gpScreen->format->Gmask, gpScreen->format->Bmask,
      gpScreen->format->Amask);

   SDL_Rect dstrect;
   dstrect.x = 60;
   dstrect.y = 105;
   dstrect.w = 48;
   dstrect.h = 78;

   SDL_BlitSurface(gpScreen, &dstrect, save2, NULL);

   UTIL_Delay(200);
   gpGeneral->DrawCard(drawn, 60, 105, 48, 78, true);
   UTIL_Delay(200);

   count = 0;
   for (i = 0; i < m_iNumDeskCard; i++) {
      if (drawn == m_DeskCards[i]) {
         if (count >= 3) {
            TerminateOnError("CGame::CardDiscarded(): count >= 3");
         }
         index[count++] = i;
      }
   }

   if (count <= 0 || leavethree) {
      // No card matches the discarded one. Just throw the
      // discarded one to the desk.
      int oldslot = slot;
      slot = FindFreeDeskCardSlot(oldslot);
      int dx = 140 + 48 * (slot / 2);
      int dy = 100 + 78 * (slot & 1);

      SDL_Surface *card2 = AnimCardMove(60, 105, dx, dy, 48, 78, save2, true, true);
      gpGeneral->PlaySound(SOUND_MOVECARD);

      m_DeskCards[slot] = drawn;
      if (slot >= m_iNumDeskCard) {
         m_iNumDeskCard = slot + 1;
      }

      if (slot == oldslot + 1 && !(oldslot & 1)) {
         assert(save1 != NULL);
         SDL_Rect dstrect2;

         dstrect.x = 10;
         dstrect.y = 0;
         dstrect.w = 38;
         dstrect.h = 10;

         dstrect2.x = 0;
         dstrect2.y = 68;
         dstrect2.w = 38;
         dstrect2.h = 10;

         SDL_BlitSurface(card2, &dstrect, save1, &dstrect2);

         dstrect2.x = dx + 10;
         dstrect2.y = dy;

         SDL_BlitSurface(save2, &dstrect, gpScreen, &dstrect2);
         gpGeneral->UpdateScreen(dstrect2.x, dstrect2.y, dstrect2.w, dstrect2.h);
      } else if (slot == oldslot + 2) {
         SDL_Rect dstrect2;

         dstrect.x = 0;
         dstrect.y = 10;
         dstrect.w = 10;
         dstrect.h = 68;

         dstrect2.x = 38;
         dstrect2.y = 0;
         dstrect2.w = 10;
         dstrect2.h = 68;

         SDL_BlitSurface(card2, &dstrect, save1, &dstrect2);

         dstrect2.x = dx;
         dstrect2.y = dy + 10;

         SDL_BlitSurface(save2, &dstrect, gpScreen, &dstrect2);
         gpGeneral->UpdateScreen(dstrect2.x, dstrect2.y, dstrect2.w, dstrect2.h);
      } else if (slot == oldslot + 3 && !(oldslot & 1)) {
         SDL_Rect dstrect2;

         dstrect.x = 38;
         dstrect.y = 68;
         dstrect.w = 10;
         dstrect.h = 10;

         dstrect2.x = 0;
         dstrect2.y = 0;
         dstrect2.w = 10;
         dstrect2.h = 10;

         SDL_BlitSurface(card2, &dstrect2, save1, &dstrect);

         dstrect.x = dx;
         dstrect.y = dy;

         SDL_BlitSurface(save2, &dstrect2, gpScreen, &dstrect);
         gpGeneral->UpdateScreen(dstrect.x, dstrect.y, dstrect.w, dstrect.h);
      }

      SDL_FreeSurface(save2);
      SDL_FreeSurface(card2);
      save2 = NULL;

      UTIL_Delay(200);

      if (leavethree) {
         current->m_iNumLeaveThree++;
         if (leavethree && current->GetNumHandCard() >= 8) {
            // This happens in first round. Get 3 points from opponent
            CBox box(20, 300, 595, 33, 40, 55, 85);
            gpGeneral->PlaySound(SOUND_HINT);
            if (current->IsBot()) {
               gpGeneral->DrawText(msg("comget3pts"), 30, 300, 255, 255, 24);
               m_iScore -= 3;
            } else {
               gpGeneral->DrawText(msg("youget3pts"), 30, 300, 255, 255, 24);
               m_iScore += 3;
            }
            UTIL_Delay(2000);
            DrawScore();
         }
      }
   } else if (count == 1) {
      // Only one card matches the discarded one
      x2 = 140 + 48 * (index[0] / 2) + 10;
      y2 = 100 + 78 * (index[0] & 1) + 10;

      save2 = AnimCardMove(60, 105, x2, y2, 48, 78, save2, true);
      gpGeneral->PlaySound(SOUND_PICKCARD);
      UTIL_Delay(200);

      if (drawn.GetMonth() == getfour_month) {
         if (current->GetNumHandCard() > 1 && current->GetOpponent()->GetNumHandCard() > 0) {
            GetOneCardFromOpponent(current);
         }
      }

      current->AddCapturedCard(m_DeskCards[index[0]]);
      current->AddCapturedCard(drawn);
      m_DeskCards[index[0]].Destroy();
   } else if (GetGameMode() == GAMEMODE_KOREAN && count >= 3) {
      // Three cards match the discarded one. Pick all these three cards
      x2 = 140 + 48 * (index[0] / 2) + 10;
      y2 = 100 + 78 * (index[0] & 1) + 10;

      save2 = AnimCardMove(60, 105, x2, y2, 48, 78, save2, true);
      gpGeneral->PlaySound(SOUND_PICKCARD);
      UTIL_Delay(200);

      if (save1 != NULL && save2 != NULL && (x1 < x2 || y1 < y2)) {
         SDL_Surface *savet = save1;
         save1 = save2;
         save2 = savet;
         i = x1;
         x1 = x2;
         x2 = i;
         i = y1;
         y1 = y2;
         y2 = i;
      }

      gpGeneral->PlaySound(SOUND_MOVECARD);
      AnimCardMove(x2, y2, 575, current->IsBot() ? 10 : 400, 48, 78, save2);
      UTIL_Delay(50);
      gpGeneral->PlaySound(SOUND_MOVECARD);
      AnimCardMove(x2 - 10, y2 - 10, 575, current->IsBot() ? 10 : 400);
      UTIL_Delay(50);
      save2 = NULL;

      if (save1 != NULL) {
         gpGeneral->PlaySound(SOUND_MOVECARD);
         AnimCardMove(x1, y1, 575, current->IsBot() ? 10 : 400, 48, 78, save1);
         UTIL_Delay(50);
         gpGeneral->PlaySound(SOUND_MOVECARD);
         AnimCardMove(x1 - 10, y1 - 10, 575, current->IsBot() ? 10 : 400);
         UTIL_Delay(50);
         save1 = NULL;
      }

      for (int k = 1; k < 3; k++) {
         x2 = 140 + 48 * (index[k] / 2);
         y2 = 100 + 78 * (index[k] & 1);

         gpGeneral->PlaySound(SOUND_MOVECARD);
         AnimCardMove(x2, y2, 575, current->IsBot() ? 10 : 400);
         UTIL_Delay(50);
      }

      current->AddCapturedCard(drawn);
      current->AddCapturedCard(m_DeskCards[index[0]]);
      current->AddCapturedCard(m_DeskCards[index[1]]);
      current->AddCapturedCard(m_DeskCards[index[2]]);

      m_DeskCards[index[0]].Destroy();
      m_DeskCards[index[1]].Destroy();
      m_DeskCards[index[2]].Destroy();

      if (current->GetNumHandCard() > 1 && current->GetOpponent()->GetNumHandCard() > 0) {
         GetOneCardFromOpponent(current);
      }
   } else if (count == 2) {
      // Two cards match the discarded one
      if (m_DeskCards[index[0]].GetType() == m_DeskCards[index[1]].GetType()) {
         if (m_DeskCards[index[1]].GetValue() == 43 || m_DeskCards[index[1]].GetValue() == 45) {
            // These two cards counts as 2 normal cards each in Korean game,
            // so always pick these ones
            slot = index[1];
         } else {
            slot = index[0];
         }
      } else {
         slot = current->SelectCardOnDesk(m_DeskCards[index[0]].GetMonth(), drawn);
      }
      x2 = 140 + 48 * (slot / 2) + 10;
      y2 = 100 + 78 * (slot & 1) + 10;

      save2 = AnimCardMove(60, 105, x2, y2, 48, 78, save2, true);
      gpGeneral->PlaySound(SOUND_PICKCARD);
      UTIL_Delay(200);

      current->AddCapturedCard(m_DeskCards[slot]);
      current->AddCapturedCard(drawn);
      m_DeskCards[slot].Destroy();
   }

   UTIL_Delay(200);

   if (save1 != NULL && save2 != NULL && (x1 < x2 || y1 < y2)) {
      SDL_Surface *savet = save1;
      save1 = save2;
      save2 = savet;
      i = x1;
      x1 = x2;
      x2 = i;
      i = y1;
      y1 = y2;
      y2 = i;
   }

   if (save2 != NULL) {
      gpGeneral->PlaySound(SOUND_MOVECARD);
      AnimCardMove(x2, y2, 575, current->IsBot() ? 10 : 400, 48, 78, save2);
      UTIL_Delay(50);
      gpGeneral->PlaySound(SOUND_MOVECARD);
      AnimCardMove(x2 - 10, y2 - 10, 575, current->IsBot() ? 10 : 400);
      UTIL_Delay(50);
   }

   if (save1 != NULL) {
      gpGeneral->PlaySound(SOUND_MOVECARD);
      AnimCardMove(x1, y1, 575, current->IsBot() ? 10 : 400, 48, 78, save1);
      UTIL_Delay(50);
      gpGeneral->PlaySound(SOUND_MOVECARD);
      AnimCardMove(x1 - 10, y1 - 10, 575, current->IsBot() ? 10 : 400);
      UTIL_Delay(50);
   }

   if (GetGameMode() == GAMEMODE_KOREAN) {
      for (i = 0; i < gpGame->GetNumDeskCard(); i++) {
         if (gpGame->GetDeskCard(i).IsValid()) {
            break;
         }
      }
      if (i >= gpGame->GetNumDeskCard()) {
         if (current->GetNumHandCard() > 1 && current->GetOpponent()->GetNumHandCard() > 0) {
            GetOneCardFromOpponent(current);
         }
      }
   }
}

void CGame::GetOneCardFromOpponent(CBasePlayer *current)
{
   int index = -1, index2 = -1, i;
   for (i = 0; i < current->GetOpponent()->GetNumCapturedCard(); i++) {
      const CCard &c = current->GetOpponent()->GetCapturedCard(i);
      if (c.GetType() != CARD_NONE) {
         continue;
      }
      if (c.GetValue() == 43 || c.GetValue() == 45) {
         index2 = i;
      } else {
         index = i;
      }
   }

   CCard g;
   if (index == -1 && index2 == -1) {
      return;
   } else if (index == -1) {
      g = current->GetOpponent()->GetCapturedCard(index2);
      current->AddCapturedCard(g);
      current->GetOpponent()->DeleteCapturedCard(index2);
   } else {
      g = current->GetOpponent()->GetCapturedCard(index);
      current->AddCapturedCard(g);
      current->GetOpponent()->DeleteCapturedCard(index);
   }

   UTIL_Delay(500);
   gpGeneral->PlaySound(SOUND_HINT2);
   current->GetOpponent()->DrawCaptured();
   int sy = (current->IsBot() ? 400 : 10), dy = (current->IsBot() ? 10 : 400);

   SDL_Surface *save = SDL_CreateRGBSurface(gpScreen->flags & (~SDL_HWSURFACE),
      48, 78, gpScreen->format->BitsPerPixel, gpScreen->format->Rmask,
      gpScreen->format->Gmask, gpScreen->format->Bmask,
      gpScreen->format->Amask);

   SDL_Rect dstrect;
   dstrect.x = 575;
   dstrect.y = sy;
   dstrect.w = 48;
   dstrect.h = 78;

   SDL_BlitSurface(gpScreen, &dstrect, save, NULL);

   gpGeneral->DrawCard(g, 575, sy, 48, 78, true);
   AnimCardMove(575, sy, 575, dy, 48, 78, save);
   current->DrawCaptured();
   UTIL_Delay(500);
}

int CGame::SelectCardOnDesk(int month)
{
   CButton *b[2];
   int count = 0, index[2], i;

   for (i = 0; i < 2; i++) {
      b[i] = NULL;
      index[i] = -1;
   }

   SDL_Surface *save = SDL_CreateRGBSurface(gpScreen->flags & (~SDL_HWSURFACE),
      480, 166, gpScreen->format->BitsPerPixel, gpScreen->format->Rmask,
      gpScreen->format->Gmask, gpScreen->format->Bmask,
      gpScreen->format->Amask);

   SDL_Rect dstrect;
   dstrect.x = 140;
   dstrect.y = 100;
   dstrect.w = 480;
   dstrect.h = 166;

   SDL_BlitSurface(gpScreen, &dstrect, save, NULL);

   for (i = 0; i < m_iNumDeskCard; i++) {
      if (m_DeskCards[i].GetMonth() == month) {
         if (count >= 2) {
            TerminateOnError("CGame::SelectCardOnDesk(): count >= 2");
         }
         index[count] = i;
         b[count] = new CButton(count, 140 + (i / 2) * 48,
            100 + (i & 1) * 78, 48, 78, 0, 0, 0);
         count++;
         m_DeskCards[i].m_iRenderEffect |= EF_BOX;
         gpGeneral->DrawCard(m_DeskCards[i], 140 + (i / 2) * 48,
            100 + (i & 1) * 78, 48, 78, true);
      }
   }

   count = -1;

   while (1) {
      int k = gpGeneral->ReadKey();
      if (k >= SDLK_LAST) {
         count = k - SDLK_LAST;
         break;
      }
   }

   for (i = 0; i < 2; i++) {
      if (b[i] != NULL) {
         delete b[i];
      }
   }

   SDL_BlitSurface(save, NULL, gpScreen, &dstrect);
   gpGeneral->UpdateScreen(dstrect.x, dstrect.y, dstrect.w, dstrect.h);
   SDL_FreeSurface(save);

   for (i = 0; i < m_iNumDeskCard; i++) {
      m_DeskCards[i].m_iRenderEffect &= ~(EF_DARK | EF_BOX);
   }

   return index[count];
}

void CGame::InitScreen()
{
   int i;
   CCard c;

   // clear the screen
   gpGeneral->ClearScreen(true, true, false);

   // draw the card pile
   c = 255;
   for (i = 0; i < 5; i++) {
      gpGeneral->DrawCard(c, 50 + i * 2, 95 + i * 2, 48, 78, true);
   }

   DrawScore();
}

void CGame::AnimDeal()
{
   SDL_Surface *save = SDL_CreateRGBSurface(gpScreen->flags & (~SDL_HWSURFACE),
      48, 78, gpScreen->format->BitsPerPixel, gpScreen->format->Rmask,
      gpScreen->format->Gmask, gpScreen->format->Bmask,
      gpScreen->format->Amask);

   SDL_Surface *card;

   int i, j, k, l;

   for (i = 0; i < 2; i++) {
      for (j = 0; j < 3; j++) {
         k = ((CBasePlayer::GetDealer() == m_pPlayers[1]) ? j : 1 - j);
         for (l = 0; l < ((m_iGameMode == GAMEMODE_BET && j < 2) ? 3 : 4); l++) {
            CCard c;
            if (j < 2) {
               if (k != 0 && k != 1) {
                  TerminateOnError("CGame::AnimDeal(): k != 0 && k != 1");
               }
               if (!m_pPlayers[k]->IsBot()) {
                  c = m_pPlayers[k]->GetHandCard(i * ((m_iGameMode == GAMEMODE_BET) ? 3 : 4) + l);
               } else {
                  c = CCard(255);
               }
            } else {
               c = m_DeskCards[l + i * 4];
            }

            card = gpGeneral->RenderCard(c, 48, 78);

            int first = SDL_GetTicks(), now = first;
            SDL_Rect dstrect, dstrect2;

            dstrect.x = 60;
            dstrect.y = 105;
            dstrect.w = dstrect2.w = card->w;
            dstrect.h = dstrect2.h = card->h;

            if (j < 2) {
               dstrect2.x = 10 + (l + i * ((m_iGameMode == GAMEMODE_BET) ? 3 : 4)) * 48;
               dstrect2.y = k ? 10 : 400;
            } else {
               dstrect2.x = 140 + (l / 2 + i * 2) * 48;
               dstrect2.y = 100 + (l & 1) * 78;
            }

            SDL_Rect prev_dstrect3 = dstrect;
            SDL_BlitSurface(gpScreen, &prev_dstrect3, save, NULL);
            do {
               SDL_Rect dstrect3;

               float ratio = (now - first) / m_flAnimDuration;
               dstrect3.x = (int)(dstrect.x + (dstrect2.x - dstrect.x) * ratio);
               dstrect3.y = (int)(dstrect.y + (dstrect2.y - dstrect.y) * ratio);

               SDL_BlitSurface(save, NULL, gpScreen, &prev_dstrect3);
               SDL_BlitSurface(gpScreen, &dstrect3, save, NULL);

               SDL_BlitSurface(card, NULL, gpScreen, &dstrect3);
               gpGeneral->UpdateScreen(prev_dstrect3.x, prev_dstrect3.y,
                  prev_dstrect3.w, prev_dstrect3.h);
               gpGeneral->UpdateScreen(dstrect3.x, dstrect3.y,
                  dstrect3.w, dstrect3.h);

               now = SDL_GetTicks();
               prev_dstrect3 = dstrect3;
            } while (now < first + m_flAnimDuration);

            SDL_BlitSurface(save, NULL, gpScreen, &prev_dstrect3);
            SDL_BlitSurface(card, NULL, gpScreen, &dstrect2);
            gpGeneral->UpdateScreen(prev_dstrect3.x, prev_dstrect3.y,
               prev_dstrect3.w, prev_dstrect3.h);
            gpGeneral->UpdateScreen(dstrect2.x, dstrect2.y,
               dstrect2.w, dstrect2.h);
            SDL_FreeSurface(card);
            gpGeneral->PlaySound(SOUND_DRAWCARD);
         }
         UTIL_Delay(200);
      }
   }

   SDL_FreeSurface(save);

   CBox box(20, 300, 595, 33, 40, 55, 85);
   gpGeneral->PlaySound(SOUND_HINT);
   gpGeneral->DrawText((CBasePlayer::GetDealer() == m_pPlayers[1]) ?
      msg("comdealer") : msg("youdealer"), 30, 300, 255, 255, 24);
   UTIL_Delay(2000);
}

SDL_Surface *CGame::AnimCardMove(int sx, int sy, int dx, int dy,
   int w, int h, SDL_Surface *save, bool retsave, bool retcard)
{
   int first = SDL_GetTicks(), now = first;
   SDL_Rect dstrect, dstrect2;

   SDL_Surface *card = SDL_CreateRGBSurface(gpScreen->flags & (~SDL_HWSURFACE),
      w, h, gpScreen->format->BitsPerPixel, gpScreen->format->Rmask,
      gpScreen->format->Gmask, gpScreen->format->Bmask,
      gpScreen->format->Amask);

   if (save == NULL) {
      save = SDL_CreateRGBSurface(gpScreen->flags & (~SDL_HWSURFACE),
         w, h, gpScreen->format->BitsPerPixel, gpScreen->format->Rmask,
         gpScreen->format->Gmask, gpScreen->format->Bmask,
         gpScreen->format->Amask);
      UTIL_FillRect(save, 0, 0, w, h, 30, 130, 100);
   }

   dstrect.x = sx;
   dstrect.y = sy;
   dstrect.w = dstrect2.w = w;
   dstrect.h = dstrect2.h = h;

   SDL_BlitSurface(gpScreen, &dstrect, card, NULL);

   dstrect2.x = dx;
   dstrect2.y = dy;

   SDL_Rect prev_dstrect3 = dstrect;
   do {
      SDL_Rect dstrect3;

      float ratio = (now - first) / m_flAnimDuration;
      dstrect3.x = (int)(dstrect.x + (dstrect2.x - dstrect.x) * ratio);
      dstrect3.y = (int)(dstrect.y + (dstrect2.y - dstrect.y) * ratio);

      SDL_BlitSurface(save, NULL, gpScreen, &prev_dstrect3);
      SDL_BlitSurface(gpScreen, &dstrect3, save, NULL);

      SDL_BlitSurface(card, NULL, gpScreen, &dstrect3);
      gpGeneral->UpdateScreen(prev_dstrect3.x, prev_dstrect3.y,
         prev_dstrect3.w, prev_dstrect3.h);
      gpGeneral->UpdateScreen(dstrect3.x, dstrect3.y,
         dstrect3.w, dstrect3.h);

      now = SDL_GetTicks();
      prev_dstrect3 = dstrect3;
   } while (now < first + m_flAnimDuration);

   SDL_BlitSurface(save, NULL, gpScreen, &prev_dstrect3);

   if (retsave) {
      SDL_BlitSurface(gpScreen, &dstrect2, save, NULL);
   } else {
      SDL_FreeSurface(save);
      save = NULL;
   }

   SDL_BlitSurface(card, NULL, gpScreen, &dstrect2);
   gpGeneral->UpdateScreen(prev_dstrect3.x, prev_dstrect3.y,
      prev_dstrect3.w, prev_dstrect3.h);
   gpGeneral->UpdateScreen(dstrect2.x, dstrect2.y,
      dstrect2.w, dstrect2.h);

   if (retcard) {
      return card;
   }

   SDL_FreeSurface(card);

   return save;
}

void CGame::DrawScore()
{
   UTIL_FillRect(gpScreen, 25, 190, 110, 70, 30, 130, 100);
   CBox s(25, 190, 110, 70, 0, 175, 0, 160, true);
   gpGeneral->DrawTextBrush("SCORE", 30, 190, 255, 255, 0, 32);
   gpGeneral->DrawTextBrush(va("%6d", m_iScore), 30, 220);
   gpGeneral->UpdateScreen(25, 190, 110, 70);
}

void CGame::DrawDeskCard()
{
   SDL_Rect dstrect;

   dstrect.w = 48;
   dstrect.h = 78;

   int i;

   for (i = 0; i < 20; i++) {
      dstrect.x = 140 + (i / 2) * 48;
      dstrect.y = 100 + (i & 1) * 78;
      if (i < m_iNumDeskCard && m_DeskCards[i].IsValid()) {
         gpGeneral->DrawCard(m_DeskCards[i], dstrect.x, dstrect.y,
            48, 78, false);
      } else {
         UTIL_FillRect(gpScreen, dstrect.x, dstrect.y, dstrect.w,
            dstrect.h, 30, 130, 100);
      }
   }

   UTIL_FillRect(gpScreen, 140, 100 + 78 * 2, 48 * 10, 10, 30, 130, 100);
   gpGeneral->UpdateScreen(140, 100, 48 * 10, 78 * 2 + 10);
}

bool CGame::DoubleUp(CBasePlayer *player)
{
   CBox *mainbox = new CBox(20, 290, 595, 70, 80, 55, 85);
   CButton *yesbtn = new CButton(1, 30, 325, 100, 30, 128, 128, 128);
   CButton *nobtn = new CButton(2, 145, 325, 100, 30, 128, 128, 128);
   bool ret = true;

   gpGeneral->DrawText(msg("doubleupyesorno"), 30, 290, 255, 255, 0, 32);
   gpGeneral->DrawText(msg("yes"), 35, 325, 255, 0, 255, 30);
   gpGeneral->DrawText(msg("no"), 150, 325, 0, 255, 255, 30);

   while (1) {
      int k = gpGeneral->ReadKey();
      if (k > SDLK_LAST) {
         if (k == SDLK_LAST + 2) {
            // the "no" button is clicked
            ret = false;
         }
         break;
      }
   }

   delete yesbtn;
   delete nobtn;
   delete mainbox;

   if (!ret) {
      return false;
   }

   gpGeneral->ClearScreen();

   // draw the card pile
   CCard c(255);
   int i;
   for (i = 0; i < 5; i++) {
      gpGeneral->DrawCard(c, 50 + i * 2, 95 + i * 2, 48, 78, true);
   }

   c = RandomLong(0, 47);

   mainbox = new CBox(20, 290, 595, 70, 80, 55, 85);
   yesbtn = new CButton(1, 30, 325, 100, 30, 128, 128, 128);
   nobtn = new CButton(2, 145, 325, 100, 30, 128, 128, 128);

   gpGeneral->DrawText(msg("bigorsmall"), 30, 290, 255, 255, 0, 32);
   gpGeneral->DrawText(msg("big"), 35, 325, 255, 0, 255, 30);
   gpGeneral->DrawText(msg("small"), 150, 325, 0, 255, 255, 30);

   CBox s(25, 190, 110, 70, 0, 175, 0, 160);
   gpGeneral->DrawTextBrush("WIN", 30, 190, 255, 255, 0, 32);
   gpGeneral->DrawTextBrush(va("%6d", player->m_Result.score), 30, 220);
   gpGeneral->UpdateScreen(25, 190, 110, 70);

   bool isbig = true;
   while (1) {
      int k = gpGeneral->ReadKey();
      if (k > SDLK_LAST) {
         if (k == SDLK_LAST + 2) {
            // the "small" button is clicked
            isbig = false;
         }
         break;
      }
   }

   delete yesbtn;
   delete nobtn;
   delete mainbox;

   SDL_Surface *save = SDL_CreateRGBSurface(gpScreen->flags & (~SDL_HWSURFACE),
      48, 78, gpScreen->format->BitsPerPixel, gpScreen->format->Rmask,
      gpScreen->format->Gmask, gpScreen->format->Bmask,
      gpScreen->format->Amask);

   SDL_Rect dstrect;
   dstrect.x = 60;
   dstrect.y = 105;
   dstrect.w = 48;
   dstrect.h = 78;

   SDL_BlitSurface(gpScreen, &dstrect, save, NULL);

   gpGeneral->DrawCard(c, 60, 105, 48, 78);
   gpGeneral->PlaySound(SOUND_DRAWCARD);
   AnimCardMove(60, 105, 150, 115, 48, 78, save);
   UTIL_Delay(800);

   if ((isbig && c.GetMonth() >= 7) || (!isbig && c.GetMonth() < 7)) {
      ret = true;
      player->m_Result.score *= 2;
      gpGeneral->DrawTextBrush("WIN", 210, 125, 255, 255, 128, 64);
      gpGeneral->PlaySound(SOUND_WIN);
   } else {
      ret = false;
      player->m_Result.score = 0;
      gpGeneral->DrawTextBrush("LOSE", 210, 125, 128, 255, 255, 64);
      gpGeneral->PlaySound(SOUND_LOSE);
   }

   UTIL_Delay(2000);
   return ret;
}

int CGame::FindFreeDeskCardSlot(int exclude)
{
   int i;
   for (i = 0; i < m_iNumDeskCard; i++) {
      if (exclude != -1 && i == exclude)
         continue;
      if (!m_DeskCards[i].IsValid() && i != exclude) {
         return i;
      }
   }
   return i;
}

void CGame::RemoveDeskCard(int index)
{
   m_DeskCards[index].Destroy();
}

