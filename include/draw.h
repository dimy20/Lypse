#pragma once

#include <cstdint>
#include "fbuffer.h"

void draw_line(Fbuffer * fb, int x1, int y1, int x2, int y2, uint32_t color);
void draw_rect(Fbuffer * fb, int x1, int y1, int w, int h, uint32_t color);
void draw_triangle(Fbuffer * fb, int x0, int y0, int x1, int y1, int x2, int y2, uint32_t color);
void draw_texture_mapped_triangle(Fbuffer * fb, int x0, int y0, double u0, double v0,
						                        int x1, int y1, double u1, double v1,
						                        int x2, int y2, double u2, double v2, uint32_t * texture_pixels);
