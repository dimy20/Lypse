#pragma once
#include <SDL2/SDL.h>

struct Display{
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *fbuffer_texture;
    int width;
    int height;
};

bool display_init(Display *display, int w, int h, const char *window_name);
void display_quit(Display *display);
bool display_update_pixels(Display *display, const uint8_t *pixels);
void display_present_pixels(Display *display);
