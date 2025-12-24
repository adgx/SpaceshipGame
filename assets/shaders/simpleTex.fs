#version 400 core

out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D albedo_tex;

void main()
{
    FragColor = texture(albedo_tex, TexCoords);
}