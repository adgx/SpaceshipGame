#version 400 core

layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexCoords;

uniform mat4 projection;
uniform vec2 uiPos;
uniform vec2 size;

out vec2 TexCoords;

void main()
{
    TexCoords = aTexCoords;
    vec2 p = uiPos + aPos * size;
    gl_Position = projection * vec4(aPos, 0.0, 1.0);
}