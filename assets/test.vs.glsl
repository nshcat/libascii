#version 450

#define SHADOW_N 0x1 << 8
#define SHADOW_W 0x1 << 9
#define SHADOW_S 0x1 << 10
#define SHADOW_E 0x1 << 11

#define SHADOW_TL 0x1 << 12
#define SHADOW_TR 0x1 << 13
#define SHADOW_BL 0x1 << 14
#define SHADOW_BR 0x1 << 15

#define CURSOR 0x1 << 16

#define FOG_MASK 0xFF
#define SHADOW_MASK 0xFF00
#define LIGHT_MASK 0xF0000
#define LIGHT_SHIFT 16

#define LIGHT_NONE 0
#define LIGHT_DIM 1
#define LIGHT_FULL 2

// Front an Background color
flat out vec4 frontColor;
flat out vec4 backColor;

// Fog factor
flat out float fogFactor;

// Texture coordinates
smooth out vec2 texCoords;
smooth out vec2 shadowCoords;
smooth out vec2 cursorCoords;

flat out int hasCursor;
flat out uint[8] shadowTypes;
flat out uint lightingMode;

flat out vec4 lightColor;	// The transparent result color that resembles accumulated influence of light sources.

uniform ivec2 glyph_dimensions;
uniform ivec2 sheet_dimensions;
uniform ivec2 glyph_count;
uniform mat4 projection_mat;
uniform float fog_density;
uniform ivec2 cursor_pos;


// Light data
uniform bool use_lighting;
uniform bool use_dynamic_lighting;
uniform bool use_smooth_lighting;
uniform vec4 ambient_light;
uniform vec2 top_left_pos;
uniform vec2 light_pos;
uniform vec4 light_clr;
uniform float light_intensity; // [0, 1]
uniform float light_intensity2; // [0, 1]
uniform bool debug_mode;
// --



// Format: (r,g,b,glyph)(r,g,b,data)
// With data:
//
// FF FF   0  0  0  0  0 0 0 0 FF
//         ^  ^  ^  ^  ^ ^ ^ ^ ^
//         BR BL TR TL E S W N fog
//		   \__Drop Shadows__/
//
// Uses uvec4.
// Cursor field is unused for now.
uniform usamplerBuffer input_buffer;

const vec2 positions[] = vec2[6](
	vec2(1, 1),	// BR
	vec2(1, 0),	// TR
	vec2(0, 0),	// TL
	
	vec2(1, 1),	// BR
	vec2(0, 0),	// TL
	vec2(0, 1)	// BL
);

uint get_shadow_data()
{
	const uvec4 t_entry = texelFetch(input_buffer, (gl_InstanceID*2)+1);
	
	return t_entry.a & SHADOW_MASK;
}

uint get_lighting_mode()
{
	const uvec4 t_entry = texelFetch(input_buffer, (gl_InstanceID*2)+1);
	return (t_entry.a & LIGHT_MASK) >> LIGHT_SHIFT;
}

uint get_lighting_mode(in uvec2 pos)
{
	const int index = int((pos.y * glyph_count.x) + pos.x);

	const uvec4 t_entry = texelFetch(input_buffer, (index*2)+1);
	return (t_entry.a & LIGHT_MASK) >> LIGHT_SHIFT;
}

uint get_data()
{
	const uvec4 t_entry = texelFetch(input_buffer, (gl_InstanceID*2)+1);
	
	return t_entry.a;
}

void get_front(out vec4 p_front)
{
	const uvec4 t_entry = texelFetch(input_buffer, gl_InstanceID*2);	
	p_front.rgb = vec3(t_entry.rgb) / 255.f;
	
	p_front.r = min(p_front.r, 1.0f);
	p_front.g = min(p_front.g, 1.0f);
	p_front.b = min(p_front.b, 1.0f);
	
	p_front.a = 1.0f;
}

void get_back(out vec4 p_back)
{
	const uvec4 t_entry = texelFetch(input_buffer, (gl_InstanceID*2)+1);	
	p_back.rgb = vec3(t_entry.rgb) / 255.f;
	
	p_back.r = min(p_back.r, 1.0f);
	p_back.g = min(p_back.g, 1.0f);
	p_back.b = min(p_back.b, 1.0f);
	
	p_back.a = 1.0f;
}

void get_glyph(out vec2 p_glyph)
{
	const uvec4 t_entry = texelFetch(input_buffer, gl_InstanceID*2);
	const uint t_glyphid = t_entry.a;
	p_glyph = vec2(
		t_glyphid % sheet_dimensions.x,
		t_glyphid / sheet_dimensions.x
	);
}

float get_fog_percentage()
{
	const uvec4 t_entry = texelFetch(input_buffer, (gl_InstanceID*2)+1);
	
	const uint t_fog = t_entry.a & FOG_MASK;
	
	return float(t_fog)/255.f;
}

void calc_fog()
{
	// Calculate fog factor f
	//fogFactor = (1.f - get_fog_percentage());
	
	fogFactor = exp(- fog_density * get_fog_percentage());
	
	
	// Clamp it to [0, 1]
	fogFactor = clamp(fogFactor, 0.0f, 1.0f);
}


