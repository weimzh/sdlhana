/* fntgen.cpp

   Utility to generate a font file used by the game from
   a TrueType font.

   Copyright (c) 2006, Wei Mingzhi. All Rights Reserved.

   This program is free software; the author gives unlimited
   permission to copy, distribute and modify it.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <iostream>
using namespace std;

#include "SDL.h"
#include "SDL_ttf.h"

#include "font.h"

int Encode(const char *filename, unsigned char *header, int headersize, unsigned char *buffer, int bufsize);

void
trim(char *str)
{
  int pos = 0;
  char *dest = str;

  /* skip leading blanks */
  while (str[pos] <= ' ' && str[pos] > 0)
    {
      pos++;
    }

  while (str[pos])
    {
      *(dest++) = str[pos];
      pos++;
    }

  *(dest--) = '\0'; /* store the null */

  /* remove trailing blanks */
  while (dest >= str && *dest <= ' ' && *dest > 0)
    {
      *(dest--) = '\0';
    }
}

/* Return the pixel value at (x, y) */
/* NOTE: The surface must be locked before calling this! */
unsigned long
UTIL_GetPixel (SDL_Surface *surface, int x, int y)
{
  int bpp = surface->format->BytesPerPixel;

  /* Here p is the address to the pixel we want to retrieve */
  unsigned char *p = (unsigned char *)surface->pixels + y * surface->pitch + x * bpp;

  switch (bpp)
    {
    case 1:
      return *p;

    case 2:
      return *(unsigned short *)p;

    case 3:
      if (SDL_BYTEORDER == SDL_BIG_ENDIAN)
        {
          return p[0] << 16 | p[1] << 8 | p[2];
        }
      else
        {
          return p[0] | p[1] << 8 | p[2] << 16;
        }

    case 4:
      return *(unsigned long *)p;
    }

  return 0; /* shouldn't happen, but avoids warnings */
}

int UTIL_PutPixel(SDL_Surface *f, int x, int y,
                  unsigned char r, unsigned char g, unsigned char b)
{
  unsigned long pixel;
  unsigned char *pp;

  int n;

  if (f == NULL)
    return -1;

  pp = (unsigned char *)f->pixels;

  if (x >= f->w || y >= f->h)
    return -1;

  pp += (f->pitch * y);
  pp += (x * f->format->BytesPerPixel);

  pixel = SDL_MapRGB(f->format, r, g, b);

  for (n = 0; n < f->format->BytesPerPixel; ++n, ++pp)
    {
      *pp = (unsigned char)(pixel & 0xFF);
      pixel >>= 8;
    }

  return 0;
}

int UTIL_GetPixelRGB (SDL_Surface *f, int x, int y,
                      unsigned char *r, unsigned char *g, unsigned char *b)
{
  unsigned long pixel;
  unsigned char *pp;

  int n; /* general purpose 'n'. */

  if (f == NULL)
    return -1;

  pp = (unsigned char *)f->pixels;

  if (x >= f->w || y >= f->h)
    return -1;

  pp += (f->pitch * y);
  pp += (x * f->format->BytesPerPixel);

  /* We do not lock the surface here, it would be inefficient.
     This reads the pixel as though it was a big-endian integer.
     I'm trying to avoid reading part the end of the pixel data by
     using a data-type that's larger than the pixels */
  for (n = 0, pixel = 0; n < f->format->BytesPerPixel; ++n, ++pp)
    {
#if SDL_BYTEORDER == SDL_LIL_ENDIAN
      pixel >>= 8;
      pixel |= *pp << (f->format->BitsPerPixel - 8);
#else
      pixel |= *pp;
      pixel <<= 8;
#endif
    }

  SDL_GetRGB (pixel, f->format, r, g, b);
  return 0;
}

SDL_Surface *
UTIL_ScaleSurface (SDL_Surface *s, int w, int h)
{
  SDL_Surface *r = SDL_CreateRGBSurface (s->flags,
    w, h, s->format->BitsPerPixel, s->format->Rmask,
    s->format->Gmask, s->format->Bmask, s->format->Amask);

  SDL_Rect dstrect, dstrect2;

  dstrect.x = 0;
  dstrect.y = 0;
  dstrect.w = w;
  dstrect.h = h;

  dstrect2.x = 0;
  dstrect2.y = 0;
  dstrect2.w = s->w;
  dstrect2.h = s->h;

  SDL_SoftStretch (s, &dstrect2, r, &dstrect);
  return r;
}

