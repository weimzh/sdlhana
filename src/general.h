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

#ifndef GENERAL_H
#define GENERAL_H

enum
{
   SOUND_MOVECARD = 0,
   SOUND_PICKCARD,
   SOUND_DRAWCARD,

   SOUND_WIN,
   SOUND_LOSE,
   SOUND_GO,
   SOUND_HINT,
   SOUND_HINT2,

   NUM_SOUND,
};

#include "font.h"
#include "card.h"

class CGeneral
{
public:
   CGeneral();
   ~CGeneral();

   void          ScreenFade(int duration = 500, SDL_Surface *s = NULL);
   int           ReadKey();
   void          UpdateScreen(int x = 0, int y = 0, int w = 0, int h = 0);

   void          ClearScreen(bool fadein = false, bool fadeout = false, bool bg = false);

   void          DrawTextBrush(const char *t, int x, int y, int r = 255, int g = 255, int b = 255, int size = 32);
   void          DrawText(const char *t, int x, int y, int r = 255, int g = 255, int b = 255, int size = 32);
   SDL_Surface  *RenderCard(const CCard &c, int w = 64, int h = 104);
   void          DrawCard(const CCard &c, int x, int y, int w = 64, int h = 104, bool update = true);

   void          PlaySound(int num);

   void          LoadFonts();
   void          LoadSound();

private:
   void          LoadImages();
   void          FreeImages();
   void          FreeSound();

   SDL_Surface  *LoadBitmapFile(const char *filename);
   SDL_AudioCVT *LoadSoundFile(const char *filename);

   void          InitCursor();
   void          FreeCursor();

   SDL_Surface  *m_imgCards, *m_imgBack;
   SDL_AudioCVT *m_snd[NUM_SOUND];
   bool          m_fSndLoaded;

   CFont         m_fntBrush, m_fnt;
   SDL_Cursor   *m_HandCursor;
};

class CBox
{
public:
   CBox(int x, int y, int w, int h, int r, int g, int b, int a = 160, bool keep = false);
   virtual ~CBox();

protected:
   SDL_Surface  *m_pSavedArea;  // save the area behind this box
   SDL_Rect      m_SavedRect;
   bool          m_fFakeBox;
};

#define MAX_BUTTONS 256

class CButton : public CBox
{
public:
   CButton(int id, int x, int y, int w, int h, int r, int g, int b);
   virtual ~CButton();

   static int bx[MAX_BUTTONS], by[MAX_BUTTONS], bw[MAX_BUTTONS], bh[MAX_BUTTONS];
   static int bid[MAX_BUTTONS], num_buttons;

   static int GetButtonId(int x, int y);

private:
   int m_iId;
};

extern CGeneral *gpGeneral;

#endif