// ^u
// |    v
// +--->
const vec2 textureCoords[] = vec2[6](
	vec2(1, 1),	// BR
	vec2(1, 0),	// TR
	vec2(0, 0),	// TL
	
	vec2(1, 1),	// BR
	vec2(0, 0),	// TL
	vec2(0, 1)	// BL
);

// Texture coordinates for left most shadow quad
const vec2 shadowCoordsArray[] = vec2[6](
	vec2(1.f/8.f, 1),	// BR
	vec2(1.f/8.f, 0),	// TR
	vec2(0, 0),			// TL
	
	vec2(1.f/8.f, 1),	// BR
	vec2(0, 0),			// TL
	vec2(0, 1)			// BL
);


const vec4 colors[] = vec4[6](
	vec4(1.0f, 0.f, 0.f, 1.f),
	vec4(0.0f, 1.f, 0.f, 1.f),
	vec4(0.0f, 0.f, 1.f, 1.f),
	
	vec4(1.0f, 1.f, 0.f, 1.f),
	vec4(1.0f, 0.f, 1.f, 1.f),
	vec4(0.0f, 1.f, 1.f, 1.f)
);

void calc_tex_coords(in vec2 p_glyph, out vec2 p_texcoords)
{
	// Dim of glyph texture in uv space
	const vec2 t_dimTex = vec2( 1.f/sheet_dimensions.x, 1.f/sheet_dimensions.y );

	// Glyph coords on tex sheet, in discrete glyph index form

	// Calculate bottom left corner
	vec2 t_base_bl = t_dimTex * p_glyph;
	
	// Transform if needed
	p_texcoords = t_base_bl + (t_dimTex * textureCoords[gl_VertexID]);
}

bool can_see_light(in ivec2 start_pos, in ivec2 end_pos)
{
	const int len = max(abs(end_pos.x - start_pos.x), abs(end_pos.y - start_pos.y));
	const float dx = (end_pos.x - start_pos.x)/len;
	const float dy = (end_pos.y - start_pos.y)/len;
	
	for(int i = 0; i <= len; ++i)
	{
		const ivec2 point = ivec2(int(floor(start_pos.x + i*dx)), int(floor(start_pos.y + i*dy)));
		
		const uint mode = get_lighting_mode(uvec2(point));
		
		if(mode == LIGHT_NONE || mode == LIGHT_DIM)
			return false;
	}
	
	return true;
}

bool check_point(in uvec2 point)
{
	const uint mode = get_lighting_mode(uvec2(point));
	
	return !(mode == LIGHT_NONE || mode == LIGHT_DIM);
}

bool can_see_light2(in ivec2 start_pos, in ivec2 end_pos)
{
	ivec2 start = start_pos;

	int delta_x = end_pos.x - start_pos.x;
	int ix = 0;
	{
		if(delta_x > 0)
			ix = 1;
		else if(delta_x < 0)
			ix = -1;
	}
	delta_x = abs(delta_x) << 1;
	
	
	int delta_y = end_pos.y - start_pos.y;
	int iy = 0;
	{
		if(delta_y > 0)
			iy = 1;
		else if(delta_y < 0)
			iy = -1;
	}
	delta_y = abs(delta_y) << 1;
	
	if(!check_point(uvec2(start)))
		return false;
		
	if(delta_x >= delta_y)
	{
		int error = (delta_y - (delta_x >> 1));
		
		while(start.x != end_pos.x)
		{
			if( (error >= 0) && ( (error != 0) || (ix > 0)) )
			{
				error -= delta_x;
				start.y += iy;
			}
			
			error += delta_y;
			start.x += ix;
			
			if(!check_point(uvec2(start)))
				return false;
		}
	}
	else
	{
		int error = (delta_x - (delta_y >> 1));
		
		while(start.y != end_pos.y)
		{
			if( (error >= 0) && ( (error != 0) || (iy > 0)) )
			{
				error -= delta_y;
				start.x += ix;
			}
			
			error += delta_x;
			start.y += iy;
			
			if(!check_point(uvec2(start)))
				return false;
		}
	}
	
	return true;
}

// Calculate light color of single light source
/*void light(in ivec2 p_cellPos, in ivec2 p_lightPos, in float p_lightIntensity, in vec4 p_lightColor, inout vec4 p_shadeColor)
{
	// TODO: Cutoff. If distance is greater than a value N, stop right away.

	// --- Check if light is visible
	if(!can_see_light2(p_cellPos, p_lightPos))
		return;
		
	// --- Calculate distance to light
	const float dist = length(p_cellPos - p_lightPos);
	
	// --- Calculate intensity TODO custom a, b and c as vec3 in light data
	const float intensity = min(1.f / (1. + 0.4*dist + 0.01*pow(dist, 2.0f)), 1.f) * p_lightIntensity;
	
	// --- Create light color vector
	const vec4 tmpColor = vec4(p_lightColor.rgb, intensity);
	
	// --- Use alpha blending to create new intermediate color
	p_shadeColor.a = tmpColor.a + p_shadeColor.a*(1.f - tmpColor.a);
	p_shadeColor.rgb = (tmpColor.rgb*tmpColor.a + p_shadeColor.rgb*p_shadeColor.a*(1.f-tmpColor.a)) / p_shadeColor.a;
	
	if(p_shadeColor.a == 0.f)
		p_shadeColor.rgb = vec3(0.f);
}*/


