#version 400 core

out vec4 FragColor;
in vec2 TexCoords;
in vec3 WorldPos;
in vec3 Normal;

// material params
uniform vec4 albedo_color_val;
uniform float metallic_val;
uniform float roughness_val;
uniform float ambient_occlusion_val;

// material textures
uniform sampler2D albedo_tex;
uniform sampler2D metalness_tex;
uniform sampler2D roughness_tex;
uniform sampler2D normal_map_tex;
uniform sampler2D ambient_occlusion_tex;

// lights
struct Light
{
    vec3 pos;
    vec3 color;
};

uniform Light lights[4];

// camera
uniform vec3 camPos;

const float PI = 3.14159265359;

// subroutines
subroutine vec3 sAlbedo();
subroutine uniform sAlbedo albedoMode;
subroutine float sMetalness();
subroutine uniform sMetalness metalnessMode;
subroutine float sRoughness();
subroutine uniform sRoughness roughnessMode;
subroutine vec3 sNormals();
subroutine uniform sNormals normalsMode;
subroutine float sAO();
subroutine uniform sAO AOMode;

// Albedo
subroutine( sAlbedo )
vec3 getAlbedoFromVal()
{
    return albedo_color_val.xyz;
}

subroutine( sAlbedo )
vec3 getAlbedoFromTex()
{
    return pow(texture(albedo_tex, TexCoords).rgb, vec3(2.2));
}

// Metalness
subroutine( sMetalness )
float getMetalnessFromVal()
{
    return metallic_val;
}

subroutine( sMetalness )
float getMetalnessFromTex()
{
    return texture(metalness_tex, TexCoords).r;
}

// Roughness
subroutine( sRoughness )
float getRoughnessFromVal()
{
    return roughness_val;
}

subroutine( sRoughness )
float getRoughnessFromTex()
{
    return texture(roughness_tex, TexCoords).r;
}

// Normals
subroutine( sNormals )
vec3 getNormalsFromVal()
{
    return normalize(Normal);
}

subroutine( sNormals )
vec3 getNormalsFromTex()
{
    vec3 tangentNormal = texture(normal_map_tex, TexCoords).xyz * 2.0 - 1.0;

    vec3 Q1  = dFdx(WorldPos);
    vec3 Q2  = dFdy(WorldPos);
    vec2 st1 = dFdx(TexCoords);
    vec2 st2 = dFdy(TexCoords);

    vec3 N   = normalize(Normal);
    vec3 T  = normalize(Q1*st2.t - Q2*st1.t);
    vec3 B  = -normalize(cross(N, T));
    mat3 TBN = mat3(T, B, N);

    return normalize(TBN * tangentNormal);
}

// Ambient Occlusion
subroutine( sAO )
float getAOFromVal()
{
    return ambient_occlusion_val;
}

subroutine( sAO )
float getAOFromTex()
{
    return texture(ambient_occlusion_tex, TexCoords).r;
}

// Cook-Torrance functions
float distributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float nom = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / denom;
}

float geometrySchlickGGX(float NdotV, float roughness)
{
    float r = roughness + 1.0;
    float k = (r * r) / 8.0;

    float nom = NdotV; 
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}

float geometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx1 = geometrySchlickGGX(NdotL, roughness);
    float ggx2 = geometrySchlickGGX(NdotV, roughness);

    return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

void main()
{
    vec3 albedo = albedoMode();
    float metallic = metalnessMode();
    float roughness = roughnessMode();
    float ao = AOMode();

    vec3 N = normalsMode();
    vec3 V = normalize(camPos - WorldPos);

    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo, metallic);

    vec3 Lo = vec3(0.0);

    for(int i = 0; i < 4; i++)
    {
        // calculate per-light radiance
        vec3 L = normalize(lights[i].pos - WorldPos);
        vec3 H = normalize(V + L);

        //float distance = length(lights[i].pos - WorldPos);
        //float attenuation = 1.0 / (distance * distance);
        vec3 radiance = lights[i].color;
        //cook-torrance BRDF
        float NDF = distributionGGX(N, H, roughness);
        float G = geometrySmith(N, V, L, roughness);
        vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);

        vec3 numerator = NDF * G * F;
        float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
        vec3 specular = numerator / denominator;

        vec3 kS = F;
        vec3 kD = vec3(1.0) - kS;
        kD *= 1.0 - metallic;

        float NdotL = max(dot(N, L), 0.0);

        Lo += (kD * albedo / PI + specular) * radiance * NdotL;
    }

    vec3 ambient = vec3(0.03) * albedo * ao;

    vec3 color = ambient + Lo;

    // HDR tonemap + gamma correction
    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0 / 2.2));

    FragColor = vec4(color, 1.0);
}