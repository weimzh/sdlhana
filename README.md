INTRODUCTION
===========

SDLHana is an SDL-based Hanafuda game. Hanafuda is a Japanese-oriented
card game which is mostly played in Japan and Korea, also known as
"Hwa-T'u" in Korean.

For more details about Hanafuda, see
[Wikipedia](http://en.wikipedia.org/wiki/Hanafuda).

Enjoy playing.


INSTALLATION
===========

If you are installing with the source package, make sure you have the SDL
runtime and development files correctly installed. Otherwise the source
code will fail to compile. You may download it at http://www.libsdl.org/,
or the official software repository of any GNU/Linux distributions.

Get the latest source code:

```
git clone https://gitlab.com/weimzh/sdlhana
```

Compile and install (for GNU/Linux, maybe for other UNIX-like OSes as well):

```
cd sdlhana
autoreconf -f -i
./configure
make
make install
```
