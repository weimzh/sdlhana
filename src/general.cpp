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

CGeneral *gpGeneral = NULL;

const char *soundfile[NUM_SOUND + 1] = {
   "card1",
   "card2",
   "card3",
   "win",
   "lose",
   "koi",
   "hint",
   "hint2",
   "",
};

CGeneral::CGeneral()
{
   m_fSndLoaded = false;
   LoadFonts();
   InitCursor();
   LoadImages();
   LoadSound();
}

CGeneral::~CGeneral()
{
   FreeCursor();
   FreeImages();
   FreeSound();
}

void CGeneral::ScreenFade(int duration, SDL_Surface *s)
{
   SDL_Surface *pNewFadeSurface = SDL_CreateRGBSurface(gpScreen->flags & (~SDL_HWSURFACE),
      gpScreen->w, gpScreen->h, gpScreen->format->BitsPerPixel, gpScreen->format->Rmask,
      gpScreen->format->Gmask, gpScreen->format->Bmask,
      gpScreen->format->Amask);

   if (!pNewFadeSurface) {
      // cannot create surface, just blit the surface to the screen
      if (s != NULL) {
         SDL_BlitSurface(s, NULL, gpScreen, NULL);
         SDL_UpdateRect(gpScreen, 0, 0, gpScreen->w, gpScreen->h);
      }
      return;
   }

   if (s == NULL) {
      // make black screen
      SDL_FillRect(pNewFadeSurface, NULL,
         SDL_MapRGB(pNewFadeSurface->format, 0, 0, 0));
   } else {
      SDL_BlitSurface(s, NULL, pNewFadeSurface, NULL);
   }

   if (SDL_MUSTLOCK(gpScreen)) {
      if (SDL_LockSurface(gpScreen) < 0) {
         // cannot lock screen, just blit the surface to the screen
         if (s != NULL) {
            SDL_BlitSurface(s, NULL, gpScreen, NULL);
            SDL_UpdateRect(gpScreen, 0, 0, gpScreen->w, gpScreen->h);
         }
         return;
      }
   }

   const unsigned int size = gpScreen->pitch * gpScreen->h;
   unsigned char *fadeFromRGB = (unsigned char *)calloc(size, 1);
   unsigned char *fadeToRGB = (unsigned char *)calloc(size, 1);
   if (fadeFromRGB == NULL || fadeToRGB == NULL) {
      TerminateOnError("Memory allocation error !");
   }

   memcpy(fadeFromRGB, gpScreen->pixels, size);
   memcpy(fadeToRGB, pNewFadeSurface->pixels, size);

   int first = SDL_GetTicks(), now = first;

   do {
      // The +50 is to allow first frame to show some change
      float ratio = (now - first + 50) / (float)duration;
      const unsigned char amount = (unsigned char)(ratio * 255);
      const unsigned char oldamount = 255 - amount;
      unsigned char *pw = (unsigned char *)gpScreen->pixels;
      unsigned char *stop = pw + size;
      unsigned char *from = fadeFromRGB;
      unsigned char *to = fadeToRGB;

      do {
         //dividing by 256 instead of 255 provides huge optimization
         *pw = (oldamount * *(from++) + amount * *(to++)) / 256;
      } while (++pw != stop);

      now = SDL_GetTicks();
      SDL_UpdateRect(gpScreen, 0, 0, gpScreen->w, gpScreen->h);
   } while (now - first + 50 < duration);

   free(fadeFromRGB);
   free(fadeToRGB);

   SDL_BlitSurface(pNewFadeSurface, NULL, gpScreen, NULL);
   SDL_UpdateRect(gpScreen, 0, 0, gpScreen->w, gpScreen->h);

   if (SDL_MUSTLOCK(gpScreen))
      SDL_UnlockSurface(gpScreen);

   SDL_FreeSurface(pNewFadeSurface);
}

int CGeneral::ReadKey()
{
   SDL_Event event;

   // clear the event queue
   while (SDL_PollEvent(&event)) {
   }

   while (1) {
      if (SDL_WaitEvent(&event)) {
         if (event.type == SDL_KEYDOWN) {
            break;
         } else if (event.type == SDL_MOUSEBUTTONDOWN) {
            if (event.button.button == SDL_BUTTON_RIGHT) {
               return SDLK_RIGHT;
            } else if (event.button.button == SDL_BUTTON_LEFT) {
               int id = CButton::GetButtonId(event.button.x, event.button.y);
               if (id < 0) {
                  return SDLK_RETURN;
               }
               return SDLK_LAST + id;
            }
         }
      }
   }

   return event.key.keysym.sym;
}

