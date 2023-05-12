
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <SDL2/SDL.h>
#include <assert.h>
#include "error.h"
#include "display.h"

static SDL_Window * window;
static SDL_Renderer * renderer;
static SDL_Texture * fb_texture;
static int w;
static int h;

void display::init(int window_w, int window_h){
	DIE((SDL_Init(SDL_INIT_VIDEO) < 0), SDL_GetError());

	window = SDL_CreateWindow("3d Renderer", 
				              SDL_WINDOWPOS_CENTERED,
							  SDL_WINDOWPOS_CENTERED,
							  window_w,
							  window_h,
							  0);

	DIE(!window, SDL_GetError());
	DIE(!(renderer = SDL_CreateRenderer(window, -1, 0)), SDL_GetError());

	fb_texture = SDL_CreateTexture( renderer, 
									SDL_PIXELFORMAT_RGBA8888,
									SDL_TEXTUREACCESS_STREAMING,
									window_w,
									window_h);
	DIE(!fb_texture, SDL_GetError());

	w = window_w;
	h = window_h;
};

void display::quit(){
	SDL_DestroyTexture(fb_texture);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
}

void display::present_pixels(const uint32_t * pixels){
	assert(pixels != NULL);
	int pitch = w * sizeof(uint32_t);
	DIE((SDL_UpdateTexture(fb_texture, NULL, pixels, pitch) < 0), SDL_GetError());
	DIE((SDL_RenderCopy(renderer, fb_texture, NULL, NULL) < 0), SDL_GetError());
	SDL_RenderPresent(renderer);
}
