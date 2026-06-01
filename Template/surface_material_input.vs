struct MaterialVertexInputs {
#ifdef HAS_ATTRIBUTE_COLOR
    vec4 color;
#endif
#ifdef HAS_ATTRIBUTE_UV0
    vec2 uv0;
#endif
#ifdef HAS_ATTRIBUTE_UV1
    vec2 uv1;
#endif
#ifdef VARIABLE_CUSTOM0
    vec4 VARIABLE_CUSTOM0;
#endif
#ifdef VARIABLE_CUSTOM1
    vec4 VARIABLE_CUSTOM1;
#endif
#ifdef VARIABLE_CUSTOM2
    vec4 VARIABLE_CUSTOM2;
#endif
#ifdef VARIABLE_CUSTOM3
    vec4 VARIABLE_CUSTOM3;
#endif
#ifdef HAS_ATTRIBUTE_TANGENTS
    vec3 worldNormal;
#endif
    vec4 worldPosition;
};

vec4 getWorldPosition(const MaterialVertexInputs material) {
    return material.worldPosition;
}

void initMaterialVertex(out MaterialVertexInputs inputs) {
    inputs.worldPosition = computeWorldPosition();
#ifdef HAS_ATTRIBUTE_COLOR
    inputs.color = vec4(1.0);
#endif
#ifdef HAS_ATTRIBUTE_UV0
    inputs.uv0 = mesh_uv0.xy;
#endif
#ifdef HAS_ATTRIBUTE_UV1
    inputs.uv1 = mesh_uv1.xy;
#endif
#ifdef VARIABLE_CUSTOM0
    inputs.VARIABLE_CUSTOM0 = vec4(0.0);
#endif
#ifdef VARIABLE_CUSTOM1
    inputs.VARIABLE_CUSTOM1 = vec4(0.0);
#endif
#ifdef VARIABLE_CUSTOM2
    inputs.VARIABLE_CUSTOM2 = vec4(0.0);
#endif
#ifdef VARIABLE_CUSTOM3
    inputs.VARIABLE_CUSTOM3 = vec4(0.0);
#endif
}