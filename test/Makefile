all:
	g++ -std=c++0x -Wall -pedantic sdl2_test.cpp -o sdl2_test `sdl2-config --cflags --libs` -lSDL2_image
	g++ -std=c++0x -Wall -pedantic sdl2_ttf.cpp -o sdl2_ttf `sdl2-config --cflags --libs` -lSDL2_image -lSDL2_ttf

clean:
	rm -f sdl2_test sdl2_ttf core
