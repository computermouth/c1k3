
EXT_SRC = lodepng.c
INT_SRC = data.c main.c map.c math.c model.c render.c
INT_H   = data.h map.h math.h model.h render.h

all:
	clang -Wall -I/usr/include/ $(INT_SRC) $(EXT_SRC) -o main -lm -lGLESv2

release:
	clang -Os -flto -Wall $(INT_SRC) $(EXT_SRC) -o main -lm -lGLESv2
	strip main

memtest:
	valgrind --track-origins=yes --leak-check=yes ./main

lint:
	astyle -n $(INT_SRC) $(INT_H)
