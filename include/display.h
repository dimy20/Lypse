#pragma once
#include <SDL2/SDL.h>

struct Display{
    Display() {};
    bool init(int w, int h, const char *window_name);
    void quit();
    bool update_pixels(const uint8_t *pixels);
    void present_pixels();

    int m_width;
    int m_height;
    private:
        SDL_Window *m_window;
        SDL_Renderer *m_renderer;
        SDL_Texture *m_fbuffer_texture;
};

bool display_init(Display *display, int w, int h, const char *window_name);
void display_quit(Display *display);
bool display_update_pixels(Display *display, const uint8_t *pixels);
void display_present_pixels(Display *display);
