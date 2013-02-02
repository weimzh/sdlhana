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

#ifndef MAIN_H_
#define MAIN_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>
#include <math.h>
#include <limits.h>
#include <assert.h>
#ifdef _WIN32
#include <io.h>
#include <windows.h>
#define vsnprintf _vsnprintf
#else
#include <unistd.h>
#endif

#include "SDL.h"

#ifdef WITH_CONFIG_H
#include "config.h"
#endif

#ifndef PREFIX
#define PREFIX "./"
#endif

#ifndef DATA_ROOT
#define DATA_ROOT PREFIX
#endif

#ifndef DATA_DIR
#define DATA_DIR DATA_ROOT "data/"
#endif

#ifndef CONFIG_FILE
#define CONFIG_FILE DATA_DIR "sdlhana.ini"
#endif

#ifndef SOUND_DIR
#define SOUND_DIR DATA_ROOT "sound/"
#endif

#ifndef IMAGES_DIR
#define IMAGES_DIR DATA_ROOT "images/"
#endif

#ifndef FONTS_DIR
#define FONTS_DIR DATA_ROOT "fonts/"
#endif

extern SDL_Surface *gpScreen;
extern bool g_fNoSound;

// main.cpp functions...
void UserQuit();

// util.cpp functions...
void trim(char *str);
char *va(const char *format, ...);
int RandomLong(int from, int to);
float RandomFloat(float from, float to);
int log2(int val);
void TerminateOnError(const char *fmt, ...);

char *UTIL_StrGetLine(const char *buf, int width, int &length);
unsigned int UTIL_GetPixel(SDL_Surface *surface, int x, int y);
void UTIL_PutPixel(SDL_Surface *surface, int x, int y, unsigned int pixel);
int UTIL_GetPixel(SDL_Surface *f, int x, int y, unsigned char *r, unsigned char *g, unsigned char *b);
int UTIL_PutPixel(SDL_Surface *f, int x, int y, unsigned char r, unsigned char g, unsigned char b);
int UTIL_PutPixelAlpha(SDL_Surface *f, int x, int y, unsigned char r, unsigned char g, unsigned char b, unsigned char a);
void UTIL_RevertSurfaceX(SDL_Surface *s);
void UTIL_RevertSurfaceY(SDL_Surface *s);
void UTIL_RevertSurfaceXY(SDL_Surface *s);
SDL_Surface *UTIL_ScaleSurface(SDL_Surface *s, int w, int h);
int UTIL_ScaleBlit(SDL_Surface *src, SDL_Rect *sr, SDL_Surface *dst, SDL_Rect *dr);
void UTIL_Scale2X(SDL_Surface *src, SDL_Surface *dst);
void UTIL_HorzLine(SDL_Surface *surface, short x, short y, short l, unsigned char r, unsigned char g, unsigned char b);
void UTIL_VertLine(SDL_Surface *surface, short x, short y, short l, unsigned char r, unsigned char g, unsigned char b);
void UTIL_Rect(SDL_Surface *surface, int x1, int y1, int w, int h, int r, int g, int b);
void UTIL_VertLineShade(SDL_Surface *surface, short x, short y, short l, unsigned char r1, unsigned char g1, unsigned char b1, unsigned char r2, unsigned char g2, unsigned char b2);
void UTIL_HorzLineShade(SDL_Surface *surface, short x, short y, short l, unsigned char r1, unsigned char g1, unsigned char b1, unsigned char r2, unsigned char g2, unsigned char b2);
void UTIL_RectShade(SDL_Surface *surface, int x, int y, int w, int h, int r1, int g1, int b1, int r2, int g2, int b2, int rt, int gt, int bt);
void UTIL_FillRect(SDL_Surface *surface, int x, int y, int w, int h, int r, int g, int b);
void UTIL_FillRectAlpha(SDL_Surface *surface, int x, int y, int w, int h, int r, int g, int b, int a);
void UTIL_Delay(int duration);
void UTIL_ToggleFullScreen();

// text.cpp functions...
void InitTextMessage();
void FreeTextMessage();
const char *msg(const char *name);

// compress.cpp functions...
int Decode(const char *filename, int headersize, unsigned char *buffer, int bufsize);
int Encode(const char *filename, unsigned char *header, int headersize, unsigned char *buffer, int bufsize);

// config.cpp functions...
void LoadCfg();
void SaveCfg();

// sound.cpp functions...
int SOUND_OpenAudio(int freq, int format, int channels, int samples);
void SOUND_FillAudio(void *udata, unsigned char *stream, int len);
void SOUND_PlayWAV(SDL_AudioCVT *audio);
void SOUND_FreeWAV(SDL_AudioCVT *audio);
SDL_AudioCVT *SOUND_LoadWAV(const char *filename);

#include "ini.h"
#include "font.h"
#include "general.h"
#include "card.h"
#include "player.h"
#include "game.h"
#include "bot.h"

extern CIniFile cfg;

#endif

