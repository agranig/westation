#include <SDL2/SDL.h>

void eventloop_run() {
    int quit = 0;
    SDL_Event e;

    while (!quit) {
        while (SDL_PollEvent(&e)) {
           if (e.type == SDL_QUIT) {
              quit = 1;
           }
        } 
    }
}
