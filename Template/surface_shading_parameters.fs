// computed shading Params
vec3 GetWorldNormal()
{
    return shading_normal;
}

vec3 GetReflectedVector()
{
    return shading_reflected;
}

vec3 GetWorldPosition()
{
    return shading_position;
}

vec3 GetViewVector()
{
    return shading_view;
}

float GetNoVDot()
{
    return shading_NoV;
}

void computeShadingParams(vec3 position, vec3 normal)
{
    shading_geometricNormal = normalize(normal);

    shading_position = position;

    shading_view = normalize(frameUniforms.worldFromViewMatrix[3].xyz - shading_position);
}

void prepareMaterial(const MaterialInputs material)
{
#if defined(MATERIAL_HAS_NORMAL)
    shading_normal = normalize(shading_tangentToWorld * material.normal);
#else
    shading_normal = shading_geometricNormal;
#endif
    shading_NoV = max(dot(shading_normal, shading_view), 1e-4);
    shading_reflected = reflect(-shading_view, shading_normal);
}