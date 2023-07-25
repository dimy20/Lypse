#include "display.h"
#include "SDL_error.h"
#include "error.h"
#include <cassert>

bool Display::init(int width, int height, const char *window_name){
    FMT_LOG_ERROR_RET(SDL_Init(SDL_INIT_VIDEO) < 0, "Error: %s\n", SDL_GetError());

    m_window = SDL_CreateWindow(window_name,
                                SDL_WINDOWPOS_CENTERED,
                                SDL_WINDOWPOS_CENTERED,
                                width,
                                height,
                                0);

    FMT_LOG_ERROR_RET(!m_window, "Error: %s\n", SDL_GetError());

    m_renderer = SDL_CreateRenderer(m_window, -1, SDL_RENDERER_PRESENTVSYNC);
    FMT_LOG_ERROR_RET(!m_renderer, "Error: %s\n", SDL_GetError());


    m_fbuffer_texture = SDL_CreateTexture(m_renderer,
                                                 SDL_PIXELFORMAT_RGB24,
                                                 SDL_TEXTUREACCESS_STREAMING,
                                                 width,
                                                 height);

    FMT_LOG_ERROR_RET(!m_fbuffer_texture, "Error: %s\n", SDL_GetError());

    m_width = width;
    m_height = height;

    return true;
}

void Display::quit(){
    SDL_DestroyTexture(m_fbuffer_texture);
    SDL_DestroyRenderer(m_renderer);
    SDL_DestroyWindow(m_window);
    SDL_Quit();
}

bool Display::update_pixels(const uint8_t *pixels){
    assert(pixels != NULL);

    int pitch = m_width * 3;
    int ret;
    ret = SDL_UpdateTexture(m_fbuffer_texture, NULL, pixels, pitch);
    FMT_LOG_ERROR_RET(ret < 0, "Error: %s\n", SDL_GetError());

    ret = SDL_RenderCopy(m_renderer, m_fbuffer_texture, NULL, NULL);
    FMT_LOG_ERROR_RET(ret < 0, "Error: %s\n", SDL_GetError());

    return true;
}

void Display::present_pixels(){ SDL_RenderPresent(m_renderer); }
