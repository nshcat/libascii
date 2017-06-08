#version 450

// output to the framebuffer
layout (location = 0) out vec4 fragmentColor;

uniform sampler2D sheet_texture;

flat in vec4 frontColor;
flat in vec4 backColor;

smooth in vec2 texCoords;

void main()
{
	vec4 texColor = texture2D(sheet_texture, texCoords);

	fragmentColor = mix(backColor, frontColor * texColor, texColor.a);
}

