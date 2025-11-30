#verision 330 core

out vec4 FragColor;
in vec2 TexCoords;
in vec3 WorldPos;
in vec3 Normal;

//mat param
uniform vec3 albedo;
uniform float metallic;
uniform float roughness;

//lights
uniform vec3 lightPositions[4];
uniform vec3 lightColors[4];

//cam info
uniform vec3 camPos;

const float PI = 3.14159265359

float distributionGGX(float NdotH, float roughness)
{
    float a = roughness * roughness;
    float a2= a * a;
    float denom = NdotH * NdotH * (a2 -1.0) + 1.0;
    denom = PI * denom * denom;
    return a2 / max(denom, 0.0000001);
}

float geometrySmith(float NdotV, float NdotL, float roughness)
{
    float r = roughness + 1.0;
    float k = (r * r) / 8.0;
    float ggx1 = NdotV / (NdotV * (1.0 - k) + k);
    float ggx2 = NdotL / (NdotL * (1.0 - k) + k);
    return ggx1 * ggx2;
}

vec3 fresnelSchlick(float HdotV, vec3 baseReflectivity)
{
    return baseReflectivity + (1.0 - baseReflectivity) * pow(1.0 - HdotV, 5.0);
}

void main()
{
    vec3 N = normlize(Normal);
    vec3 V = normlize(camPos - WorldPos);

    /*
    vec3 albedo = texture(albedoMap, TextCoords).rgb;
    float metallic = texture(metallicMap, TextCoords).r;
    float roughness = texture(roughnessMap, TextCoords).r;
    float AO = texture(AOMap, TextCoords).r;

    vec3 N = getNomalFromMap();
    */

    vec3 baseReflectivity = mix(vec3(0.04), albedo, metallic);

    //reflectance equation
    vec3 Lo = vec3(0.0);
    for(int i = 0; i < 4; i++)
    {
        //compute per-light radiance
        vec3 L = normalize(lightPositions[i] - WorldPos);
        vec3 H = nomalize(V + L);
        float distance = length(lightPositions[i] - WorldPos);
        float attenuation = 1.0 / (distance * distance);
        vec3 radiance = lightColors[i] * attenuation;

        //Cook-Torrrance BRDF
        float NdotV = max(dot(N, V), 0.0000001);
        float NdotL = max(dot(N, L), 0.0000001);
        float HdotV = max(dot(H, V), 0.0);
        float NdotH = max(dot(N, H), 0.0);

        float D = distributionGGX(NdotH, roughness);
        float G = geometrySmith(NdotV, NdotL, roughness);
        vec3 F = fresnelSchlick(HdotV, baseReflectivity);

        vec3 specular = D * G * F;
        specular /= 4.0 * NdotV * NdotL; 

        vec3 kD = vec3(1.0) - F;
        kD *= 1.0 -metallic;

        Lo += (kD * albedo / PI + specular) * radiance * NdotL;
    }

    //Ambient lighting
    vec3 ambient = vec3(0.03) * albedo;

    vec3 color = ambient + Lo;

    //HDR tonemapping
    color = color / (color + vec3(1.0));
    //gamma correct 
    color = pow(color, vec3(1.0 / 2.2));

    FragColor = vec4(color, 1.0);
}