#include "display.h"
#include "SDL_error.h"
#include "error.h"
#include <cassert>

bool display_init(Display *display, int w, int h, const char *window_name){
    assert(display != NULL);

    FMT_LOG_ERROR_RET(SDL_Init(SDL_INIT_VIDEO) < 0, "Error: %s\n", SDL_GetError());

    display->window = SDL_CreateWindow(window_name, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, w, h, 0);
    FMT_LOG_ERROR_RET(!display->window, "Error: %s\n", SDL_GetError());

    display->renderer = SDL_CreateRenderer(display->window, -1, SDL_RENDERER_PRESENTVSYNC);
    FMT_LOG_ERROR_RET(!display->renderer, "Error: %s\n", SDL_GetError());


    display->fbuffer_texture = SDL_CreateTexture(display->renderer,
            SDL_PIXELFORMAT_RGB24,
            SDL_TEXTUREACCESS_STREAMING,
            w,
            h);

    FMT_LOG_ERROR_RET(!display->fbuffer_texture, "Error: %s\n", SDL_GetError());

    display->width = w;
    display->height = h;

    return true;
}

void display_quit(Display *display){
    if(display != NULL){
        SDL_DestroyTexture(display->fbuffer_texture);
        SDL_DestroyRenderer(display->renderer);
        SDL_DestroyWindow(display->window);
        SDL_Quit();
    }
}

bool display_update_pixels(Display *display, const uint8_t *pixels){
    assert(display != NULL);
    assert(pixels != NULL);

    int pitch = display->width * 3;
    int ret;
    ret = SDL_UpdateTexture(display->fbuffer_texture, NULL, pixels, pitch);
    FMT_LOG_ERROR_RET(ret < 0, "Error: %s\n", SDL_GetError());

    ret = SDL_RenderCopy(display->renderer, display->fbuffer_texture, NULL, NULL);
    FMT_LOG_ERROR_RET(ret < 0, "Error: %s\n", SDL_GetError());

    return true;
}

void display_present_pixels(Display *display){
    assert(display != NULL);
    SDL_RenderPresent(display->renderer);
}
