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

#ifndef GAME_H
#define GAME_H

enum
{
   GAMEMODE_KOIKOI = 0,
   GAMEMODE_BET,
   GAMEMODE_KOREAN,
};

class CGame
{
public:
   CGame();
   ~CGame();

   void            MainMenu();
   void            Settings();

   void            RunGame();

   void            InitGame();
   void            NewRound();

   void            PlayRound();

   int             SelectCardOnDesk(int month);

   inline int      GetGameMode() const  { return m_iGameMode; }

   inline int      GetNumDeskCard()   const  { return m_iNumDeskCard; }
   inline CCard    GetDeskCard(int i) const  { assert(i >= 0 && i < m_iNumDeskCard); return m_DeskCards[i]; }

private:
   int             m_iGameMode;
   int             m_iScore;

   float           m_flAnimDuration;

   CBasePlayer    *m_pPlayers[2];

   CCard           m_DeskCards[24];
   int             m_iNumDeskCard;

   void            InitScreen();
   void            AnimDeal();
   SDL_Surface    *AnimCardMove(int sx, int sy, int dx, int dy, int w = 48, int h = 78, SDL_Surface *save = NULL, bool retsave = false, bool retcard = false);
   void            DrawScore();
   void            DrawDeskCard();
   void            CardDiscarded(const CCard &s, CBasePlayer *current, int sx, int sy);
   void            GetOneCardFromOpponent(CBasePlayer *current);
   bool            DoubleUp(CBasePlayer *player);

   int             FindFreeDeskCardSlot(int exclude = -1);
   void            RemoveDeskCard(int index);
};

extern CGame *gpGame;

#endif

