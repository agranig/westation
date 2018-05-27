all: sdl2_test sdl2_ttf icon

clean:
	rm -f sdl2_test sdl2_ttf icon core

sdl2_test:
	g++ -std=c++0x -Wall -pedantic sdl2_test.cpp -o sdl2_test `sdl2-config --cflags --libs` -lSDL2_image

sdl2_ttf:
	g++ -std=c++0x -Wall -pedantic sdl2_ttf.cpp -o sdl2_ttf `sdl2-config --cflags --libs` -lSDL2_image -lSDL2_ttf

icon:
	g++ -std=c++0x -Wall -pedantic icon.cpp -o icon `sdl2-config --cflags --libs` -lSDL2_image -lSDL2_ttf
