#version 450

// output to the framebuffer
layout (location = 0) out vec4 fragmentColor;

uniform sampler2D sheet_texture;
uniform sampler2D shadow_texture;
uniform vec4 fog_color;

flat in vec4 frontColor;
flat in vec4 backColor;
flat in float fogFactor;

smooth in vec2 texCoords;
smooth in vec2 shadowCoords;

flat in uint[8] shadowTypes;

void draw_shadow(inout vec4 color, in uint index)
{
	// Calculate u increment
	const float u_offset = index * 1.f/8.f;
	
	// Calculate new shadow texture coordinates
	vec2 tex_coords = shadowCoords;
	tex_coords.x += u_offset;
	
	// Retrieve shadow texture value
	const vec4 tex_color = texture2D(shadow_texture, tex_coords);
	
	// Mix it with current fragment color
	color = mix(color, tex_color, tex_color.a);
	color.a = 1.f;
}

void main()
{
	vec4 texColor = texture2D(sheet_texture, texCoords);

	// Mix back and front color together: (1-a)*bg + a*fg
	// We multiply the front color by texColor here to allow textures to
	// affect all color channels as a multiplicative brightness factor.
	// This allows textures to create all kinds of effects.
	// It doesn't matter that if texColor.a == 0.f we destroy the frontColor,
	// since in that case it is already canceled out by being multiplied by 0 :)
	// TODO doesn't this multiply frontColor twice by alpha?
	// Yes, it will result in alpha != 1.f. Maybe thats bad.
	// FIXME maybe just multiply with texcolor.rgb, with a being 1.f, to only
	// scale the colors.
	fragmentColor = mix(backColor, frontColor * texColor, texColor.a);
	
	
	// Mix in shadows, if requested
	for(uint i = 0; i < 8; ++i)
	{
		if(shadowTypes[i] > 0)
		{
			draw_shadow(fragmentColor, i);
		}
	}
	
	
	// Mix in fog
	fragmentColor =	mix(fog_color, fragmentColor, fogFactor);
	fragmentColor.a = 1.f;
	
	
	// Mix in Cursor (not affected by fog!)
	
	// for cursor:
	// vec4 cursorTex = ... ;
	
	// Cursor color OVER fragmentcolor
	// TODO maybe this doesnt work? We need to FIRST colourize the cursor texture,
	// and THEN alpha blend it like usual (see shader cookbook: applying multiple textures)
	// cursorPixel = mix(vec4(0.f, 0.f, 0.f, 0.f), cursorColor, cursorTex.a)
	// fragmentColor = mix(fragmentColor, cursorPixel, cursorPixel.a);
	
}

