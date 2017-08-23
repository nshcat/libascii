//===-- ascii.fs.glsl - Main fragment shader -------------------*- GLSL -*-===//
//
//                     			 gl_app
//
// This file is distributed under the MIT Open Source License.
// See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the main fragment shader for the ascii graphics engine.
//
//===----------------------------------------------------------------------===//
// TODO:
//  - Cursor support
//
//===----------------------------------------------------------------------===//

#version 450

//===----------------------------------------------------------------------===//
// Constants and macros
//

// Drop shadow orientation bit positions. A cell may have any combination of
// them. They will be rendered on-top of each other.
#define SHADOW_N 	0x1U << 8U
#define SHADOW_W 	0x1U << 9U
#define SHADOW_S 	0x1U << 10U
#define SHADOW_E 	0x1U << 11U
#define SHADOW_TL 	0x1U << 12U
#define SHADOW_TR 	0x1U << 13U
#define SHADOW_BL 	0x1U << 14U
#define SHADOW_BR 	0x1U << 15U

// Amount of different shadow orientations
#define NUM_SHADOWS 8U

// Shadow bit mask, contains shadow orientation bit field
#define SHADOW_MASK 0xFF00U

// Light modes. These are used to determine how a cell reacts to lighting.
#define LIGHT_NONE 	0U	//< Block light. Stays completely dark.
#define LIGHT_DIM 	1U	//< Block light. Receive small amount of light.
#define LIGHT_FULL 	2U	//< Don't block light. Receive full amount of light.

// Mask and shift for light mode value
#define LIGHT_MASK 	0xF0000U
#define LIGHT_SHIFT 16U

// Glyph sets
#define GLYPH_SET_TEXT 0U
#define GLYPH_SET_GRAPHICS 1U

// Gui mode bit position
#define GUI_MODE 0x1U << 20U

// Mask for fog percentage value
#define FOG_MASK 0xFFU

// Cursor bit position
#define CURSOR 0x1U << 16U

// Maximum number of lights allowed in the light data uniform
#define MAX_LIGHTS 25U

//===----------------------------------------------------------------------===//





//===----------------------------------------------------------------------===//
// Struct definitions
//

// Struct containing all information a light source has
struct Light
{
	ivec2 position;		// ┐	//< Position in the game world TODO if something breaks revert this to "vec2"
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
	float dim_light;	// ┐	//< How much light dim surfaces will receive [0, 1]
	float padding1;		// │
	float padding2;		// │
	float padding3;		// ┘
};

//===----------------------------------------------------------------------===//





//===----------------------------------------------------------------------===//
// Uniform Data
//

// Buffer containg all lights to use in lighting calculations
layout (std140, binding = 0) uniform LightData
{
	LightingState state;
	Light lights[MAX_LIGHTS];
	uint num_lights;
} light_data;


// Miscellaneous uniforms
uniform sampler2D text_texture;
uniform sampler2D graphics_texture;
uniform sampler2D shadow_texture;
uniform vec4 fog_color;
uniform vec4 cursor_default; //< Default cursor front color

//===----------------------------------------------------------------------===//




//===----------------------------------------------------------------------===//
// Output data
//

// Main fragment shader output
layout (location = 0) out vec4 fragmentColor;

//===----------------------------------------------------------------------===//




//===----------------------------------------------------------------------===//
// Input data
//

// Flat input data (no interpolation)
in vs_flat_out
{
	flat vec4 front_color;		//< Foreground color of glyph
	flat vec4 back_color;		//< Background color of glyph
	flat float fog_factor;		//< Fog interpolation value [0, 1]
	flat uint[8] shadows;		//< Array of shadow orientation flags
	flat int has_cursor;		//< Flag indicating presence of cursor
	flat uint light_mode;		//< How this cell should react to light
	flat uint gui_mode;			//< Flag indicating GUI mode (see CellData)
	flat uint glyph_set;		//< Glyph set to use to render cell
	flat vec4 lighting_result;	//< Color representing result of lighting
								//  calculations. Is blended in with colored
								//  glyph texture in fragment shader.
} flat_in;


// Smooth output data
in vs_smooth_out
{
	smooth vec2 tex_coords;		//< Glyph texture coordinates for cell
	smooth vec2 cursor_coords;	//< Cursor texture coordinates for cell
	smooth vec2 shadow_coords;	//< Shadow texture coordinates. This actually
								//  references the leftmost shadow texture,
								//  but will be offset in fragment shader
								//  to blend all needed orientations together.
} smooth_in;

//===----------------------------------------------------------------------===//



//===----------------------------------------------------------------------===//
// Subroutines
//

// Calculate base pixel color using the sheet texture
vec4 calc_pixel()
{
	vec4 t_texColor = vec4(0.f);
	
	switch(flat_in.glyph_set)
	{
		case GLYPH_SET_TEXT:
		{
			t_texColor = texture2D(text_texture, smooth_in.tex_coords);
			break;
		}		
		case GLYPH_SET_GRAPHICS:
		{
			t_texColor = texture2D(graphics_texture, smooth_in.tex_coords);
			break;
		}
	}

	return mix(	flat_in.back_color,
				flat_in.front_color * t_texColor,
				t_texColor.a );
}

void light_pixel(inout vec4 p_pixel)
{
	// Only do lighting if enabled and not gui mode
	if(	light_data.state.use_lighting &&
		!bool(flat_in.gui_mode) &&
		(flat_in.light_mode != LIGHT_NONE) )
	{
		// Apply ambient lighting
		p_pixel = mix(vec4(0.f), p_pixel, light_data.state.ambient);
		
		// Only apply dynamic lighting if requested
		if(light_data.state.use_dynamic)
		{
			// LIGHT_DIM will not accept dynamic lighting
			if(flat_in.light_mode != LIGHT_DIM)
			{
				p_pixel += flat_in.lighting_result;
			}
			else
			{
				p_pixel += light_data.state.dim_light * flat_in.lighting_result;
			}
		}
	}
}

void add_shadows(inout vec4 p_pixel)
{
	for(uint i = 0; i < NUM_SHADOWS; ++i)
	{
		if(flat_in.shadows[i] > 0)
		{
			// We only need to modify the U-component of the shadow texture
			// coordinates, since all shadow textures are stored in one
			// sequential line
			const float t_u_offset = i * 1.f/8.f;
			
			// Calculate new shadow texture coordinates
			const vec2 t_shadowCoords = vec2(
				smooth_in.shadow_coords.x + t_u_offset,
				smooth_in.shadow_coords.y
			);
			
			// Fetch texel
			const vec4 t_color = texture2D(shadow_texture, t_shadowCoords);
			
			// Blend it using alpha blending
			p_pixel = mix(p_pixel, t_color, t_color.a);
			p_pixel.a = 1.f;
		}
	}
}

void add_fog(inout vec4 p_pixel)
{
	p_pixel = mix(fog_color, p_pixel, flat_in.fog_factor);
	p_pixel.a = 1.f;
}
//===----------------------------------------------------------------------===//




//===----------------------------------------------------------------------===//
// Shader entry point
//
void main()
{
	// Calculate pixel color based on glyph texture
	vec4 t_color = calc_pixel();
	
	// Light the pixel
	light_pixel(t_color);
	
	// Mix in drop shadows
	add_shadows(t_color);
	
	// Mix in fog
	add_fog(t_color);
	
	// Output pixel
	fragmentColor = t_color;
}
//===----------------------------------------------------------------------===//





