
layout(location = 0) out vec3 gAlbedo;
layout(location = 1) out vec3 gNormal;
// location 2: reconstructed from depth, no output needed
layout(location = 2) out vec4 gMaterial;

void main() {
    logical_instance_index = instance_index;
    initObjectUniforms();

    vec3 n = vertex_worldNormal;
    vec3 t = vertex_worldTangent.xyz;
    vec3 b = cross(n, t) * vertex_worldTangent.w;

    shading_tangentToWorld = mat3(t, b, n);
    computeShadingParams(vertex_worldPosition.xyz, vertex_worldNormal);

    MaterialInputs inputs;
    initMaterial(inputs);

    material(inputs);

    gAlbedo = inputs.baseColor.xyz;

    gMaterial = vec4(inputs.roughness, inputs.metallic, inputs.ambientOcclusion,
                     float(SHADING_MODEL_ID) / 255.0);

    gNormal = shading_normal;
}
