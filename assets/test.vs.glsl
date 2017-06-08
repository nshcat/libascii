#version 450


// Front an Background color
flat out vec4 frontColor;
flat out vec4 backColor;

// Texture coordinates
smooth out vec2 texCoords;

uniform ivec2 glyph_dimensions;
uniform ivec2 sheet_dimensions;
uniform ivec2 glyph_count;
uniform mat4 projection_mat;

// Format: (r,g,b,glyph)(r,g,b,cursor)
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

// ^u
// |    v
// +--->
const vec2 textureCoords[] = vec2[6](
	/*vec2(1, 0),	// BR
	vec2(1, 1),	// TR
	vec2(0, 1),	// TL
	
	vec2(1, 0),	// BR
	vec2(0, 1),	// TL
	vec2(0, 0)	// BL*/
	vec2(1, 1),	// BR
	vec2(1, 0),	// TR
	vec2(0, 0),	// TL
	
	vec2(1, 1),	// BR
	vec2(0, 0),	// TL
	vec2(0, 1)	// BL
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
	// Compute coordinate of top left vertex. All other vertex coordinates can be derived from this.
	vec2 coords = vec2(
		gl_InstanceID % glyph_count.x,
		gl_InstanceID / glyph_count.x
	);
	
	vec2 t_bl = coords * vec2(glyph_dimensions);

	// Transform BL coordinates into matching coordinates by doing a look-up based on the local index
	vec2 t_vertex = (positions[gl_VertexID] * vec2(glyph_dimensions)) + t_bl;


	// Transform with projection matrix
	vec4 t_vertex4 = vec4(0.f, 0.f, 0.f, 1.f);
	t_vertex4.xy = t_vertex;
	gl_Position = projection_mat * t_vertex4;

	// Set texture coordinates
	vec2 t_glyph = vec2(0.f);
	get_glyph(t_glyph);
	calc_tex_coords(t_glyph,texCoords);

	// Assign a color
	//frontColor = colors[(gl_InstanceID / 16 ) %6];
	//backColor = vec4(0.0f, 0.0f, 0.0f, 1.f);
	get_front(frontColor);
	get_back(backColor);
}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------