void CGeneral::UpdateScreen(int x, int y, int w, int h)
{
   SDL_UpdateRect(gpScreen, x, y, w, h);
}

void CGeneral::ClearScreen(bool fadein, bool fadeout, bool bg)
{
   SDL_Surface *r = SDL_CreateRGBSurface(gpScreen->flags,
      gpScreen->w, gpScreen->h, gpScreen->format->BitsPerPixel,
      gpScreen->format->Rmask, gpScreen->format->Gmask,
      gpScreen->format->Bmask, gpScreen->format->Amask);

   if (bg) {
      int w = r->w;
      while (w > 0) {
         int h = r->h;
         while (h > 0) {
            SDL_Rect dstrect;
            dstrect.x = r->w - w;
            dstrect.y = r->h - h;
            dstrect.w = m_imgBack->w;
            dstrect.h = m_imgBack->h;
            SDL_BlitSurface(m_imgBack, NULL, r, &dstrect);
            h -= m_imgBack->h;
         }
         w -= m_imgBack->w;
      }
   } else {
      UTIL_FillRect(r, 0, 0, gpScreen->w, gpScreen->h, 30, 130, 100);
   }

   UTIL_RectShade(r, 0, 0, gpScreen->w, gpScreen->h, 196, 196,
      0, 0, 196, 196, 196, 0, 196);

   if (fadeout) {
      ScreenFade(300);
   }

   ScreenFade(fadein ? 300 : 0, r);
   SDL_FreeSurface(r);
}

void CGeneral::DrawTextBrush(const char *t, int x, int y, int r, int g, int b, int size)
{
   SDL_Surface *s = m_fntBrush.Render(t, r, g, b, size, ((size < 32) ? false : true));

   SDL_Rect dstrect;
   dstrect.x = x;
   dstrect.y = y;
   dstrect.w = s->w;
   dstrect.h = s->h;

   SDL_BlitSurface(s, NULL, gpScreen, &dstrect);
   SDL_FreeSurface(s);

   UpdateScreen(x, y, dstrect.w, dstrect.h);
}

void CGeneral::DrawText(const char *t, int x, int y, int r, int g, int b, int size)
{
   SDL_Surface *s = m_fnt.Render(t, r, g, b, size);

   SDL_Rect dstrect;
   dstrect.x = x;
   dstrect.y = y;
   dstrect.w = s->w;
   dstrect.h = s->h;

   SDL_BlitSurface(s, NULL, gpScreen, &dstrect);
   SDL_FreeSurface(s);

   UpdateScreen(x, y, dstrect.w, dstrect.h);
}

SDL_Surface *CGeneral::RenderCard(const CCard &c, int w, int h)
{
   SDL_Surface *s;
   int pw = m_imgCards->w / 4, ph = m_imgCards->h / 13;

   s = SDL_CreateRGBSurface(gpScreen->flags & ~SDL_HWSURFACE,
      w, h, gpScreen->format->BitsPerPixel,
      gpScreen->format->Rmask, gpScreen->format->Gmask,
      gpScreen->format->Bmask, gpScreen->format->Amask);

   w--;
   h--;

   SDL_SetColorKey(s, SDL_SRCCOLORKEY, SDL_MapRGBA(s->format, 0, 0, 0, 0));
   UTIL_FillRect(s, w, 1, 1, h - 1, 1, 1, 1);
   UTIL_FillRect(s, 1, h, w, 1, 1, 1, 1);

   SDL_Rect dstrect, dstrect2;
   if (c.GetValue() >= 48) {
      dstrect.x = pw * 2;
      dstrect.y = ph * 12;
   } else {
      dstrect.x = (c.GetValue() & 3) * pw;
      dstrect.y = (c.GetMonth() - 1) * ph;
   }
   dstrect.w = pw;
   dstrect.h = ph;
   dstrect2.x = dstrect2.y = 0;
   dstrect2.w = w;
   dstrect2.h = h;

   UTIL_ScaleBlit(m_imgCards, &dstrect, s, &dstrect2);

   if (c.m_iRenderEffect & EF_DARK) {
      int i, j;
      unsigned char r, g, b;
      for (i = 0; i < s->w; i++) {
         for (j = 0; j < s->h; j++) {
            UTIL_GetPixel(s, i, j, &r, &g, &b);
            r = r * 2 / 3;
            g = g * 2 / 3;
            b = b * 2 / 3;
            UTIL_PutPixel(s, i, j, r, g, b);
         }
      }
   }

   if (c.m_iRenderEffect & EF_BOX) {
      UTIL_RectShade(s, 0, 0, w, h, 255, 255, 0, 0, 255, 255, 255, 0, 255);
      UTIL_RectShade(s, 1, 1, w - 2, h - 2, 255, 255, 0, 0, 255, 255, 255, 0, 255);
   }

   return s;
}

