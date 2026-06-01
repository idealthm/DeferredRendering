
mat4 getWorldFromModelMatrix() {
    return object_uniforms_worldFromModelMatrix;
}

mat3 getWorldFromModelNormalMatrix() {
    return object_uniforms_worldFromModelNormalMatrix;
}

int getVertexIndex() {
    return gl_VertexID;
}

vec4 getPosition() { return mesh_position; }

vec4 computeWorldPosition() {
    mat4 transform = getWorldFromModelMatrix();
    vec3 position = getPosition().xyz;
    return mulMat4x4Float3(transform, position);
}

#if defined(HAS_ATTRIBUTE_CUSTOM0)
vec4 getCustom0() { return mesh_custom0; }
#endif
#if defined(HAS_ATTRIBUTE_CUSTOM1)
vec4 getCustom1() { return mesh_custom1; }
#endif
#if defined(HAS_ATTRIBUTE_CUSTOM2)
vec4 getCustom2() { return mesh_custom2; }
#endif
#if defined(HAS_ATTRIBUTE_CUSTOM3)
vec4 getCustom3() { return mesh_custom3; }
#endif
#if defined(HAS_ATTRIBUTE_CUSTOM4)
vec4 getCustom4() { return mesh_custom4; }
#endif
#if defined(HAS_ATTRIBUTE_CUSTOM5)
vec4 getCustom5() { return mesh_custom5; }
#endif
#if defined(HAS_ATTRIBUTE_CUSTOM6)
vec4 getCustom6() { return mesh_custom6; }
#endif
#if defined(HAS_ATTRIBUTE_CUSTOM7)
vec4 getCustom7() { return mesh_custom7; }
#endif