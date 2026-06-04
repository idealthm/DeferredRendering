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

bool isPerspectiveProjection() {
    return frameUniforms.clipFromViewMatrix[2].w != 0.0;
}