int
convert (const char *input_file, const char *output_file, const char *ttf_file)
{
  FILE *input;
  SDL_Surface *tmp_surface, *surface;
  TTF_Font *f;
  char buf[256], *p, sz[4];
  unsigned int i, j, k, rmask, gmask, bmask, amask;
  fntfile_header_t fntheader;
  fntchar_t fntchar, *chars;
  SDL_Color textcolor = {255, 255, 255, 0};
  union
    {
      unsigned char c[4];
      unsigned long i;
    } char_code;

  rmask = 0x000000ff;
  gmask = 0x0000ff00;
  bmask = 0x00ff0000;
  amask = 0xff000000;

  /* load the TrueType font */
  f = TTF_OpenFont (ttf_file, 64);
  if (f == NULL)
    {
      cerr << "FATAL ERROR: cannot open font %s: "
           << ttf_file << SDL_GetError() << endl;
      return 1;
    }

  /* open the input file */
  input = fopen (input_file, "r");
  if (input == NULL)
    {
      cerr << "FATAL ERROR: cannot open input file "
           << input_file << endl;
      return 1;
    }

  /* get the number of characters */
  fgets (buf, 256, input);
  trim (buf);

  /* set the magic number and the number of characters of the header */
  strcpy (fntheader.magic, "FNT");
  fntheader.num_chars = atoi (buf);

  if (fntheader.num_chars <= 0)
    {
      cerr << "Invalid input file" << endl;
      return 1;
    }

  chars = (fntchar_t *) malloc (sizeof (fntchar_t) * fntheader.num_chars);
  if (chars == NULL)
    {
      cerr << "Memory allocation error!" << endl;
      return 1;
    }

  for (i = 0; i < fntheader.num_chars; )
    {
      if (feof (input))
        {
          cerr << "WARNING: unexpected end of file found" << endl;
          break;
        }

      fgets (buf, 256, input);
      trim (buf);

      p = buf;
      while (*p != '\0')
        {
          char_code.c[0] = *p++;
          if (char_code.c[0] & 0x80 && *p != '\0')
            {
              /* This is a multibyte character */
              char_code.c[1] = *p++;
              if ((char_code.c[0] & 0xF0) == 0xE0)
                {
                  char_code.c[2] = *p++;
                }
              else
                {
                  char_code.c[2] = '\0';
                }
              char_code.c[3] = '\0';
            }
          else
            {
              /* This is a standard ASCII character */
              char_code.c[1] = '\0';
              char_code.c[2] = '\0';
              char_code.c[3] = '\0';
            }

          fntchar.code = char_code.i;

          sz[0] = char_code.c[0];
          sz[1] = char_code.c[1];
          sz[2] = char_code.c[2];
          sz[3] = '\0';

          /* render the character */
          tmp_surface = TTF_RenderUTF8_Blended(f, sz, textcolor);
          surface = UTIL_ScaleSurface(tmp_surface, sz[2] ? 64 : 32, 64);

          for (j = 0; j < 64; j++)
            {
              for (k = 0; k < 8; k++)
                {
                  fntchar.pixeldata[j][k] = 0;
                }
            }

          for (j = 0; j < 64; j++)
            {
              for (k = 0; k < (sz[1] ? 64 : 32); k++)
                {
                  if (UTIL_GetPixel (surface, k, j) & surface->format->Amask)
                    {
                      fntchar.pixeldata[j][k / 8] |= (1 << (k & 7));
                    }
                }
            }

#ifdef _DEBUG
          cout << "\n[" << sz << endl;
          for (j = 0; j < 64; j++)
            {
              for (k = 0; k < (sz[1] ? 64 : 32); k++)
                {
                  if (fntchar.pixeldata[j][k / 8] & (1 << (k % 8)))
                    {
                      cout << "*";
                    }
                  else
                    {
                      cout << " ";
                    }
                }
              cout << endl;
            }
#endif

          chars[i] = fntchar;

          if (++i >= fntheader.num_chars)
            {
              break;
            }

          SDL_FreeSurface(tmp_surface);
          SDL_FreeSurface(surface);
        }
    }

  /* after process, close everything */
  TTF_CloseFont(f);
  fclose(input);

  /* write to the output file */
  if (Encode (output_file, (unsigned char *)&fntheader,
     sizeof (fntfile_header_t), (unsigned char *)chars,
     sizeof (fntchar_t) * fntheader.num_chars) <= 0)
    {
      cerr << "cannot write to output file " << output_file << endl;
      free (chars);
      return 1;
    }

  free (chars);
  return 0;
}

int
main (int argc, char *argv[])
{
  if (argc != 4)
    {
      cout << "usage: fntgen [input_file.txt] [output_file.fnt] [ttf_file.ttf]" << endl;
      return 2;
    }

  /* Initialize the SDL library */
  if (SDL_Init (SDL_INIT_VIDEO | SDL_INIT_NOPARACHUTE) == -1)
    {
      cerr << "FATAL ERROR: Could not initialize SDL: " << SDL_GetError() << endl;
      return 255;
    }

  /* Initialize the TTF library */
  if (TTF_Init () < 0)
    {
      SDL_Quit ();
      cerr << "FATAL ERROR: Couldn't initialize TTF: " << SDL_GetError() << endl;
      return 254;
    }

  if (convert (argv[1], argv[2], argv[3]) != 0)
    {
      SDL_Quit ();
      TTF_Quit ();
      return 1;
    }

  SDL_Quit ();
  TTF_Quit ();
  return 0;
}

