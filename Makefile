
all:
	clang -Wall main.c math.c render.c data.c map.c lodepng.c -o main -lm -lGLESv2

memtest:
	valgrind --track-origins=yes --leak-check=yes ./main