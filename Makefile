
EXT_SRC = lodepng.c
MAIN_C  = main.c
INT_SRC = data.c entity.c entity_barrel.c entity_light.c entity_particle.c entity_player.c game.c input.c map.c math.c model.c render.c
INT_H   = data.h entity.h entity_barrel.h entity_light.h entity_particle.h entity_player.h game.h input.h map.h math.h model.h render.h
TST_SRC = tests/test.c

all:
	clang -Wall -g -I/usr/include/ $(MAIN_C) $(INT_SRC) $(EXT_SRC) -o main -lm -lGLESv2 -lSDL2

release:
	clang -Os -flto -Wall $(INT_SRC) $(EXT_SRC) -o main -lm -lGLESv2 -lSDL2
	strip main

memtest:
	valgrind --track-origins=yes --leak-check=yes --suppressions=extra/suppressions.valg ./main

lint:
	astyle -n $(MAIN_C) $(INT_SRC) $(INT_H) $(TST_SRC)

test:
	## super overkill with the linking and building in all objects
	clang -Wall $(INT_SRC) $(EXT_SRC) tests/*.c -o tester -lSDL2 -lm -lGLESv2
	./tester

debug: all
	lldb ./main
