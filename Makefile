
EXT_SRC = $(wildcard external/*.c)
EXT_OBJ = $(EXT_SRC:.c=.o)

INT_SRC = $(wildcard *.c)
INT_OBJ = $(INT_SRC:.c=.o)

# just for lint
INT_H   = $(wildcard *.h)
# maybe someday
TST_SRC = tests/test.c

# todo generate
IFLAGS = -I ./c1k3-assets -I ./c1k3-assets/img -I ./c1k3-assets/audio -I ./external

CFLAGS = -Wall $(IFLAGS) $(shell pkg-config --cflags sdl2 SDL2_mixer SDL2_ttf glesv2)
LFLAGS = -lm $(shell pkg-config --libs sdl2 SDL2_mixer SDL2_ttf glesv2)

# debug junk
SAN_FLAGS = -fsanitize=address -fsanitize=undefined -fno-sanitize-recover=all -fsanitize=float-divide-by-zero -fsanitize=float-cast-overflow -fno-sanitize=null -fno-sanitize=alignment
SAN_OPT = ASAN_OPTIONS=abort_on_error=1:fast_unwind_on_malloc=0:detect_leaks=0 UBSAN_OPTIONS=print_stacktrace=1

DEBUG_CFLAGS = $(CFLAGS) $(SAN_FLAGS) -g
RELEA_CFLAGS = $(CFLAGS) -Os -flto

all: OPT_FLAGS = $(DEBUG_CFLAGS)
all: assets $(INT_OBJ) $(EXT_OBJ)
	$(CC) $(OPT_FLAGS) -o main $(INT_OBJ) $(EXT_OBJ) $(LFLAGS)

release: OPT_FLAGS = $(RELEA_CFLAGS)
release: assets $(INT_OBJ) $(EXT_OBJ)
	$(CC) $(OPT_FLAGS) -o main $(INT_OBJ) $(EXT_OBJ) $(LFLAGS)
	strip main

.NOTPARALLEL:
assets:
	make -C c1k3-assets

%.o: %.c
	$(CC) $(OPT_FLAGS) -c $< -o $@

valbuild:
	$(CC) -Wall -g $(INT_SRC) $(EXT_SRC) -o main $(L_FLAGS)

memtest: valbuild
	valgrind --track-origins=yes --leak-check=yes --gen-suppressions=all --suppressions=extra/suppressions.valg ./main

lint:
	astyle -n $(INT_SRC) $(INT_H)

test:
	## super overkill with the linking and building in all objects
	clang -Wall $(INT_SRC) $(EXT_SRC) tests/*.c -o tester -lSDL2 -lSDL2_mixer -lm -lGLESv2
	./tester

debug: all
	$(SAN_OPT) lldb ./main
