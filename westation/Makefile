all:
	gcc -g -o westation main.c weather.c display.c eventloop.c `sdl2-config --cflags --libs` -lSDL2_image -lSDL2_ttf -lcurl -ljson-c -lpthread

clean:
	rm -f westation