void CGeneral::DrawCard(const CCard &c, int x, int y, int w, int h, bool update)
{
   SDL_Surface *p = RenderCard(c, w, h);
   SDL_Rect dstrect;

   dstrect.x = x;
   dstrect.y = y;
   dstrect.w = w;
   dstrect.h = h;

   SDL_BlitSurface(p, NULL, gpScreen, &dstrect);
   SDL_FreeSurface(p);

   if (update) {
      UpdateScreen(x, y, dstrect.w, dstrect.h);
   }
}

void CGeneral::LoadFonts()
{
   m_fntBrush.Load(FONTS_DIR "brush.fnt");
   m_fnt.Load(va("%s%s.fnt", FONTS_DIR, cfg.Get("OPTIONS", "Language", "eng")));
}

void CGeneral::LoadImages()
{
   m_imgCards = LoadBitmapFile(IMAGES_DIR "cards.bmp");
   m_imgBack = LoadBitmapFile(IMAGES_DIR "back.bmp");
}

void CGeneral::FreeImages()
{
   SDL_FreeSurface(m_imgCards);
   SDL_FreeSurface(m_imgBack);
}

void CGeneral::LoadSound()
{
   if (g_fNoSound) {
      memset(m_snd, 0, sizeof(m_snd));
      return;
   }

   if (m_fSndLoaded) {
      return;
   }

   m_fSndLoaded = true;

   int i;

   for (i = 0; i < NUM_SOUND; i++) {
      assert(*soundfile[i]);
      m_snd[i] = LoadSoundFile(va("%s%s.wav", SOUND_DIR, soundfile[i]));
   }
}

void CGeneral::FreeSound()
{
   if (g_fNoSound) {
      return;
   }

   int i;

   for (i = 0; i < NUM_SOUND; i++) {
      if (m_snd[i] != NULL) {
         SOUND_FreeWAV(m_snd[i]);
      }
   }
}

void CGeneral::PlaySound(int num)
{
   assert(num >= 0 && num < NUM_SOUND);
   if (g_fNoSound) {
      return;
   }
   SOUND_PlayWAV(m_snd[num]);
}

SDL_Surface *CGeneral::LoadBitmapFile(const char *filename)
{
   SDL_Surface *pic = SDL_LoadBMP(filename);

   if (pic == NULL) {
      TerminateOnError("Cannot load Bitmap file %s: %s", filename, SDL_GetError());
   }

   return pic;
}

SDL_AudioCVT *CGeneral::LoadSoundFile(const char *filename)
{
   if (g_fNoSound) {
      return NULL;
   }

   SDL_AudioCVT *s = SOUND_LoadWAV(filename);

   if (s == NULL) {
      TerminateOnError("Cannot load sound file %s: %s",
         filename, SDL_GetError());
   }

   return s;
}

