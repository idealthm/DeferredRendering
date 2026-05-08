
struct MaterialInputs {
    vec4 baseColor;

    float roughness;

    vec4 emissive;

    float metallic;

    float ambientOcclusion;

    vec3 normal;
};

void initMaterial(out MaterialInputs inputs)
{
    inputs.baseColor = vec4(1.0);

    inputs.roughness = 1.0;

    inputs.emissive = vec4(0.0);

    inputs.metallic = 0.0;

    inputs.ambientOcclusion = 0;

    inputs.normal = vec3(0.0, 0.0, 1.0);
}