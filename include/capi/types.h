#pragma once
#include <stdint.h>


typedef struct
{
	uint32_t x;
	uint32_t y;
} uvec2_t;


typedef struct
{
	union
	{
		struct
		{
			uint32_t x;
			uint32_t y;
			uint32_t z;
		};
		struct
		{
			uint32_t r;
			uint32_t g;
			uint32_t b;
		};
	};
} uvec3_t;


typedef struct
{
	int32_t x;
	int32_t y;
} ivec2_t;


typedef struct
{
	float r;
	float g;
	float b;
	float a;
} vec4_t;


typedef struct
{
	union
	{
		struct
		{
			float r;
			float g;
			float b;
		};
		float entries[3];
	};
} vec3_t;


typedef int bool_t;


