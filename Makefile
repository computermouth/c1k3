
EXT_SRC = lodepng.c
INT_SRC = data.c main.c map.c math.c render.c
INT_H   = data.h map.h math.h render.h

all:
	clang -Wall $(INT_SRC) $(EXT_SRC) -o main -lm -lGLESv2

memtest:
	valgrind --track-origins=yes --leak-check=yes ./main

lint:
	astyle -n $(INT_SRC) $(INT_H)