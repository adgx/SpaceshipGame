#version 400 core
in vec2 TexCoords;
out vec4 color;
uniform sampler2D text_tex;
uniform vec3 color_val;

void main()
{
    vec4 sampled = vec4(1.0, 1.0, 1.0, texture(text_tex, TexCoords).r);
    color = vec4(color_val, 1.0) * sampled;
}