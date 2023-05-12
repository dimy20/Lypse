#include "draw.h"

#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <stdio.h>
void draw_line(Fbuffer * fb, int x1, int y1, int x2, int y2, uint32_t color){

	int dx = abs(x1 - x2);
	int dy = abs(y1 - y2);

	bool transposed = false;

	if(dy > dx){ // transpose the line
		std::swap(x1, y1);
		std::swap(x2, y2);
		std::swap(dx, dy);
		transposed  = true;
	}

	// make sure left to write
	if(x1 > x2){
		std::swap(x1, x2);
		std::swap(y1, y2);
	}

	int offset_step = 2 * dy; // avoid use of float
	int offset = 0;
	int y_step = y1 < y2 ? 1 : -1;
	int y = y1;

	if(transposed){
		for(int x = x1; x <= x2; x++){
			fb->set_pixel(y, x, color);
			offset += offset_step;
			if(offset > dx){
				y += y_step;
				offset -= dx * 2;
			}
		}
	}else{
		for(int x = x1; x <= x2; x++){
			fb->set_pixel(x, y, color);
			offset += offset_step;
			if(offset > dx){
				y += y_step;
				offset -= dx * 2;
			}
		}
	}
};

void draw_rect(Fbuffer * fb, int x1, int y1, int w, int h, uint32_t color){
   if(w < 0 || h < 0) return;

	if(x1 >= fb->w || y1 >= fb->h || x1 < 0 || y1 < 0){
		return;
	}

	int start_x = x1;
	int end_x = std::min(x1 + w, fb->w - 1);

	int min_y = y1;
	int max_y = std::min(y1 + h, fb->h - 1);

	for(int y = min_y; y <= max_y; y++){
		draw_line(fb, start_x, y, end_x, y, color);
	}
};

/* compute the determinant of a 2x2 matrix formed by 2 colum vectors
 * As we know the det represents the signed area of the parallelogram formed
 * by this two vectors we can use this value to check wether c is to the left or
 * right of the vector b-a, if edge_function > 0, the the 3 points are in counter clockwise other
 * meaning c is to the left of b-a.*/
static inline double edge_function(int ax, int ay, int bx, int by, int cx, int cy){
	return (bx - ax) * (cy - ay) - (by - ay) * (cx - ax);
}

void draw_triangle(Fbuffer& fb, int x0, int y0, int x1, int y1, int x2, int y2, uint32_t color){
	/*compute a min bounding box, we're gonna use this box as our search space for finding
	  points P inside the triangle v0v1v2 */
	int min_x = fb.w - 1;
	int max_x = 0;

	int min_y = fb.h - 1;
	int max_y = 0;

	int x_values[3] = {x0, x1, x2};
	int y_values[3] = {y0, y1, y2};

	for(int i = 0; i < 3; i++){
		min_x = std::max(std::min(x_values[i], min_x), 0);
		max_x = std::min(std::max(x_values[i], max_x), fb.w - 1);

		min_y = std::max(std::min(min_y, y_values[i]), 0);
		max_y = std::min(std::max(max_y, y_values[i]), fb.h - 1);
	}

	// this is actually 2 times the area;
	//double area = edge_function(v0, v1, v2);
	for(int y = min_y; y <= max_y; y++){
		for(int x = min_x; x <= max_x; x++){
			//A point P is inside the triangle v0v1v2 if the point P is to the lef of all 3 edges
			//v0-v1, v1-v2, v2-v0.
			double w0 = edge_function(x0, y0, x1, y1, x, y);
			double w1 = edge_function(x1, y1, x2, y2, x, y);
			double w2 = edge_function(x2, y2, x0, y0, x, y);

			// barycentric coordiantes
			//auto b0 = w0 / area;
			//auto b1 = w1 / area;
			//auto b2 = w2 / area;

			if(w0 <= 0 && w1 <= 0 && w2 <= 0){
				fb.set_pixel(x, y, color);
			}
		}
	}
}
