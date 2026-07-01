

#ifdef LIGHTING_PASS

float D_GGX(vec3 N, vec3 H, float roughness) {
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;
    float nom = a2;
    float denom = NdotH2 * (a2 - 1.0) + 1.0;
    return nom / (PI * denom * denom);
}

float GeometrySchlickGGX(float NdotV, float roughness) {
    float r = roughness + 1.0;
    float k = (r * r) / 8.0;
    return NdotV / (NdotV * (1.0 - k) + k);
}

float G_Smith(vec3 N, vec3 V, vec3 L, float roughness) {
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    return GeometrySchlickGGX(NdotV, roughness) * GeometrySchlickGGX(NdotL, roughness);
}

vec3 F_Schlick(float cosTheta, vec3 F0) {
    return F0 + (vec3(1.0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

vec3 CalculateLighting_PBR(vec3 L, vec3 N, vec3 V, vec3 albedo, float roughness, float metallic, vec3 lightColor)
{
    vec3 F0 = mix(vec3(0.04), albedo, metallic);
    vec3 H = normalize(V + L);

    float D = D_GGX(N, H, roughness);
    float G = G_Smith(N, V, L, roughness);
    vec3 F = F_Schlick(max(dot(H, V), 0.0), F0);

    vec3 kS = F;
    vec3 kD = (vec3(1.0) - kS) * (1.0 - metallic);

    vec3 numerator = D * G * F;
    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
    vec3 specular = numerator / denominator;

    float NdotL = max(dot(N, L), 0.0);
    return (kD * albedo / PI + specular) * NdotL * lightColor;
}

vec3 evaluateIBL(const MaterialInputs material) {
    vec3 N = normalize(material.normal);
    vec3 V = normalize(shading_view);
    vec3 R = reflect(-V, N);

    float metallic  = material.metallic;
    float roughness = max(material.roughness, 0.045);
    vec3  albedo    = material.baseColor.rgb;
    vec3  F0        = mix(vec3(0.04), albedo, metallic);
    float NdotV     = max(dot(N, V), 1e-4);
    float mipCount  = frameUniforms.iblParams.x;
    float ao        = material.ambientOcclusion;

    vec3 irradiance = textureLod(irradianceMap, N, 0.0).rgb;
    vec3 prefilteredColor = textureLod(prefilterMap, R, roughness * (mipCount - 1.0)).rgb;
    vec2 envBRDF = textureLod(brdfLut, vec2(NdotV, roughness), 0.0).rg;

    vec3 kS = F_Schlick(NdotV, F0);
    vec3 kD = mix(vec3(1.0) - kS, vec3(0.0), metallic);

    vec3 diffuse  = kD * irradiance * albedo;
    vec3 specular = prefilteredColor * (kS * envBRDF.x + envBRDF.y);

    return (diffuse + specular) * ao;
}

vec3 CalculateLighting(const MaterialInputs material)
{
    vec3 N = normalize(material.normal);
    vec3 albedo = material.baseColor.rgb;
    float roughness = material.roughness;
    float metallic = material.metallic;
    float AO = material.ambientOcclusion;

    vec3 color = vec3(0.0);

    for (int i = 0; i < lightData.NumLights && i < 16; i++)
    {
        if (int(lightData.lights[i].direction.w) != 0) continue;

        vec3 L = -normalize(lightData.lights[i].direction.xyz);
        vec3 radiance = lightData.lights[i].color.rgb * lightData.lights[i].color.a;
        color += CalculateLighting_PBR(L, N, shading_view, albedo, roughness, metallic, radiance);
    }

    color += evaluateIBL(material);
    return color;
}

vec4 evaluateMaterialLit(const MaterialInputs material) {
    return vec4(CalculateLighting(material), 1.0);
}

#else
vec4 evaluateMaterialLit(const MaterialInputs material) { return material.baseColor; }
#endif
