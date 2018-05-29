all:
	gcc -o westation main.c weather.c display.c eventloop.c `sdl2-config --cflags --libs` -lSDL2_image -lSDL2_ttf -lcurl -ljson-c 

clean:
	rm -f westation
