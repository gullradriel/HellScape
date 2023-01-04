RM=rm -f
CC=gcc
EXT=
CLIBS=

#OPT=-W -Wall -D_XOPEN_SOURCE=600 -D_XOPEN_SOURCE_EXTENTED -std=gnu99 -O3
OPT=-W -Wall -D_XOPEN_SOURCE=600 -D_XOPEN_SOURCE_EXTENTED -std=gnu99 -g


VPATH=LIB/src/
INCLUDE=LIB/include

#CLIBS=`pkg-config --cflags --libs allegro-5 allegro_acodec-5 allegro_audio-5 allegro_color-5 allegro_dialog-5 allegro_font-5 allegro_image-5 allegro_main-5 allegro_memfile-5 allegro_physfs-5 allegro_primitives-5 allegro_ttf-5`

#ALLEGRO_LIBS=-lallegro_acodec -lallegro_audio -lallegro_color -lallegro_dialog -lallegro_image -lallegro_main -lallegro_memfile -lallegro_physfs -lallegro_primitives -lallegro_ttf -lallegro_font -lallegro

ALLEGRO_LIBS=-lallegro_acodec -lallegro_audio -lallegro_color -lallegro_image -lallegro_main -lallegro_primitives -lallegro_ttf -lallegro_font -lallegro
LIBNILOREA=-lnilorea64
CFLAGS+= -DALLEGRO_UNSTABLE

dir_name=$(shell date +%Y_%m_%d_%HH%MM%SS )

ifeq ($(OS),Windows_NT)
    CFLAGS+= -I$(INCLUDE) -D__USE_MINGW_ANSI_STDIO $(OPT)
	RM= del /Q
    CC= gcc
	ifeq (${MSYSTEM},MINGW32)
        RM=rm -f
        CFLAGS+= -m32
        EXT=.exe
        LIBNILOREA=-lnilorea32
        CLIBS=-IC:/msys64/mingw32/include -LC:/msys64/mingw32/lib
    endif
    ifeq (${MSYSTEM},MINGW64)
        RM=rm -f
        CFLAGS+= -DARCH64BITS
        EXT=.exe
        LIBNILOREA=-lnilorea64
        CLIBS=-IC:/msys64/mingw64/include -LC:/msys64/mingw64/lib
    endif
	ifeq (${MSYSTEM},MINGW64CB)
        RM=del /Q
        CFLAGS+= -DARCH64BITS
        EXT=.exe
        LIBNILOREA=-lnilorea64
        CLIBS=-IC:/msys64/mingw64/include -LC:/msys64/mingw64/lib
    endif
    CLIBS+= $(ALLEGRO_LIBS) -Wl,-Bstatic -lpthread  -Wl,-Bdynamic -lws2_32  -L../LIB/. #-mwindows
else
	LIBNILOREA=-lnilorea
	UNAME_S= $(shell uname -s)
	RM=rm -f
	CC=gcc
	EXT=
    ifeq ($(UNAME_S),Linux)
        CFLAGS+= -I$(INCLUDE) $(OPT)
        CLIBS+= $(ALLEGRO_LIBS) -lpthread -lm -no-pie
    endif
    ifeq ($(UNAME_S),SunOS)
        CC=cc
        CFLAGS+= -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64 -g -v -xc99 -I ../../LIB/include/ -mt -lm
        CLIBS+= $(ALLEGRO_LIBS) -lm -lsocket -lnsl -lpthread -lrt -L..
    endif
endif


SRC=n_common.c n_log.c n_str.c n_list.c cJSON.c states_management.c n_fluids.c HellScape.c
OBJ=$(SRC:%.c=%.o)
.c.o:
	$(COMPILE.c) $<

%.o:%.c
	$(CC) $(CFLAGS) -o $@ -c $<

HellScape$(EXT): $(OBJ)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(CLIBS)

all: HellScape$(EXT)

clean:
	$(RM) *.o
	$(RM) HellScape$(EXT)
