#pragma once
#include <vector>
#include <cstdint>

struct Fbuffer{
	Fbuffer(int _w, int _h) : w(_w), h(_h) { pixels = new uint32_t[w * h]; };
	~Fbuffer() { delete[] pixels; };
	inline void set_pixel(int x, int y, uint32_t color){
		if(x >= 0 && x < w && y >= 0 && y < h){
			pixels[(h - y - 1) * w + x] = color; // flip y axis
			//pixels[y * w + x] = color;
		}
	};
	int w;
	int h;
	uint32_t * pixels;
};
