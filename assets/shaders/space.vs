#version 400

layout (location = 0) in vec2 aPos;

out vec2 FragCoord;

uniform vec2 res;

void main()
{
    FragCoord = 0.5 + aPos * 0.5 * res; 
    gl_Position = vec4(aPos, 0., 1.);
}
