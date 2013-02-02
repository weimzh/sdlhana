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

SDL_Surface *gpScreen = NULL;
bool g_fNoSound = false;

CIniFile cfg;

static char *MakeFileName(const char *fn)
{
   static char str[256];
   char *p = str, *p1;
   while (fn != NULL && *fn != '\0') {
      if (*fn == '~') {
#ifndef _WIN32
         p1 = getenv("HOME");
         while (p1 != NULL && *p1 != '\0') {
            *p++ = *p1++;
         }
#else
         *p++ = '.';
#endif
      } else {
         *p++ = *fn;
      }
      fn++;
   }
   return str;
}

void LoadCfg()
{
   if (cfg.Load(MakeFileName(CONFIG_FILE)) != 0) {
      cfg.Load(DATA_DIR "sdlhana.ini"); // load the default config file
   }
}

void SaveCfg()
{
   cfg.Save(MakeFileName(CONFIG_FILE));
}

static int EventFilter(const SDL_Event *event)
{
   if (event->type == SDL_KEYDOWN) {
      if (event->key.keysym.sym == SDLK_RETURN &&
         event->key.keysym.mod & KMOD_ALT)
      {
         UTIL_ToggleFullScreen();
         return 0;
      } else if (event->key.keysym.sym == SDLK_ESCAPE) {
         // Quit the program immediately if user pressed ESC
         UserQuit();
         return 0;
      }
   } else if (event->type == SDL_QUIT) {
      UserQuit();
      return 0;
   }

   return 1;
}

int main(int argc, char *argv[])
{
   LoadCfg(); // load the configuration file

   // Initialize defaults, video and audio
   if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER | SDL_INIT_NOPARACHUTE) == -1) { 
      fprintf(stderr, "FATAL ERROR: Could not initialize SDL: %s.\n", SDL_GetError());
      exit(1);
   }

   // Initialize the display in a 640x480 24-bit mode
   int f = (atoi(cfg.Get("OPTIONS", "FullScreen", "0")) > 0);
   if (f) {
      f = SDL_FULLSCREEN;
   }

   gpScreen = SDL_SetVideoMode(640, 480, 24, SDL_HWSURFACE | f);

   if (gpScreen == NULL) {
      gpScreen = SDL_SetVideoMode(640, 480, 24, SDL_SWSURFACE | f);
   }

   if (gpScreen == NULL) {
      fprintf(stderr, "FATAL ERROR: Could not set video mode: %s\n", SDL_GetError());
      exit(1);
   }

   SDL_WM_SetCaption(va("SDLHana (%s)", __DATE__), NULL);

   g_fNoSound = (atoi(cfg.Get("OPTIONS", "NoSound", "0")) > 0);

   // Open the audio device
   if (!g_fNoSound) {
      if (SOUND_OpenAudio(22050, AUDIO_S16, 1, 1024)) {
         fprintf(stderr, "WARNING: Couldn't open audio: %s\n", SDL_GetError());
         g_fNoSound = true;
      }
   }

   InitTextMessage();

   gpGeneral = new CGeneral;
   gpGame = new CGame;
   if (gpGeneral == NULL || gpGame == NULL) {
      TerminateOnError("Memory Allocation Error!");
   }

   SDL_SetEventFilter(EventFilter);
   gpGame->MainMenu();

   UserQuit();

   return 255;
}

void UserQuit()
{
   if (gpScreen != NULL)
      SDL_FreeSurface(gpScreen);

   SDL_CloseAudio();
   SDL_Quit();

   FreeTextMessage();
   SaveCfg();

   exit(0);
}

