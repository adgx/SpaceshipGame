#version 400 core

out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D albedo_tex;

void main()
{
    vec4 texColor = texture(albedo_tex, TexCoords);
    
    if(texColor.a < 0.1)
        discard;
        
    FragColor = texColor;
}