#version 400 core

out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D diffuse_tex;

void main()
{
    FragColor = texture(diffuse_tex, TexCoords);
}