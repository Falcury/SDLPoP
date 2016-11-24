# Project: SDLPoP

CC = gcc
RM = rm -f

HFILES = common.h config.h data.h proto.h types.h
OBJ = main.o data.o seg000.o seg001.o seg002.o seg003.o seg004.o seg005.o seg006.o seg007.o seg008.o seg009.o seqtbl.o replay.o options.o
BIN = ../prince

MOD_OBJ = main.o data.o seg000.o seg001.o seg002.o seg003.o seg004.o seg005.o seg006.o seg007.o seg008.o seg009.o seqtbl.o replay.o options.o
MOD_BIN = ../sotc

OS      := $(shell uname)

ifeq ($(OS),Darwin)
LIBS := $(shell sdl2-config --libs) -lSDL2_image -lSDL2_mixer -lm
INCS := -I/opt/local/include
CFLAGS += $(INCS) -Wall -std=gnu99 -D_GNU_SOURCE=1 -D_THREAD_SAFE -DOSX -O2
else
LIBS := $(shell pkg-config --libs   sdl2 SDL2_image SDL2_mixer m)
INCS := $(shell pkg-config --cflags sdl2 SDL2_image SDL2_mixer m)
CFLAGS += $(INCS) -Wall -std=gnu99 -O2
endif

all: $(BIN) $(MOD_BIN)
prince: %(BIN)
mod: $(MOD_BIN)

clean:
	$(RM) $(OBJ) $(BIN)

$(BIN): $(OBJ)
	$(CC) $(LDFLAGS) $(OBJ) -o $@ $(LIBS)

$(MOD_BIN): $(MOD_OBJ)
    $(CC) $(LDFLAGS) $(OBJ) -o $@ $(LIBS)

%.o: %.c $(HFILES)
	$(CC) $(CFLAGS) $(LDFLAGS) -c $<

.PHONY: all clean
