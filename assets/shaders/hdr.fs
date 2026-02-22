#version 400 core

out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D scene;
uniform sampler2D highlight;
uniform float exposure;

void main()
{             
    const float gamma = 2.2;
    vec3 hdrColor = texture(scene, TexCoords).rgb;      
    vec3 highLightColor = texture(highlight, TexCoords).rgb;
    hdrColor += highLightColor; // additive blending
    // tone mapping
    vec3 result = vec3(1.0) - exp(-hdrColor * exposure);
    // also gamma correct while we're at it       
    //result = pow(result, vec3(1.0 / gamma));
    // tone mapping
    FragColor = vec4(result, 1.0);
    FragColor = vec4(highLightColor, 1.0);
}