#pragma once
#include <stddef.h>
#include <stdio.h>

#define DIE(v, s) do{\
	if(v){\
		fprintf(stderr, "Error: %s - at %s:%d\n", s, __FILE__, __LINE__);\
		exit(1);\
	}\
}while(0);
