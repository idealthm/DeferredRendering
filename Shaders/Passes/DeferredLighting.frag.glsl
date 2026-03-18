#include "../PBR/BRDF.glsl"
#include "../Common/Structures.glsl"

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedoSpec;
uniform sampler2D gRMS;

in vec2 vvTexCoords;

vec3 CalculateLighting_Phong()
{
    vec3 dirlight = CalculateDirectionLighting(lightDir);
}

vec3 CalculateLighting_PBR()
{
    vec3 N = texture(gNormal, vTexCoords).rgb;
    vec4 albedoSpec = texture(gAlbedoSpec, vTexCoords);
    float roughness = texture(gRMS, vTexCoords).r;
    
    vec3 albedo = albedoSpec.xyz;
    float metallic = albedo.w;
    
    // basic F (IOR)
    vec3 F0 = mix(vec3(0.04), albedo, metallic);
    
    vec3 L = normalize(lightPos - FragPos);
    vec3 V = normalize(camPos - FragPos);
    vec3 H = normalize(V + L);

    float D = D_GGX(N, H, roughness);
    float G = G_Smith(N, V, L, roughness);
    vec3 F = F_Schlick(max(dot(H, V), 0.0), F0);

    vec3 ks = F;
    vec3 kd = vec3(1.0) - ks;
    kd *= ( 1.0 - metallic);

    vec3 numerator = D * G * F;
    vec3 denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
    
    vec3 specular = numerator / denominator;

    float NDotL = max(dot(N, L), 0.0);
    vec3 Lo = (Kd * albedo / PI + specular) * NDotL;
    return Lo;
}

void main() {
    // 1. 从 G-Buffer 提取数据
    vec3 fragPos = texture(gPosition, vTexCoords).rgb;
    vec3 N = texture(gNormal, vTexCoords).rgb;
    vec3 albedo = texture(gAlbedoSpec, vTexCoords).rgb;
    float metallic = texture(gAlbedoSpec, vTexCoords).a;
    float roughness = texture(gRMS, vTexCoords).r;

    // 2. 基础向量计算
    vec3 V = normalize(camPos - fragPos);
    vec3 F0 = vec3(0.04); 
    F0 = mix(F0, albedo, metallic);

    // 3. 光照累加 (以一个方向光为例)
    vec3 L = normalize(lightPos - fragPos);
    vec3 H = normalize(V + L);
    
    // 运行 BRDF
    float D = D_GGX(N, H, roughness);
    float G = G_Smith(N, V, L, roughness);
    vec3 F  = F_Schlick(max(dot(H, V), 0.0), F0);
    
    // 能量守恒：镜面反射 F 决定了折射比例 kD
    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= (1.0 - metallic); 

    // Cook-Torrance BRDF 公式
    vec3 numerator    = D * G * F;
    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
    vec3 specular = numerator / denominator;

    float NdotL = max(dot(N, L), 0.0);
    vec3 Lo = (kD * albedo / PI + specular) * lightColor * NdotL;

    FragColor = vec4(Lo, 1.0);
}