// Framework: reads GBuffer, dispatches to shading model, outputs fragColor
LAYOUT_LOCATION(0) in vec2 variable_uv;

layout(location = 0) out vec4 fragColor;

vec3 reconstructWorldPosition(vec2 uv, float depth)
{
    vec4 clipPos = vec4(uv * 2.0 - 1.0, depth * 2.0 - 1.0, 1.0);
    vec4 worldPos = frameUniforms.worldFromClipMatrix * clipPos;
    return worldPos.xyz / worldPos.w;
}

void main()
{
    vec4 gbuf1 = textureLod(gNormal,   variable_uv, 0.0);
    vec4 gbuf2 = textureLod(gAlbedo,   variable_uv, 0.0);
    vec4 gbuf3 = textureLod(gMaterial, variable_uv, 0.0);
    float depth = textureLod(gDepth,   variable_uv, 0.0).r;

    MaterialInputs material;
    material.baseColor = vec4(gbuf2.rgb, 1.0);
    material.roughness = gbuf3.r;
    material.metallic  = gbuf3.g;
    material.ambientOcclusion = gbuf3.b;
    material.normal    = gbuf1.rgb;
    material.emissive  = vec4(0.0);

    computeShadingParams(reconstructWorldPosition(variable_uv, depth), material.normal);

    shading_NoV = max(dot(shading_normal, shading_view), 1e-4);
    shading_reflected = reflect(-shading_view, shading_normal);

    // Debug: set frameUniforms.RenderMode to 2..5 to inspect GBuffer data
    if (frameUniforms.RenderMode == 2)   { fragColor = vec4(shading_position, 1.0);          return; }
    if (frameUniforms.RenderMode == 3)   { fragColor = vec4(shading_view, 1.0); return; }
    if (frameUniforms.RenderMode == 4)   { fragColor = vec4(material.normal * 0.5 + 0.5, 1.0);     return; }
    if (frameUniforms.RenderMode == 5)   { fragColor = vec4(shading_NoV,shading_NoV,shading_NoV, 1.0);               return; }

    int shadingModel = int(gbuf3.a * 255.0 + 0.5);

    if (shadingModel == 0)
        fragColor = evaluateMaterialUnlit(material);
    else
        fragColor = evaluateMaterialLit(material);
}
