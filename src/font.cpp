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

CFont::CFont():
m_iNumChar(0),
m_pChars(NULL)
{
}

CFont::CFont(const char *filename):
m_iNumChar(0),
m_pChars(NULL)
{
   Load(filename);
}

CFont::~CFont()
{
   FreeAllTheStuff();
}

int CFont::Load(const char *filename)
{
   if (IsLoaded()) {
      FreeAllTheStuff();
   }

   FILE *fp = fopen(filename, "rb");
   if (fp == NULL) {
      TerminateOnError("Cannot load font file %s", filename);
   }

   fntfile_header_t header;

   fread(&header, 1, sizeof(fntfile_header_t), fp);

   if (strcmp(header.magic, "FNT") != 0) {
      fclose(fp);
      TerminateOnError("Invalid font file %s", filename);
   }

   m_iNumChar = header.num_chars;

   m_pChars = (fntchar_t *)malloc(sizeof(fntchar_t) * header.num_chars);
   if (m_pChars == NULL) {
      TerminateOnError("Memory allocation Error!");
   }

   fclose(fp);

   // Decompress the font data
   Decode(filename, sizeof(fntfile_header_t), (unsigned char *)m_pChars, sizeof(fntchar_t) * header.num_chars);

   Sort(); // sort the font data
   return 0;
}

SDL_Surface *CFont::Render(const char *sz, int r, int g, int b, int size, bool shadow)
{
   int length = 0, i, j, cur = 0;
   const char *p = sz;
   SDL_Surface *s = NULL;

   while (*p) {
      if (*p < 0) {
         p += 3;
         length += 2;
      } else {
         p++;
         length++;
      }
   }

   s = SDL_CreateRGBSurface((gpScreen->flags & (~SDL_HWSURFACE)) | SDL_SRCALPHA,
      size / 2 * (length + 2), size, gpScreen->format->BitsPerPixel,
      gpScreen->format->Rmask, gpScreen->format->Gmask,
      gpScreen->format->Bmask, gpScreen->format->Amask);

   SDL_SetColorKey(s, SDL_SRCCOLORKEY, SDL_MapRGBA(s->format, 0, 0, 0, 0));
   SDL_FillRect(s, NULL, SDL_MapRGBA(s->format, 0, 0, 0, 0));

   // HACKHACK: to make black color not transparent
   if (r == 0 && g == 0 && b == 0) {
      r = 1;
   }

   p = sz;
   while (*p) {
      union {
         unsigned int i;
         char c[4];
      } code;

      int bb = 1;
      if ((unsigned char)*p & 0x80) {
         // This is an multi-byte character
         // FIXME: four-or-more-byte sequences not supported
         if (((unsigned char)*p & 0xE0) == 0xC0) {
            // Two-byte sequence.
            code.c[0] = *p++;
            code.c[1] = *p++;
            code.c[2] = '\0';
            code.c[3] = '\0';
            bb = 2;
         } else if (((unsigned char)*p & 0xF0) == 0xE0) {
            // Three-byte sequence.
            code.c[0] = *p++;
            code.c[1] = *p++;
            code.c[2] = *p++;
            code.c[3] = '\0';
            bb = 3;
         } else {
            TerminateOnError("CFont::Render(): four-or-more-byte sequence not supported");
         }
      } else {
         // This is a normal ASCII character
         code.c[0] = *p++;
         code.c[1] = code.c[2] = code.c[3] = '\0';
         bb = 1;
      }

      fntchar_t *c = FindChar(code.i);

      if (c != NULL) {
         SDL_Rect dstrect;

         dstrect.x = cur;
         dstrect.y = 0;
         dstrect.w = size;
         dstrect.h = size;

         SDL_Surface *char_surface = SDL_CreateRGBSurface(s->flags,
            (shadow ? 66 : 64), (shadow ? 66 : 64),
            gpScreen->format->BitsPerPixel,
            gpScreen->format->Rmask, gpScreen->format->Gmask,
            gpScreen->format->Bmask, gpScreen->format->Amask);

         SDL_SetColorKey(char_surface, SDL_SRCCOLORKEY, SDL_MapRGBA(s->format, 0, 0, 0, 0));
         SDL_FillRect(char_surface, NULL, SDL_MapRGBA(char_surface->format, 0, 0, 0, 0));

         if (shadow) {
            for (i = 0; i < 64; i++) {
               for (j = 0; j < 64; j++) {
                  if (c->pixeldata[i][j / 8] & (1 << (j & 7))) {
                     UTIL_PutPixel(char_surface, j + 2, i + 2, SDL_MapRGBA(char_surface->format, 1, 1, 1, 255));
                  }
               }
            }
         }

         for (i = 0; i < 64; i++) {
            for (j = 0; j < 64; j++) {
               if (c->pixeldata[i][j / 8] & (1 << (j & 7))) {
                  UTIL_PutPixel(char_surface, j, i, SDL_MapRGBA(char_surface->format, r, g, b, 255));
               }
            }
         }

         SDL_SoftStretch(char_surface, NULL, s, &dstrect);
         SDL_FreeSurface(char_surface);
      }

      cur += ((bb == 3) ? size : size / 2);
   }

   return s;
}

void CFont::Sort(fntchar_t *begin, fntchar_t *end)
{
   if (begin == NULL && end == NULL) {
      begin = m_pChars;
      end = &m_pChars[m_iNumChar - 1];
   }

   if (end <= begin)
      return;

   fntchar_t pivot = *begin, *a = begin, *b = end;

   while (a < b) {
      while (a < b && *b >= pivot)
         b--;
      *a = *b;
      while (a < b && *a <= pivot)
         a++;
      *b = *a;
   }

   *a = pivot;

   Sort(begin, a - 1);
   Sort(a + 1, end);
}

fntchar_t *CFont::FindChar(unsigned int code)
{
   int low = 0, high = m_iNumChar - 1;

   while (low <= high) {
      int mid = (low + high) / 2;
      if (m_pChars[mid].code < code) {
         low = mid + 1;
      } else if (m_pChars[mid].code > code) {
         high = mid - 1;
      } else {
         return &m_pChars[mid];
      }
   }

   return NULL; // not found
}

void CFont::FreeAllTheStuff()
{
   if (m_pChars != NULL)
      free(m_pChars);
   m_pChars = NULL;
   m_iNumChar = 0;
}

