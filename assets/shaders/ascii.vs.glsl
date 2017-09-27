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
// Note that this shader receives no actual geometry - all vertex data is
// calculated "just-in-time" by this shader. The calling program uses instanced
// drawing to cause the GPU to execute this shader six times per glyph position
// (two triangles to form one quad).
//
//===----------------------------------------------------------------------===//
// TODO:
//  - Add real glow. Add flag that controls if bg or fg are glowing,
//    then use classic glow algorithm (only render glowing parts to FBO,
//    blur them, blend them additively over main scene)
//    (glBlendEquation(GL_ADD); glBlendFunc(GL_ONE, GL_ONE);)
//
//  - Support MULTIPLE glyph sets: glyph_set::graphical, glyph_set::text,
//    glyph_set::draw (<= special set to draw pictures, contains triangles etc)
//    (see ultima ratio regum)
//    ==> Could use upper bits of glyph entry to set that
//    IMPLEMENTATION: One class to manage ALL glyph textures. Takes up to two
//                    paths, or ONE NAME (from_name, from_path(1str or 2str)
//                    from_name appends "graphic" and "text" to it.
//                    Will check if size is the same.
//
//  - CanSeeLight: If the start is a LIGHT_DIM, then it should still receive
//    light, if it is directly connected to a tile that is permeable
//    (Only the first layer of wall should be illuminated)
//  - ==> /!\ Better: Count the number of walls the light has to pass through
//                    that are LIGHT_DIM, and calculate falloff!
//					  If we reach LIGHT_NORMAL _AFTER_ LIGHT_DIM (aka
// 					  pass_through > 0) we return false!
//
//  - Allow walls to receive a small amount of dynamic lights
//  - Use interface blocks for smooth and flat output data
//  - Use one global CellData instance to store cell data
//  - To implement fog: Change fog value in input buffer to "depth", a value
//    that can be both used for fog calulation aswell as lighting (should be
//    integral)
//  - Lighting should not light the drop shadows!
//  - Lighting needs to use 3D norm for distance, using the stored "depth"
//    value
//  - The gui_mode bit should result in the cell being lit 100%, but being used
//    like a "normal" cell for lighting calculations
//  - Add two ambient light values: inside and outside. This would require a bit
//    to indicate whether a certain cell is inside or outside.
//  - Maybe add materials that let light through but color it (stained glass).
//    This could be implemented using new light modes.
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

// Shadow bit mask, contains shadow orientation bit field
#define SHADOW_MASK 0xFF00U

// Light modes. These are used to determine how a cell reacts to lighting.
#define LIGHT_NONE 	0U	//< Block light. Stays completely dark.
#define LIGHT_DIM 	1U	//< Block light. Receive small amount of light.
#define LIGHT_FULL 	2U	//< Don't block light. Receive full amount of light.

// Mask and shift for light mode value
#define LIGHT_MASK 	0xF0000U
#define LIGHT_SHIFT 16U

// Gui mode bit position
#define GUI_MODE 0x1U << 20U

// Mask for integral depth value
#define DEPTH_MASK 0xFFU

// Cursor bit position
#define CURSOR 0x1U << 16U

// Maximum number of lights allowed in the light data uniform
#define MAX_LIGHTS 25U

// Glyph sets
#define GLYPH_SET_TEXT 0U
#define GLYPH_SET_GRAPHICS 1U

// Glyph set mask and shift value
#define GLYPH_SET_MASK 0xF00U
#define GLYPH_SET_SHIFT 0x8U

// Glyph value mask
#define GLYPH_MASK 0xFFU

// Glyph texture offsets from the top left for the 6 vertices of a cell.
const vec2 texture_offset[] = vec2[6](
	vec2(1, 1),	// BR
	vec2(1, 0),	// TR
	vec2(0, 0),	// TL
	
	vec2(1, 1),	// BR
	vec2(0, 0),	// TL
	vec2(0, 1)	// BL
);

// Drop shadow coordinates of the first shadow element for the 6 vertices of
// the cell
const vec2 shadow_coords[] = vec2[6](
	vec2(1.f/8.f, 1),	// BR
	vec2(1.f/8.f, 0),	// TR
	vec2(0, 0),			// TL
	
	vec2(1.f/8.f, 1),	// BR
	vec2(0, 0),			// TL
	vec2(0, 1)			// BL
);

// Offset from top left vertex for each of the six vertices of the cell quad.
// These are used to calculate the vertex data for every shader call.
const vec2 vertex_offset[] = vec2[6](
	vec2(1, 1),	// BR
	vec2(1, 0),	// TR
	vec2(0, 0),	// TL
	
	vec2(1, 1),	// BR
	vec2(0, 0),	// TL
	vec2(0, 1)	// BL
);

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

