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
smooth out vec2 cellCoords;

flat out int hasCursor;

flat out uint[8] shadowTypes;

uniform ivec2 glyph_dimensions;
uniform ivec2 sheet_dimensions;
uniform ivec2 glyph_count;
uniform mat4 projection_mat;
uniform float fog_density;
uniform ivec2 cursor_pos;

// Lighting mode
flat out uint lightingMode;


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
	
	
	// Retrieve light data
	lightingMode = get_lighting_mode();
	
	// Output cell position
	// Top left + tex offset
	cellCoords = coords + textureCoords[gl_VertexID];
}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------

