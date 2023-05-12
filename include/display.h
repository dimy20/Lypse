#pragma once
#include <stdint.h>

namespace display{
	void init(int window_w, int window_h);
	void quit();
	void present_pixels(const uint32_t * pixels);
};

