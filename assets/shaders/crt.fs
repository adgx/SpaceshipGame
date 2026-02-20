float scan = 0.9; // simulate darkness between scanlines

void mainImage(out vec4 fragColor,in vec2 fragCoord)
{
// squared distance from center
vec2 uv = fragCoord/iResolution.xy;

    // determine if we are drawing in a scanline
    float apply = abs(sin(fragCoord.y + iTime*2.)*0.5*scan);
    // sample the texture
    fragColor = vec4(mix(texture(iChannel0,uv).rgb,vec3(0.0),apply),1.0);
    
}