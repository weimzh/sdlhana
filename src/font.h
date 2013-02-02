/* Copyright (c) 2005, 2006 Wei Mingzhi <whistler@openoffice.org>
   All Rights Reserved.

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of
   the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
   02110-1301, USA.
*/

#ifndef FNTFILE_H
#define FNTFILE_H

typedef struct fntchar_s
{
   unsigned int     code; /* internal code of the character (in UTF-8) */
   unsigned char    pixeldata[64][8]; /* pixel data of this character (64x64) */
#ifdef __cplusplus
   bool operator<(const fntchar_s &a) { return code < a.code; }
   bool operator<=(const fntchar_s &a) { return code <= a.code; }
   bool operator>(const fntchar_s &a) { return code > a.code; }
   bool operator>=(const fntchar_s &a) { return code >= a.code; }
#endif
} fntchar_t;

typedef struct fntfile_header_s
{
   char             magic[4]; /* magic code, must be "FNT\0" */
   unsigned short   num_chars; /* number of characters */
} fntfile_header_t;

#ifdef __cplusplus

class CFont
{
public:
   CFont();
   CFont(const char *filename);
   ~CFont();

   inline bool  IsLoaded() { return m_iNumChar > 0; }

   int          Load(const char *filename);
   SDL_Surface *Render(const char *sz, int r = 255, int g = 255, int b = 255, int size = 32, bool shadow = true);

private:
   void         Sort(fntchar_t *begin = NULL, fntchar_t *end = NULL);
   void         FreeAllTheStuff();
   fntchar_t   *FindChar(unsigned int code);

   int          m_iNumChar;
   fntchar_t   *m_pChars;
};

#endif

#endif

