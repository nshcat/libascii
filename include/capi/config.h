#pragma once

#include "types.h"

typedef enum
{
	VAL_TYPE_FLOAT = 0,
	VAL_TYPE_DOUBLE,
	VAL_TYPE_INT,
	VAL_TYPE_UINT,
	VAL_TYPE_STRING,
	VAL_TYPE_BOOL
} config_value_t;

extern "C"
{
	config_value_t configuration_get_type(const char* path);
	float configuration_get_float(const char* path);
	double configuration_get_double(const char* path);
	int configuration_get_int(const char* path);
	unsigned configuration_get_uint(const char* path);
	const char* configuration_get_string(const char* path);
	bool_t configuration_get_boolean(const char* path);
}

