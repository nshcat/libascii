#pragma once

#include <stdint.h>
#include "types.h"


typedef struct
{
	ivec2_t position;
	float intensity;
	float padding;
	vec4_t color;
	vec3_t attFactors;
	float radius;
	bool_t useRadius;
	float paddingEnd[3];
} light_t;

extern "C"
{
	uint64_t lighting_create_light(light_t* light);

	void lighting_destroy_light(uint64_t handle);

	bool_t lighting_has_space(int count);

	void lighting_set_ambient(uvec3_t* clr);
}