/*void light(in ivec2 p_cellPos, in ivec2 p_lightPos, in float p_lightIntensity, in vec4 p_lightColor, inout vec4 p_shadeColor)
{
	// TODO: Cutoff. If distance is greater than a value N, stop right away.

	// --- Check if light is visible
	if(!can_see_light2(p_cellPos, p_lightPos))
		return;
		
	// --- Calculate distance to light
	const float dist = length(p_cellPos - p_lightPos);
	
	// --- Calculate intensity TODO custom a, b and c as vec3 in light data
	const float intensity = min(1.f / (1. + 0.4*dist + 0.01*pow(dist, 2.0f)), 1.f);
	
	p_shadeColor += intensity*p_lightColor;//*p_lightIntensity;
	p_shadeColor.a = 1.f;
}*/

void light(in ivec2 p_cellPos, in ivec2 p_lightPos, in float p_lightIntensity, in vec4 p_lightColor, in float p_radius, in float p_constAttFact, inout vec4 p_shadeColor)
{
	// TODO: Cutoff. If distance is greater than a value N, stop right away.

	// --- Check if light is visible
	if(!can_see_light2(p_cellPos, p_lightPos))
		return;
		
	// --- Calculate distance to light
	const float dist = length(p_cellPos - p_lightPos);
	
	// --- Calculate intensity
	// THIS USES THE LIGHT RADIUS
	// => TODO: Expose const_att_factor and radius
	const float const_att_factor = p_constAttFact;
	float intensity = min(1.f / (const_att_factor + ((2.f/p_radius)*dist) + (1.f/pow(p_radius, 2.f))*pow(dist, 2.0f)), 1.f);
	
	// Calc max distance
	/*const float max_dist = p_radius * ( sqrt(p_lightIntensity/0.01f) - 1.f );
	
	if(dist >= max_dist)
		intensity = 0.f;*/
	
	p_shadeColor += intensity * p_lightColor * p_lightIntensity;
	p_shadeColor.a = 1.f;
}

void lighting()
{
	lightingMode = get_lighting_mode();

	if(use_dynamic_lighting && (lightingMode != LIGHT_NONE))
	{
		// --- Cell position (we only need to care about top left, since we 
		// are doing flat shading anyways)
		vec2 cellCoords = vec2(	gl_InstanceID % glyph_count.x,
								gl_InstanceID / glyph_count.x );
								
		cellCoords += top_left_pos;
		
		// TODO needed?
		//cellCoords = floor(cellCoords);
		
		// Initialize destination color
		lightColor = vec4(0.f);
		
		// Handle lights
		light(ivec2(cellCoords), ivec2(light_pos), light_intensity, light_clr, 1.5f, 1.f, lightColor);
		
		light(ivec2(cellCoords), ivec2(3, 3), light_intensity2, light_clr, 10.f, 1.f, lightColor);
	}
}


void main()
{
	// Since we use an inverted ortho projection (y axis flipped) we can work with coordinates the 
	// same way we would do on a 2d window, with (0,0) being the top left corner, and y increasing downwards.
	
	// Compute coordinate of top left vertex. All other vertex coordinates can be derived from this.
	const vec2 coords = vec2(
		gl_InstanceID % glyph_count.x,
		gl_InstanceID / glyph_count.x
	);
	
	vec2 t_bl = coords * vec2(glyph_dimensions);

	// Transform TL coordinates into matching coordinates by doing a look-up based on the local index
	vec2 t_vertex = (positions[gl_VertexID] * vec2(glyph_dimensions)) + t_bl;


	// Transform with projection matrix
	vec4 t_vertex4 = vec4(0.f, 0.f, 0.f, 1.f);
	t_vertex4.xy = t_vertex;
	gl_Position = projection_mat * t_vertex4;

	// Set texture coordinates
	vec2 t_glyph = vec2(0.f);
	get_glyph(t_glyph);
	calc_tex_coords(t_glyph, texCoords);

	shadowCoords = shadowCoordsArray[gl_VertexID];

	// Assign a color
	get_front(frontColor);
	get_back(backColor);
	
	// Calc fog
	calc_fog();
	
	uint shadowData = get_shadow_data();
	
	shadowTypes = uint[8]
	(
		shadowData & SHADOW_W,
		shadowData & SHADOW_S,
		shadowData & SHADOW_N,
		shadowData & SHADOW_E,
		
		shadowData & SHADOW_BR,
		shadowData & SHADOW_BL,
		shadowData & SHADOW_TL,
		shadowData & SHADOW_TR
	);
	
	// Cursor
	vec2 t_cursorPos = vec2(cursor_pos);
	if(t_cursorPos == coords)
	{
		hasCursor = 1;
		
		calc_tex_coords(vec2(15.f, 15.f), cursorCoords); 
	}
	else
	{
		hasCursor = 0;
	}
	
	
	// Do lighting work
	lighting();
}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------

