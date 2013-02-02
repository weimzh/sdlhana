bin_PROGRAMS = sdlhana
sdlhana_SOURCES = main.cpp bot.cpp card.cpp compress.cpp font.cpp game.cpp \
	general.cpp 	ini.cpp player.cpp text.cpp util.cpp sound.cpp

# set the include path found by configure
AM_CPPFLAGS = $(all_includes) $(LIBSDL_CFLAGS) -DWITH_CONFIG_H=1 -DPREFIX="\"$(prefix)/\"" -DDATA_ROOT="\"$(prefix)/share/sdlhana/\"" -DCONFIG_FILE="\"~/.sdlhana\""

# the library search path.
sdlhana_LDFLAGS = $(all_libraries) $(LIBSDL_RPATH)
sdlhana_LDADD = $(LIBSDL_LIBS)

SUBDIRS = images sound fonts data

