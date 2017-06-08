#version 150 core


// the xform matrix
uniform mat4 mvp;

// num instances is 1<<(2*bits)
uniform int bits = 8;


out vec4 vxcolor;

// constant vertex positions [-1,1]x[-1,1]
// for a triangle fan with start vertex in the middle
const vec2 positions[] = vec2[](
	vec2(0.5, 0.5),
	vec2(0.0, 0.0),
	vec2(1.0, 0.0),
	vec2(1.0, 1.0),
	vec2(0.0, 1.0),
	vec2(0.0, 0.0)
);

// constant vertex positions [-1,1]x[-1,1]
// for a quad build from 2 triangles
const vec2 positions2[] = vec2[6] (
	vec2( 0.0,  -1.0),
	vec2( 1.0,   0.0),
	vec2( 0.0,   1.0),

	vec2( 0.0,  -1.0),
	vec2( 0.0,   1.0),
	vec2(-1.0,   0.0)
);


//-------------------------------------------------------------------------------------------------
// main entry point for the vertex shader
//-------------------------------------------------------------------------------------------------
void main() {
	// out
	gl_Position = vec4(0.f, 0.f, 0.0f, 1.0f);
	
	vxcolor = vec4(1.f);
}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------

