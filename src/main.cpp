#include <iostream>
#include <SDL2/SDL.h>
#include "display.h"
#include "fbuffer.h"
#include "draw.h"
#include "vec.h"

#define SCREEN_W 800
#define SCREEN_H 600

inline uint32_t pack_color(uint8_t r, uint8_t g, uint8_t b){
	return static_cast<uint32_t>(r << 24 | g << 16 | b << 8 | 0xff);
}

static bool running = false;
Fbuffer fbuffer(SCREEN_W, SCREEN_H);

static void R_input(){
	SDL_Event e;
	while(SDL_PollEvent(&e)){
		if(e.type == SDL_QUIT){
			running = false;
		}
	};
};

static void R_update(){
	draw_line(&fbuffer, 0, 0, 100, 100, 0xff0000ff);
}

static void R_render(){
	display::present_pixels(fbuffer.pixels);
}

void R_run(){
	running = true;
	while(running){
		R_input();
		// TODO: cap fps
		R_update();
		R_render();
	};
};

int main(){
	display::init(SCREEN_W, SCREEN_H);
	R_run();
	display::quit();
	return 0;
};
