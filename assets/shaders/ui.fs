#version 400 core

out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D ui_tex;
uniform vec4 color_val;

void main()
{
    FragColor = texture(ui_tex, TexCoords)*color_val;
}