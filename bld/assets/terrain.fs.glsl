#version 450 core

// output to the framebuffer;
layout (location = 0) out vec4 fragmentColor;
in vec4 vxcolor;

// const colors
const vec4 white 	   = vec4(1.0);
const vec4 forestGreen = vec4(95.0, 167.0, 119.0, 255.0) / 255.0;
const vec4 burntUmber  = vec4(128.0, 85.0, 51.0, 255.0) / 255.0;

void main()
{
	fragmentColor = vxcolor;
}