// Struct containg all data that can be extracted from a cell entry in
// the input buffer
struct CellData
{
	vec2 screen_coords;		//< Screen coordinates of cell in glyphs
	vec4 front_color;		//< Front color of the glyph
	vec4 back_color;		//< Back color of the glyph
	vec2 glyph;				//< Glyph coordinates on glyph sheet
	uint depth;				//< Depth of the tile. Used to calculate fog.
	uint shadows[8];		//< Array of shadow orientation flags
	uint light_mode;		//< Light calulation mode
	uint glyph_set;			//< Glyph set to use to render this cell
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
// Every cell is described by two uvec4 instances in the buffer.
// Format: (fr,fg,fb,glyph)(br,bg,bb,data)
//
// With data being composed as follows:
//
// FF 000 0  F  0  0  0  0  0 0 0 0 FF
//        ^  ^  ^  ^  ^  ^  ^ ^ ^ ^ ^
//        GM LM BR BL TR TL E S W N depth
//	            └──Drop Shadows───┘  
//
// And glyph being composed as follows:
//
// FF FF F        F     FF
//                ^      ^
//        glyph_set  glyph
//
uniform usamplerBuffer input_buffer;


// Buffer containg all lights to use in lighting calculations
layout (std140, binding = 0) uniform LightData
{
	LightingState state;
	Light lights[MAX_LIGHTS];
	uint num_lights;
} light_data;


// Miscellaneous uniforms
uniform ivec2 glyph_dimensions;	//< Dimensions of a single glyph in pixels
uniform ivec2 sheet_dimensions;	//< Dimensions of glyph sheet in glyphs
uniform ivec2 glyph_count;		//< Screen size in glyphs
uniform mat4 projection_mat;	//< Projection matrix
uniform float fog_density;		//< Density value for fog calculation
uniform ivec2 cursor_pos;		//< Position of cursor, in screen coordinates

//===----------------------------------------------------------------------===//




//===----------------------------------------------------------------------===//
// Output data
//

// Flat output data (no interpolation)
out vs_flat_out
{
	flat vec4 front_color;		//< Foreground color of glyph
	flat vec4 back_color;		//< Background color of glyph
	flat float fog_factor;		//< Fog interpolation value [0, 1]
	flat uint[8] shadows;		//< Array of shadow orientation flags
	flat int has_cursor;		//< Flag indicating presence of cursor
	flat uint light_mode;		//< How this cell should react to light
	flat uint gui_mode;			//< Flag indicating GUI mode (see CellData)
	flat uint glyph_set;		//< Glyph set to use to render this cell
	flat vec4 lighting_result;	//< Color representing result of lighting
								//  calculations. Is blended in with colored
								//  glyph texture in fragment shader.
} flat_out;


// Smooth output data
out vs_smooth_out
{
	smooth vec2 tex_coords;		//< Glyph texture coordinates for cell
	smooth vec2 cursor_coords;	//< Cursor texture coordinates for cell
	smooth vec2 shadow_coords;	//< Shadow texture coordinates. This actually
								//  references the leftmost shadow texture,
								//  but will be offset in fragment shader
								//  to blend all needed orientations together.
} smooth_out;

//===----------------------------------------------------------------------===//




//===----------------------------------------------------------------------===//
// Global variables
//

// Information about the currently handled cell
CellData this_cell;

//===----------------------------------------------------------------------===//




//===----------------------------------------------------------------------===//
// Subroutines
//

// Retrieve light mode
// This does not directly assign to this_cell because it is also used
// by lighting calculations to detect objects that block the light ray
uint read_lm(in uint p_in)
{
	return (p_in & LIGHT_MASK) >> LIGHT_SHIFT;
}

// Retrieve drop shadow orientations
void read_shadows(in uint p_in)
{
	this_cell.shadows = uint[8]
	(
		p_in & SHADOW_W,
		p_in & SHADOW_S,
		p_in & SHADOW_N,
		p_in & SHADOW_E,	
		p_in & SHADOW_BR,
		p_in & SHADOW_BL,
		p_in & SHADOW_TL,
		p_in & SHADOW_TR
	);
}

// Calculate vertex for this shader call
void emit_vertex()
{
	// Calculate absolute (in world space) top left coordinates of this cell
	vec2 t_tl = this_cell.screen_coords * vec2(glyph_dimensions);
	
	// Add offset for vertices that are not the top left one
	t_tl += vertex_offset[gl_VertexID] * vec2(glyph_dimensions);
	
	// Create homogenous 4D vector and transform using projection matrix,
	// which implements an orthographic view frustum where the y-axis is flipped.
	// This allows us to use "screen-like" coordinates (with the origin being 
	// the top left corner of the screen, and the y-axis growing downwards)
	// in world space.
	gl_Position = projection_mat * vec4(t_tl, 0.f, 1.f);	
}

// Calculate texture coordinates for this cell
void calc_tex_coords()
{
	// Dimension of a single glyph texture in texture space (UV)
	const vec2 t_dimTex = vec2(1.f/sheet_dimensions.x, 1.f/sheet_dimensions.y);
	
	// Calculate texture coordinates of top left vertex
	vec2 t_tl = t_dimTex * this_cell.glyph;
	
	// If this vertex is in fact not the top left one, we need to add an offset
	// to calculate the texture coordinates.
	// This is simply done by adding the offset (which is either 0 or 1 in both
	// x and y direction) multiplied by the size of one glyph in texture space.
	// We will receive one of the four corners of the glyph texture.
	t_tl += texture_offset[gl_VertexID] * t_dimTex;
	
	// Write value to output interface block
	smooth_out.tex_coords = t_tl;
}

void calc_shadow_coords()
{
	// Lookup value and write to output interface block
	smooth_out.shadow_coords = shadow_coords[gl_VertexID];
}

void calc_fog()
{
	// Standard exponential distance fog equation
	const float t_fogFactor = exp(-fog_density * this_cell.depth);
	
	flat_out.fog_factor = clamp(t_fogFactor, 0.f, 1.f);
}


// Checks whether the screen cell lets light through
bool check_point(in ivec2 p_point)
{
	// Since p_point is actually in absolute game world coordinates,
	// we have to substract the absolute position of top left
	const ivec2 t_relPoint = p_point - ivec2(light_data.state.tl_position);
	
	// Special case: The start cell may very well be a LIGHT_DIM cell,
	// because the first layer of wall should receive a little bit of light.
	if(t_relPoint == this_cell.screen_coords && this_cell.light_mode == LIGHT_DIM)
		return true;
	
	// Check if it is in screen bounds
	const ivec2 t_clamped = ivec2(
		clamp(t_relPoint.x, 0, glyph_count.x-1),
		clamp(t_relPoint.y, 0, glyph_count.y-1)
	);
	
	if(t_relPoint != t_clamped)
		return false; // Assume that the light is not visible anymore.
		// This COULD be problematic (lights suddenly disappearing even though
		// they should still be in range), but we can't do any better here
		// since the only data we have is the screen. No data of the
		// surroundings is available. The game could focus the screen on the
		// player to make this limitation less obvious.
		
	// Fetch entry containg the lighting mode (low word)
	const int t_index = int((t_relPoint.y * glyph_count.x) + t_relPoint.x);
	const uint t_word = texelFetch(input_buffer, (t_index*2)+1).a;
	
	const  uint t_lm = read_lm(t_word);
	
	return !(t_lm == LIGHT_NONE || t_lm == LIGHT_DIM);
}

// Reads data of this cell and saves it to the global cell info variable
void read_cell()
{
	// Determine screen coordinates of this cell
	this_cell.screen_coords = vec2(
		gl_InstanceID % glyph_count.x,
		gl_InstanceID / glyph_count.x
	);
	
	// Retrieve the two uvec4 containing all cell data
	const uvec4 t_high = texelFetch(input_buffer, gl_InstanceID*2);
	const uvec4 t_low = texelFetch(input_buffer, (gl_InstanceID*2)+1);
	
	// Retrieve front and back color
	this_cell.front_color = vec4(vec3(t_high.rgb) / 255.f, 1.f);
	this_cell.back_color = vec4(vec3(t_low.rgb) / 255.f, 1.f);
	
	// Retrieve glyph coordinates
	this_cell.glyph = vec2(
		(t_high.a & GLYPH_MASK) % sheet_dimensions.x,
		(t_high.a & GLYPH_MASK) / sheet_dimensions.x
	);
	
	// Retrieve depth
	// There is no need to shift here, since the fog component starts with
	// the LSB
	this_cell.depth = uint(t_low.a & DEPTH_MASK);
	
	// Check if gui mode bit is set
	this_cell.gui_mode = ( (t_low.a & GUI_MODE) != 0.f ? true : false );
	
	// Retrieve light mode
	this_cell.light_mode = read_lm(t_low.a);
	
	// Read drop shadow orientations
	read_shadows(t_low.a);
	
	// Read glyph set
	this_cell.glyph_set = ((t_high.a & GLYPH_SET_MASK) >> GLYPH_SET_SHIFT);
}

// Calculates whether a light source can be seen from given cell
bool visible(in ivec2 p_cellPos, in ivec2 p_lightPos)
{
	// The following code is an implementation of Bresenham's algorithm.
	ivec2 start = p_cellPos;

	int delta_x = p_lightPos.x - p_cellPos.x;
	int ix = 0;
	{
		if(delta_x > 0)
			ix = 1;
		else if(delta_x < 0)
			ix = -1;
	}
	delta_x = abs(delta_x) << 1;
	
	
	int delta_y = p_lightPos.y - p_cellPos.y;
	int iy = 0;
	{
		if(delta_y > 0)
			iy = 1;
		else if(delta_y < 0)
			iy = -1;
	}
	delta_y = abs(delta_y) << 1;
	
	if(!check_point(start))
		return false;
		
	if(delta_x >= delta_y)
	{
		int error = (delta_y - (delta_x >> 1));
		
		while(start.x != p_lightPos.x)
		{
			if( (error >= 0) && ( (error != 0) || (ix > 0)) )
			{
				error -= delta_x;
				start.y += iy;
			}
			
			error += delta_y;
			start.x += ix;
			
			if(!check_point(start))
				return false;
		}
	}
	else
	{
		int error = (delta_x - (delta_y >> 1));
		
		while(start.y != p_lightPos.y)
		{
			if( (error >= 0) && ( (error != 0) || (iy > 0)) )
			{
				error -= delta_y;
				start.x += ix;
			}
			
			error += delta_x;
			start.y += iy;
			
			if(!check_point(start))
				return false;
		}
	}
	
	return true;
}

void calc_lighting()
{
	if(light_data.state.use_lighting && light_data.state.use_dynamic 
		&& (this_cell.light_mode != LIGHT_NONE))
	{
		// Calculate absolute position of cell in game world
		const ivec2 t_cellPos = ivec2(this_cell.screen_coords + 
			light_data.state.tl_position);
			
		// Initialize destination color
		vec4 t_lightColor = vec4(0.f);
		
		// Process all lights
		for(uint t_index = 0; t_index < light_data.num_lights; ++t_index)
		{
			// Fetch current light
			Light t_light = light_data.lights[t_index];
			
			// Lights inside of walls should not be visible
			if((t_light.position == t_cellPos) && (this_cell.light_mode == LIGHT_DIM))
				continue;
				
			// Check if light is visible
			// Note that we ALLOW the light to be outside of the screen.
			// Its important though that the routine does not access out of
			// screen bounds
			if(!visible(t_cellPos, t_light.position))
				continue;
				
			// Calculate distance to light
			const float t_dist = length(t_cellPos - t_light.position);
			
			// Calculate intensity using light attenuation function
			float t_intensity = 0.f;
			
			if(t_light.use_radius)
			{
				// Use radius to calculate falloff
				t_intensity = 1.f / (1.f + ((2.f/t_light.radius)*t_dist)
						+ (1.f/pow(t_light.radius, 2.f))*pow(t_dist, 2.0f));
			}
			else
			{
				// Use given attenuation factors to calculate falloff
				t_intensity = 1.f / (t_light.att_factors.x
						+ (t_light.att_factors.y*t_dist)
						+ (t_light.att_factors.z*pow(t_dist, 2.0f)));
			}
			
			// Clamp intensity, since at short distances the attenuation
			// function gets infinitely big
			t_intensity = min(t_intensity, 1.f);
			
			// Add to accumulated light color
			t_lightColor += (t_intensity * t_light.intensity) * t_light.color;
			t_lightColor.a = 1.f;
		}
		
		// Output calculated light color
		flat_out.lighting_result = t_lightColor;
	}
}

// Writes remaining data to the fragment shader input
void write_data()
{
	flat_out.front_color = this_cell.front_color;
	flat_out.back_color = this_cell.back_color;
	flat_out.shadows = this_cell.shadows;
	flat_out.has_cursor = 0; // TODO Implement cursor
	flat_out.light_mode = this_cell.light_mode;
	flat_out.gui_mode = this_cell.gui_mode ? 1 : 0;
	flat_out.glyph_set = this_cell.glyph_set;
}
//===----------------------------------------------------------------------===//




//===----------------------------------------------------------------------===//
// Shader entry point
//
void main()
{
	// Read cell data
	read_cell();
	
	// Create vertex depending on current vertex ID
	emit_vertex();
	
	// Calculate glyph texture coordinates for this cell
	calc_tex_coords();
	
	// Calculate shadow coordinates for this cell
	calc_shadow_coords();
	
	// Calculate fog
	calc_fog();
	
	// Do lighting
	calc_lighting();
	
	// Write remaining data to output interface blocks
	write_data();
}
//===----------------------------------------------------------------------===//





