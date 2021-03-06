#include <SDL2/SDL.h>

#include "display.h"

void eventloop_run() {
    int quit = 0;
    SDL_Event e;

    while (!quit) {
        while (SDL_PollEvent(&e)) {
           if (e.type == SDL_QUIT ||
               (e.type == SDL_KEYUP && e.key.keysym.sym == SDLK_q)) {
              quit = 1;
           }
        }
        display_process_queue();
        SDL_Delay(100);
    }
}
