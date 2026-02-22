#version 400 core

layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BrightColor;

in vec2 TexCoords;

uniform vec4 color_val; 
uniform sampler2D albedo_tex;

//subroutines
subroutine vec4 sColor();
subroutine uniform sColor colorMode;

//texture
subroutine( sColor )
vec4 getColorTex()
{
    return texture(albedo_tex, TexCoords);
}

subroutine( sColor )
vec4 getColorVal()
{
    return color_val;
}

//extract the bright information
float luma(vec3 color) 
{
    return 0.2126 * color.r + 0.7152 * color.g + 0.0722 * color.b;
}

void main()
{
    vec4 col = colorMode();
    float brightness = luma(col.rgb);

    if(brightness > 1)
        BrightColor = col;
    else
        BrightColor = vec4(0.0, 0.0, 0.0, 1.0);    
    FragColor = col;
}