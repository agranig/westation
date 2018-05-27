 #include <SDL2/SDL.h>
 #include <SDL2/SDL_image.h>
 #include <SDL2/SDL_ttf.h>
 #include <iostream>
 
 void check_error_sdl(bool check, const char* message);
 void check_error_sdl_img(bool check, const char* message);
 void check_error_sdl_ttf(bool check, const char* message);
 
 SDL_Texture* load_texture(const char* fname, SDL_Renderer *renderer);
 
 
 int main(int argc, char** argv) {
     check_error_sdl(SDL_Init(SDL_INIT_VIDEO) != 0, "Unable to initialize SDL");
     SDL_ShowCursor(SDL_DISABLE);
 
     SDL_Window* window = SDL_CreateWindow("icon", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                                           480, 320, SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL);
     check_error_sdl(window == nullptr, "Unable to create window");
 
     SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
     check_error_sdl(renderer == nullptr, "Unable to create a renderer");
 
     SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
 
     int flags = IMG_INIT_PNG;
     check_error_sdl_img((IMG_Init(flags) & flags) != flags, "Unable to initialize SDL_image");
 
     SDL_Texture* icon_texture = load_texture("sun.png", renderer);
     SDL_Rect icon_rect;
     check_error_sdl(SDL_QueryTexture(icon_texture, NULL, NULL, &icon_rect.w, &icon_rect.h) != 0,
          "Unable to get icon size");
     icon_rect.x = (480 - icon_rect.w) / 2;
     icon_rect.y = (320 - icon_rect.h) / 2;
 
     SDL_RenderClear(renderer);
     SDL_RenderCopy(renderer, icon_texture, NULL, &icon_rect);

     check_error_sdl_ttf(TTF_Init() != 0, "Unable to initialize SDL_ttf");
     TTF_Font* font = TTF_OpenFont("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", 24);
     check_error_sdl_ttf(font == nullptr, "Unable to load font");

     SDL_Color font_color = {0, 0, 0};
     const char *text = "18dC";
     SDL_Surface* icon_surface = TTF_RenderText_Solid(font, text, font_color);
     check_error_sdl_ttf(icon_surface == nullptr, "Unable to render text");

     SDL_Texture* icon_message = SDL_CreateTextureFromSurface(renderer, icon_surface);
     check_error_sdl_ttf(icon_message == nullptr, "Unable to create texture");

     SDL_Rect message_rect;
     check_error_sdl_ttf(TTF_SizeText(font, text, &message_rect.w, &message_rect.h),
          "Failed to get text size");
     message_rect.x = (480 - message_rect.w) / 2;
     message_rect.y = (320 - message_rect.h + icon_rect.h) / 2 + 20;

     SDL_RenderCopy(renderer, icon_message, NULL, &message_rect);
 
     SDL_RenderPresent(renderer);
 
     SDL_Delay(10000);
 
     SDL_DestroyTexture(icon_texture);
     IMG_Quit();
     SDL_DestroyTexture(icon_message);
     TTF_Quit();
     SDL_DestroyRenderer(renderer);
     SDL_DestroyWindow(window);
     SDL_Quit();
 
     return 0;
 }
 
 // In case of error, print the error code and close the application
 void check_error_sdl(bool check, const char* message) {
     if (check) {
         std::cout << message << " " << SDL_GetError() << std::endl;
         SDL_Quit();
         std::exit(-1);
     }
 }
 
 // In case of error, print the error code and close the application
 void check_error_sdl_img(bool check, const char* message) {
     if (check) {
         std::cout << message << " " << IMG_GetError() << std::endl;
         IMG_Quit();
         SDL_Quit();
         std::exit(-1);
     }
 }
 
 // In case of error, print the error code and close the application
 void check_error_sdl_ttf(bool check, const char* message) {
     if (check) {
         std::cout << message << " " << TTF_GetError() << std::endl;
         TTF_Quit();
         IMG_Quit();
         SDL_Quit();
         std::exit(-1);
     }
 }

 // Load an image from "fname" and return an SDL_Texture with the content of the image
 SDL_Texture* load_texture(const char* fname, SDL_Renderer *renderer) {
     SDL_Surface *image = IMG_Load(fname);
     check_error_sdl_img(image == nullptr, "Unable to load image");
 
     SDL_Texture *img_texture = SDL_CreateTextureFromSurface(renderer, image);
     check_error_sdl_img(img_texture == nullptr, "Unable to create a texture from the image");
     SDL_FreeSurface(image);
     return img_texture;
 }
