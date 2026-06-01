//------------------------------------------------------------------------------
// Attributes access
//------------------------------------------------------------------------------

#if defined(HAS_ATTRIBUTE_COLOR)
/** @public-api */
vec4 getColor() {
    return vertex_color;
}
#endif

#if defined(HAS_ATTRIBUTE_UV0) || defined(HAS_ATTRIBUTE_UV1)
/** @public-api */
highp vec2 getUV0() {
#if defined(HAS_ATTRIBUTE_UV1)
    return vertex_uv01.xy;
#else
    return vertex_uv01;
#endif
}

#if defined(HAS_ATTRIBUTE_UV1)
/** @public-api */
highp vec2 getUV1() {
    return vertex_uv01.zw;
}
#endif
#endif

/** @public-api */
highp mat3 getWorldTangentFrame() {
    return shading_tangentToWorld;
}

/** @public-api */
highp vec3 getWorldPosition() {
    return shading_position;
}

/** @public-api */
vec3 getWorldViewVector() {
    return shading_view;
}

bool isPerspectiveProjection() {
    return frameUniforms.clipFromViewMatrix[2].w != 0.0;
}

#if defined(HAS_ATTRIBUTE_TANGENTS)

/** @public-api */
vec3 getWorldNormalVector() {
    return shading_normal;
}

/** @public-api */
vec3 getWorldGeometricNormalVector() {
    return shading_geometricNormal;
}

/** @public-api */
vec3 getWorldReflectedVector() {
    return shading_reflected;
}

/** @public-api */
float getNdotV() {
    return shading_NoV;
}

#endif

highp vec3 getNormalizedPhysicalViewportCoord() {
    // make sure to handle our reversed-z
    return vec3(shading_normalizedViewportCoord, gl_FragCoord.z);
}