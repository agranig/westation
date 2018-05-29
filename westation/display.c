#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

#include "display.h"

#define FONT_PATH "/Library/Fonts/Arial.ttf"
// #define FONT_PATH "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf"

int g_sdl_initialized = 0;
int g_img_initialized = 0;
int g_ttf_initialized = 0;

int g_display_w = 0;
int g_display_h = 0;

SDL_Window *g_window = NULL;
SDL_Renderer *g_renderer = NULL;

int display_init(const char *name, int w, int h) {
    int img_flags = IMG_INIT_PNG;
    g_display_w = w;
    g_display_h = h;

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "Failed to init SDL: %s\n", SDL_GetError());
        goto err;
    }
    g_sdl_initialized = 1;

    if ((IMG_Init(img_flags) & img_flags) != img_flags) {
        fprintf(stderr, "Failed to init SDL IMG: %s\n", IMG_GetError());
        goto err;
    }
    g_img_initialized = 1;
    
    if (TTF_Init() != 0) {
        fprintf(stderr, "Failed to init SDL TTF: %s\n", TTF_GetError());
        goto err;
    }
    g_ttf_initialized = 1;

    SDL_ShowCursor(SDL_DISABLE);

    g_window = 
        SDL_CreateWindow(name, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
            w, h, SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL);
    if (!g_window) {
        fprintf(stderr, "Failed to create window: %s\n", SDL_GetError());
        goto err;
    }

    g_renderer = SDL_CreateRenderer(g_window, -1,
            SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!g_renderer) {
        fprintf(stderr, "Failed to create renderer: %s\n", SDL_GetError());
        goto err;
    }

    return 0;

err:
    display_destroy();
    return -1;
}

void display_destroy() {
    if (g_renderer) {
        SDL_DestroyRenderer(g_renderer);
        g_renderer = NULL;
    }

    if (g_window) {
        SDL_DestroyWindow(g_window);
        g_window = NULL;
    }

    if (g_sdl_initialized) {
        SDL_Quit();
        g_sdl_initialized = 0;
    }

    if (g_img_initialized) {
        IMG_Quit();
        g_img_initialized = 0;
    }

    if (g_ttf_initialized) {
        TTF_Quit();
        g_ttf_initialized = 0;
    }
}

static SDL_Texture* display_load_font_texture(const char* text, TTF_Font *font, SDL_Color *color,
        SDL_Renderer *renderer) {
    SDL_Surface *surface;
    SDL_Texture *texture;

    if (!text) {
        fprintf(stderr, "Invalid NULL text in display_load_font_texture\n");
        return NULL;
    }
    if (!font) {
        fprintf(stderr, "Invalid NULL font in display_load_font_texture\n");
        return NULL;
    }
    if (!color) {
        fprintf(stderr, "Invalid NULL color in display_load_font_texture\n");
        return NULL;
    }
    if (!renderer) {
        fprintf(stderr, "Invalid NULL renderer in display_load_font_texture\n");
        return NULL;
    }

    surface = TTF_RenderUTF8_Blended(font, text, *color);
    if (!surface) {
        fprintf(stderr, "Failed to render font surface: %s\n", TTF_GetError());
        return NULL;
    }

    texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (!texture) {
        fprintf(stderr, "Failed to create font texture: %s\n", TTF_GetError());
        return NULL;
    }

    SDL_FreeSurface(surface);
    return texture;
}

static SDL_Texture* display_load_image_texture(const char* fname, SDL_Renderer *renderer) {
    SDL_Surface *surface;
    SDL_Texture *texture;

    if (!fname) {
        fprintf(stderr, "Invalid NULL fnamein display_load_image_texture\n");
        return NULL;
    }
    if (!renderer) {
        fprintf(stderr, "Invalid NULL renderer in display_load_image_texture\n");
        return NULL;
    }
   
    surface = IMG_Load(fname);
    if (!surface) {
        fprintf(stderr, "Failed to load image surface from '%s': %s\n",
                fname, IMG_GetError());
        return NULL;
    }

    texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (!texture) {
        fprintf(stderr, "Failed to load image texture: %s\n",
                IMG_GetError());
        return NULL;
    }

    SDL_FreeSurface(surface);
    return texture;
}

