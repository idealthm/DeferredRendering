#include <PBR/Lights.glsl>
#include <PBR/BRDF.glsl>
#include <Common/Structures.glsl>

layout(location = 0) out vec4 FragColor;

in vec2 vTexCoords;

#define NR_POINT_LIGHTS 4

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedo;
uniform sampler2D gMaterial;
uniform sampler2D gShadowMap;
uniform sampler2D uBRDF_LUT;
uniform samplerCube uIrradianceMap;
uniform samplerCube uIBL_PreFilterMap;

uniform int uDebugMode;

vec3 NormalDecode(vec2 e)
{
    vec2 uv = e * 2.0 - 1.0;
    float z = 1.0 - abs(uv.x) - abs(uv.y);
    
    // 关键修复：正确重建负Z值
    if (z < 0.0) {
        vec2 uv_prev = uv;
        uv.x = (1.0 - abs(uv_prev.y)) * sign(uv_prev.x);
        uv.y = (1.0 - abs(uv_prev.x)) * sign(uv_prev.y);
        z = 1.0 - abs(uv.x) - abs(uv.y); // 重新计算正Z
        z = -z; // 恢复原始负Z值
    }
    return normalize(vec3(uv, z));
}

float calculateShadowAttenuation(vec4 fragPosLightSpace, vec3 normal, vec3 lightDir)
{
    // 背光面直接返回无阴影
    float theta = dot(normal, lightDir);
    if (theta <= 0.0) return 0.0;

    // 透视除法并映射到 [0,1]
    vec3 ProjCoord = fragPosLightSpace.xyz / fragPosLightSpace.w;
    ProjCoord = ProjCoord * 0.5 + 0.5;

    // 检查是否在光源视锥体内
    if (ProjCoord.z < 0.0 || ProjCoord.z > 1.0 ||
        ProjCoord.x < 0.0 || ProjCoord.x > 1.0 ||
        ProjCoord.y < 0.0 || ProjCoord.y > 1.0) {
        return 0.0;
    }

    float currentDepth = ProjCoord.z;
    float bias = max(0.01 * (1.0 - theta), 0.001);

    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(gShadowMap, 0);
    for(int x = -3; x <= 3; ++x) {
        for(int y = -3; y <= 3; ++y) {
            vec2 sampleCoord = clamp(ProjCoord.xy + vec2(x, y) * texelSize, 0.0, 1.0);
            float closest = texture(gShadowMap, sampleCoord).r;
            shadow += (currentDepth - bias > closest) ? 1.0 : 0.0;
        }
    }
    shadow /= 49.0;

    return shadow;
}

vec3 CalculateLighting_PBR(vec3 L, vec3 N, vec3 V, vec3 albedo, float roughness, float metallic, vec3 lightColor)
{
    // basic F (IOR)
    vec3 F0 = mix(vec3(0.04), albedo, metallic);
    
    vec3 H = normalize(V + L);

    float D = D_GGX(N, H, roughness);
    float G = G_Smith(N, V, L, roughness);
    vec3 F = F_Schlick_Roughness(max(dot(H, V), 0.0), F0, roughness);

    vec3 ks = F;
    vec3 kd = vec3(1.0) - ks;
    kd *= ( 1.0 - metallic);

    vec3 numerator = D * G * F;
    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
    
    vec3 specular = numerator / denominator;

    float NDotL = max(dot(N, L), 0.0);
    vec3 Lo = (kd * albedo / PI + specular) * NDotL * lightColor;
    return Lo;
}

vec3 CalculateLighting_PBR_IBL(vec3 N, vec3 V, vec3 albedo, vec3 irradiance, float roughness, float metallic, float AO)
{
    vec3 R = reflect(-V, N);
    // 假设 MAX_REFLECTION_LOD 是你预过滤贴图的最大 Mip 层级
    vec3 prefilteredColor = textureLod(uIBL_PreFilterMap, R, roughness * 5).rgb;
    vec2 brdf = texture(uBRDF_LUT, vec2(max(dot(N, V), 0.0), roughness)).rg;

    vec3 F0 = mix(vec3(0.04), albedo, metallic);
    vec3 F = F_Schlick_Roughness(max(dot(N, V), 0.0), F0, roughness);
    vec3 kS = F;
    vec3 kD = (1.0 - kS) * (1.0 - metallic);

    vec3 iblDiffuse = irradiance * albedo;
    vec3 iblSpecular = prefilteredColor * (F * brdf.x + brdf.y);
    vec3 ambient = (kD * iblDiffuse + iblSpecular) * AO;
    return ambient;
}


vec3 CalculateLighting()
{
    vec3 FragPos = texture(gPosition, vTexCoords).xyz;
    vec3 N = NormalDecode(texture(gNormal, vTexCoords).rg);
    vec3 albedo = texture(gAlbedo, vTexCoords).rgb;
    vec4 material = texture(gMaterial, vTexCoords);
    vec3 irradiance = texture(uIrradianceMap, N).rgb;

    float roughness = material.r;
    float metallic = material.g;
    float AO = material.b;
    
    vec3 V = normalize(uCamPos - FragPos);

    vec3 color = vec3(0.0);

    for (int i = 0; i < NumLights; i++)
    {
        if (int(lights[i].position.w) == 0)
        {
            vec3 L = -normalize(lights[i].direction.xyz);
            float shadowAttenuation = 1 - calculateShadowAttenuation(uLightVP * vec4(FragPos, 1.0), N, L);
            color += shadowAttenuation * CalculateLighting_PBR(L, N, V, albedo, roughness, metallic, lights[i].color.xyz * lights[i].color.w);
        }
    }

    vec3 ambient = CalculateLighting_PBR_IBL(N, V, albedo, irradiance, roughness, metallic, AO);

    return color + ambient;
}

vec4 CammeraToFragPos()
{
    vec3 FragPos = texture(gPosition, vTexCoords).rgb;
    return vec4(FragPos / 100, 1.0);
}

void main()
{
    switch (RenderMode) {
        case 1: // diffuseColor
            FragColor = vec4(CalculateLighting(), 1.0);
            break;
        case 2: // 位置
            FragColor = vec4(texture(gPosition, vTexCoords).rgb, 1.0);
            break;
        case 3: // 法线
            vec3 normal = NormalDecode(texture(gNormal, vTexCoords).rg);
            FragColor = vec4(normal * 0.5 + 0.5, 1.0);
            break;
        case 4: // 反照率
            FragColor = vec4(texture(gAlbedo, vTexCoords).rgb, 1.0);
            break;
        case 5: // Depth
            FragColor = vec4(vec3(texture(gShadowMap, vTexCoords).r), 1.0);
            break;
        default:
            FragColor = vec4(CalculateLighting(), 1.0);
    }
};