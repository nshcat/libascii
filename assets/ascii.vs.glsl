//===-- ascii.vs.glsl - Main vertex shader ---------------------*- GLSL -*-===//
//
//                     			 gl_app
//
// This file is distributed under the MIT Open Source License.
// See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the main vertex shader for the ascii graphics engine.
//
// It implements calculation of glyph texture coordinates, shadow intensity
// and lighting and forwards that data to the fragment shader.
//
//===----------------------------------------------------------------------===//
// TODO:
//  - Use interface blocks for smooth and flat output data
//  - Use one global CellData instance to store cell data
//
//===----------------------------------------------------------------------===//

#version 450

//===----------------------------------------------------------------------===//
// Constants and macros
//

// Shadow orientation bit positions
#define SHADOW_N 	0x1 << 8
#define SHADOW_W 	0x1 << 9
#define SHADOW_S 	0x1 << 10
#define SHADOW_E 	0x1 << 11
#define SHADOW_TL 	0x1 << 12
#define SHADOW_TR 	0x1 << 13
#define SHADOW_BL 	0x1 << 14
#define SHADOW_BR 	0x1 << 15

// Shadow bit mask, contains shadow orientation bit field
#define SHADOW_MASK 0xFF00

// Light modes
#define LIGHT_NONE 	0
#define LIGHT_DIM 	1
#define LIGHT_FULL 	2

// Mask and shift for light mode value
#define LIGHT_MASK 	0xF0000
#define LIGHT_SHIFT 16

// Mask for fog percentage value
#define FOG_MASK 0xFF

// Cursor bit position
#define CURSOR 0x1 << 16

// Maximum number of lights allowed in the light data uniform
#define MAX_LIGHTS 25

//===----------------------------------------------------------------------===//





//===----------------------------------------------------------------------===//
// Struct definitions
//

// Struct containing all information a light source has
struct Light
{
	vec2  position;		// ┐	//< Position in the game world
	float intensity;	// │	//< Intensity of the light source
	bool  use_radius;	// ┘	//< Calculate att. factors based on radius
	vec4  color;		// 		//< Color of the light
	vec3  att_factors;	// ┐	//< Factors used in attenuation function
	float radius;		// ┘	//< Radius of illumination
};

// Struct containg global state of the lighting system
struct LightingState
{
	bool use_lighting;	// ┐	//< Global lighting enable/disable
	bool use_dynamic;	// │	//< Dynamic lighting enable/disable
	vec2 tl_position;	// ┘	//< Absolute position of the top left corner
	vec4 ambient;		//		//< Global ambient illumination
};

// Struct containg all data that can be extracted from a cell entry in
// the input buffer
struct CellData
{
	vec4 front_color;		//< Front color of the glyph
	vec4 back_color;		//< Back color of the glyph
	vec2 glyph;				//< Glyph coordinates on glyph sheet
	float fog_percentage;	//< Fog percentage value
	uint shadows[8];		//< Array of shadow orientation flags
	uint light_mode;		//< Light calulation mode
	bool gui_mode;			//< Act like a normal tile for light calculations,
							//  but be fully lit (for GUI elements that overlap
							//  lit scenery)
};

//===----------------------------------------------------------------------===//





//===----------------------------------------------------------------------===//
// Uniform Data
//

// Buffer containg screen data.
//
// Format: (r,g,b,glyph)(r,g,b,data)
//
// With data being composed as follows:
//
// FF F F   0  0  0  0  0 0 0 0 FF
//      ^   ^  ^  ^  ^  ^ ^ ^ ^ ^
//      LM  BR BL TR TL E S W N fog
//	        └──Drop Shadows───┘  
//
uniform usamplerBuffer input_buffer;


// Buffer containg all lights to use in lighting calculations
layout (std140, binding = 0) uniform Lights
{
	LightingState state;
	Light lights[MAX_LIGHTS];
	uint num_lights;
};


// Miscellaneous uniforms
uniform ivec2 glyph_dimensions;	//< Dimensions of a single glyph in pixels
uniform ivec2 sheet_dimensions;	//< Dimensions of glyph sheet in glyphs
uniform ivec2 glyph_count;		//< Screen size in glyphs
uniform mat4 projection_mat;	//< Projection matrix
uniform float fog_density;		//< Density value for fog calculation
uniform ivec2 cursor_pos;		//< Position of cursor, in screen coordinates

//===----------------------------------------------------------------------===//









