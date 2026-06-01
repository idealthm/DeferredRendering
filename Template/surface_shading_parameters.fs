void computeShadingParams()
{
    highp vec3 n = vertex_worldNormal;
#if defined(MATERIAL_NEEDS_TBN)
    highp vec3 t = vertex_worldTangent.xyz;
    highp vec3 b = cross(n, t) * sign(vertex_worldTangent.w);
#else
    highp vec3 t = vec3(1.0, 0.0, 0.0);
    highp vec3 b = vec3(0.0, 1.0, 0.0);
#endif

#if defined(MATERIAL_HAS_DOUBLE_SIDED_CAPABILITY)
    if (isDoubleSided()) {
        n = gl_FrontFacing ? n : -n;
#if defined(MATERIAL_NEEDS_TBN)
        t = gl_FrontFacing ? t : -t;
        b = gl_FrontFacing ? b : -b;
#endif
    }
#endif

    shading_geometricNormal = normalize(n);

    shading_tangentToWorld = mat3(t, b, n);

    shading_position = vertex_worldPosition.xyz;

    // With perspective camera, the view vector is cast from the fragment pos to the eye position,
    // With ortho camera, however, the view vector is the same for all fragments:
    highp vec3 sv = frameUniforms.worldFromViewMatrix[3].xyz - shading_position;
    shading_view = normalize(sv);

    // we do this so we avoid doing (matrix multiply), but we burn 4 varyings:
    //    p = clipFromWorldMatrix * shading_position;
    //    shading_normalizedViewportCoord = p.xy * 0.5 / p.w + 0.5
    shading_normalizedViewportCoord = vertex_position.xy * (0.5 / vertex_position.w) + 0.5;
}

/**
 * Computes global shading parameters that the material might need to access
 * before lighting: N dot V, the reflected vector and the shading normal (before
 * applying the normal map). These parameters can be useful to material authors
 * to compute other material properties.
 *
 * This function must be invoked by the user's material code (guaranteed by
 * the material compiler) after setting a value for MaterialInputs.normal.
 */
void prepareMaterial(const MaterialInputs material) {
    shading_normal = normalize(shading_tangentToWorld * material.normal);
    shading_normal = getWorldGeometricNormalVector();
    shading_NoV = max(dot(shading_normal, shading_view), 1e-4);
    shading_reflected = reflect(-shading_view, shading_normal);
}