EXT_SRC = $(wildcard external/*/*.c)
EXT_OBJ = $(EXT_SRC:.c=.o)

INT_SRC = $(wildcard *.c)
INT_OBJ = $(INT_SRC:.c=.o)

# just for lint
INT_H   = $(wildcard *.h)
# maybe someday
TST_SRC = tests/test.c

BIN_NAM = c1k3

# todo generate
IFLAGS  = -I./c1k3-assets -I./c1k3-assets/img -I./c1k3-assets/audio -I./c1k3-assets/ttf -I./c1k3-assets/blend
IFLAGS += -I./external/lodepng -I./external/libdsa -I./external/mpack -I./external/cgltf
IFLAGS += -I./tools

# todo -Wextra
CFLAGS = -Wall $(IFLAGS) $(shell sdl2-config --cflags) $(shell pkg-config --cflags SDL2_mixer SDL2_ttf) -std=c11 -pedantic
LFLAGS = -lm $(shell sdl2-config --libs) $(shell pkg-config --libs SDL2_mixer SDL2_ttf)

UNAME   := $(shell uname)
WIN_STR := MINGW64_NT
LIN_STR := Linux
MAC_STR := Darwin
OS := none

ifeq ($(findstring $(LIN_STR), $(UNAME)), $(LIN_STR))
	CFLAGS += $(shell pkg-config --cflags glesv2)
	LFLAGS += $(shell pkg-config --libs glesv2)
	OS := linux
else ifeq ($(findstring $(WIN_STR), $(UNAME)), $(WIN_STR))
	LFLAGS += -lGLESv2
	OS := windows
else ifeq ($(findstring $(MAC_STR), $(UNAME)), $(MAC_STR))
	LFLAGS += -lGLESv2
	OS := mac
endif

# debug junk
SAN_FLAGS = -fsanitize=address -fsanitize=undefined -fno-sanitize-recover=all -fsanitize=float-divide-by-zero -fsanitize=float-cast-overflow -fno-sanitize=null -fno-sanitize=alignment
SAN_OPT = ASAN_OPTIONS=abort_on_error=1:fast_unwind_on_malloc=0:detect_leaks=0 UBSAN_OPTIONS=print_stacktrace=1

DEBUG_CFLAGS = $(CFLAGS) -g
DBGSN_CFLAGS = $(DEBUG_CFLAGS) $(SAN_FLAGS)
RELEA_CFLAGS = $(CFLAGS) -flto -Os

all: OPT_FLAGS = $(DBGSN_CFLAGS)
all: assets $(INT_OBJ) $(EXT_OBJ)
	$(CC) $(OPT_FLAGS) -o $(BIN_NAM) $(INT_OBJ) $(EXT_OBJ) $(LFLAGS)

release: OPT_FLAGS = $(RELEA_CFLAGS)
release: assets $(INT_OBJ) $(EXT_OBJ)
	$(CC) $(OPT_FLAGS) -o $(BIN_NAM) $(INT_OBJ) $(EXT_OBJ) $(LFLAGS)

package: release
	mkdir -p build/$(OS)
	cp $(BIN_NAM) build/$(OS)/
ifeq ($(OS),linux)
	tar czf $(BIN_NAM)-$(OS).tar.gz -C build/$(OS)/ .
else ifeq ($(OS),windows)
	cp /ucrt64/bin/libbrotlicommon.dll build/$(OS)/
	cp /ucrt64/bin/libbrotlidec.dll    build/$(OS)/
	cp /ucrt64/bin/libbz2-1.dll        build/$(OS)/
	cp /ucrt64/bin/libEGL.dll          build/$(OS)/
	cp /ucrt64/bin/libfreetype-6.dll   build/$(OS)/
	cp /ucrt64/bin/libgcc_s_seh-1.dll  build/$(OS)/
	cp /ucrt64/bin/libGLESv2.dll       build/$(OS)/
	cp /ucrt64/bin/libglib-2.0-0.dll   build/$(OS)/
	cp /ucrt64/bin/libgraphite2.dll    build/$(OS)/
	cp /ucrt64/bin/libharfbuzz-0.dll   build/$(OS)/
	cp /ucrt64/bin/libiconv-2.dll      build/$(OS)/
	cp /ucrt64/bin/libintl-8.dll       build/$(OS)/
	cp /ucrt64/bin/libmpg123-0.dll     build/$(OS)/
	cp /ucrt64/bin/libogg-0.dll        build/$(OS)/
	cp /ucrt64/bin/libopus-0.dll       build/$(OS)/
	cp /ucrt64/bin/libopusfile-0.dll   build/$(OS)/
	cp /ucrt64/bin/libpcre2-8-0.dll    build/$(OS)/
	cp /ucrt64/bin/libpng16-16.dll     build/$(OS)/
	cp /ucrt64/bin/libstdc++-6.dll     build/$(OS)/
	cp /ucrt64/bin/libwinpthread-1.dll build/$(OS)/
	cp /ucrt64/bin/SDL2.dll            build/$(OS)/
	cp /ucrt64/bin/SDL2_mixer.dll      build/$(OS)/
	cp /ucrt64/bin/SDL2_ttf.dll        build/$(OS)/
	cp /ucrt64/bin/zlib1.dll           build/$(OS)/
	zip -j -r $(BIN_NAM)-$(OS).zip build/$(OS)
endif

valbuild: OPT_FLAGS = $(DEBUG_CFLAGS)
valbuild: assets $(INT_OBJ) $(EXT_OBJ)
	$(CC) $(OPT_FLAGS) -o $(BIN_NAM) $(INT_OBJ) $(EXT_OBJ) $(LFLAGS)

memtest: valbuild
	valgrind --track-origins=yes --leak-check=yes --gen-suppressions=all --suppressions=valgrind.supp ./$(BIN_NAM)

.NOTPARALLEL:
assets: tools/mapc
	make -C c1k3-assets

##tools/mapc: OPT_FLAGS = $(DBGSN_CFLAGS)
tools/mapc: OPT_FLAGS = $(RELEA_CFLAGS)
tools/mapc: $(EXT_OBJ)
	$(CC) $(OPT_FLAGS) tools/mapc.c -o tools/mapc $(EXT_OBJ) $(LFLAGS)

%.o: %.c
	$(CC) $(OPT_FLAGS) -c $< -o $@

lint:
	astyle -n $(INT_SRC) $(INT_H)

test:
	## super overkill with the linking and building in all objects
	clang -Wall $(INT_SRC) $(EXT_SRC) tests/*.c -o tester -lSDL2 -lSDL2_mixer -lm -lGLESv2
	./tester

debug: all
	$(SAN_OPT) lldb ./$(BIN_NAM)

clean:
	rm $(INT_OBJ)
	rm $(EXT_OBJ)