void CGeneral::InitCursor()
{
   char cursor_data[22 + 1][18 + 1] = {
      "*****  ***********",
      "**** )) **********",
      "**** )) **********",
      "**** )) **********",
      "**** )) **********",
      "**** ))   ********",
      "**** )) ))   *****",
      "**** )) )) ))  ***",
      "**** )) )) )) ) **",
      "   ) )) )) )) )) *",
      " ))  )))))))) )) *",
      " ))) ))))))))))) *",
      "* )) ))))))))))) *",
      "** ) ))))))))))) *",
      "** ))))))))))))) *",
      "*** )))))))))))) *",
      "*** ))))))))))) **",
      "**** )))))))))) **",
      "**** )))))))))) **",
      "***** )))))))) ***",
      "***** )))))))) ***",
      "*****          ***"
   };

   unsigned char data[24 * 3], mask[24 * 3];

   int i, j, index = -1;

   for (i = 0; i < 24 * 3; i++) {
      data[i] = mask[i] = 0;
   }

   for (i = 0; i < 22; i++) {
      for (j = 0; j < 24; j++) {
         if (j % 8 != 0) {
            data[index] <<= 1;
            mask[index] <<= 1;
         } else {
            index++;
         }

         if (j >= 18) {
            continue;
         }

         switch (cursor_data[i][j]) {
            case ')':
               mask[index] |= 1;
               break;

            case ' ':
               data[index] |= 1;
               mask[index] |= 1;
               break;
         }
      }
   }

   m_HandCursor = SDL_CreateCursor(data, mask, 24, 24, 0, 0);
   SDL_SetCursor(m_HandCursor);
}

void CGeneral::FreeCursor()
{
   SDL_FreeCursor(m_HandCursor);
}

CBox::CBox(int x, int y, int w, int h, int r, int g, int b, int a, bool keep)
{
   if (r == 0 && g == 0 && b == 0) {
      m_fFakeBox = true;
      return;
   }

   m_fFakeBox = false;

   if (keep) {
      m_pSavedArea = NULL;
   } else {
      m_pSavedArea = SDL_CreateRGBSurface(gpScreen->flags,
         w, h, gpScreen->format->BitsPerPixel,
         gpScreen->format->Rmask, gpScreen->format->Gmask,
         gpScreen->format->Bmask, gpScreen->format->Amask);

      m_SavedRect.x = x;
      m_SavedRect.y = y;
      m_SavedRect.w = w;
      m_SavedRect.h = h;

      SDL_BlitSurface(gpScreen, &m_SavedRect, m_pSavedArea, NULL);
   }

   UTIL_FillRectAlpha(gpScreen, x, y, w, h, r, g, b, a);
   UTIL_RectShade(gpScreen, x, y, w, h, 255, 255, 255, 0, 0, 0,
      128, 128, 128);

   gpGeneral->UpdateScreen(x, y, w, h);
}

CBox::~CBox()
{
   if (m_fFakeBox || m_pSavedArea == NULL) {
      return;
   }

   SDL_BlitSurface(m_pSavedArea, NULL, gpScreen, &m_SavedRect);
   SDL_FreeSurface(m_pSavedArea);
   gpGeneral->UpdateScreen(m_SavedRect.x, m_SavedRect.y, m_SavedRect.w, m_SavedRect.h);
}

int CButton::bx[MAX_BUTTONS], CButton::by[MAX_BUTTONS];
int CButton::bw[MAX_BUTTONS], CButton::bh[MAX_BUTTONS];
int CButton::bid[MAX_BUTTONS], CButton::num_buttons = 0;

CButton::CButton(int id, int x, int y, int w, int h, int r, int g, int b):
CBox(x, y, w, h, r, g, b, 128, false),
m_iId(id)
{
   if (num_buttons >= MAX_BUTTONS) {
      TerminateOnError("CButton::CButton(): num_buttons > MAX_BUTTONS!");
   }
   bid[num_buttons] = id;
   bx[num_buttons] = x;
   by[num_buttons] = y;
   bw[num_buttons] = w;
   bh[num_buttons] = h;
   num_buttons++;
}

CButton::~CButton()
{
   int i;
   for (i = 0; i < num_buttons; i++) {
      if (bid[i] == m_iId) {
         break;
      }
   }
   if (i >= num_buttons) {
      TerminateOnError("CButton::~CButton(): i >= num_buttons");
   }

   while (i < num_buttons - 1) {
      bid[i] = bid[i + 1];
      bx[i] = bx[i + 1];
      by[i] = by[i + 1];
      bw[i] = bw[i + 1];
      bh[i] = bh[i + 1];
      i++;
   }

   num_buttons--;
}

int CButton::GetButtonId(int x, int y)
{
   int i;

   for (i = 0; i < num_buttons; i++) {
      if (x > bx[i] && x < bx[i] + bw[i] &&
         y > by[i] && y < by[i] + bh[i]) {
         return bid[i];
      }
   }

   return -1; // not found
}