static int display_position_texture_center(SDL_Texture *texture, SDL_Rect *rect) {
    if (!texture) {
        fprintf(stderr, "Invalid NULL texture param for display_center_texture\n");
        return -1;
    }
    if (!rect) {
        fprintf(stderr, "Invalid NULL rect param for display_center_texture\n");
        return -1;
    }

    if (SDL_QueryTexture(texture, NULL, NULL, &rect->w, &rect->h) != 0) {
        fprintf(stderr, "Failed to query texture: %s\n",
                SDL_GetError());
        return -1;
    }
    rect->x = (g_display_w - rect->w) / 2;
    rect->y = (g_display_h - rect->h) / 2;

    return 0;
}

static int display_position_texture(SDL_Texture *texture, int x, int y, SDL_Rect *rect) {

    if (display_position_texture_center(texture, rect) != 0) {
        return -1;
    }
    rect->x = x;
    rect->y = y;

    return 0;
}

// TODO: make generic!
int display_show(const char* icon_path, const char* text, int temp, int hum) {
    SDL_Rect icon_rect;
    SDL_Rect font_rect;

    SDL_Rect temp_rect;
    SDL_Rect hum_rect;
    char buffer[128];

    SDL_Color font_color = {0, 0, 0};
    TTF_Font* font = NULL;
    SDL_Texture *font_texture = NULL;
    SDL_Texture *icon_texture = NULL;

    SDL_Texture *temp_texture = NULL;
    SDL_Texture *hum_texture = NULL;

    SDL_SetRenderDrawColor(g_renderer, 255, 255, 255, 255);

    icon_texture = display_load_image_texture(icon_path, g_renderer);
    if (!icon_texture) {
        goto err;
    }
    if (display_position_texture_center(icon_texture, &icon_rect) != 0) {
        fprintf(stderr, "Failed to center icon texture\n");
    }

    // TODO: define fonts in display_init
    font = TTF_OpenFont(FONT_PATH, 24);
    if (!font) {
        fprintf(stderr, "Failed to load font: %s\n", TTF_GetError());
        goto err;
    }

    font_texture = display_load_font_texture(text, font, &font_color, g_renderer);
    if (!font_texture) {
        goto err;
    }
    if (display_position_texture_center(font_texture, &font_rect) != 0) {
        fprintf(stderr, "Failed to center font texture\n");
    }
    font_rect.y += icon_rect.h / 2 + 20;

    snprintf(buffer, sizeof(buffer), "%d°C", temp);
    temp_texture = display_load_font_texture(buffer, font, &font_color, g_renderer);
    if (!temp_texture) {
        goto err;
    }
    if (display_position_texture(temp_texture, 50, 50, &temp_rect) != 0) {
        fprintf(stderr, "Failed to position texture\n");
    }

    snprintf(buffer, sizeof(buffer), "%d%%", hum);
    hum_texture = display_load_font_texture(buffer, font, &font_color, g_renderer);
    if (!hum_texture) {
        goto err;
    }
    if (display_position_texture(hum_texture, 0, 50, &hum_rect) != 0) {
        fprintf(stderr, "Failed to position texture\n");
    }
    hum_rect.x = g_display_w - hum_rect.w - 50;


    SDL_RenderClear(g_renderer);
    SDL_RenderCopy(g_renderer, icon_texture, NULL, &icon_rect);
    SDL_RenderCopy(g_renderer, font_texture, NULL, &font_rect);
    SDL_RenderCopy(g_renderer, temp_texture, NULL, &temp_rect);
    SDL_RenderCopy(g_renderer, hum_texture, NULL, &hum_rect);
    SDL_RenderPresent(g_renderer);

    SDL_DestroyTexture(temp_texture);
    SDL_DestroyTexture(hum_texture);
    SDL_DestroyTexture(font_texture);
    SDL_DestroyTexture(icon_texture);
    TTF_CloseFont(font);

    return 0;

err:
    if (temp_texture)
        SDL_DestroyTexture(temp_texture);
    if (hum_texture)
        SDL_DestroyTexture(hum_texture);
    if (font_texture)
        SDL_DestroyTexture(font_texture);
    if (icon_texture)
        SDL_DestroyTexture(icon_texture);
    if (font)
        TTF_CloseFont(font);

    return -1;
}


