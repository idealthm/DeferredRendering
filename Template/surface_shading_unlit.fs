
vec4 evaluateMaterial(const MaterialInputs material) {
    vec4 color = material.baseColor;

#if defined(VARIANT_HAS_DIRECTIONAL_LIGHTING)
#if defined(VARIANT_HAS_SHADOWING)
    float visibility = 1.0;
    int cascade = getShadowCascade();
    bool cascadeHasVisibleShadows = bool(frameUniforms.cascades & ((1 << cascade) << 8));
    bool hasDirectionalShadows = bool(frameUniforms.directionalShadows & 1);
    if (hasDirectionalShadows && cascadeHasVisibleShadows) {
        highp vec4 shadowPosition = getShadowPosition(cascade);
        visibility = shadow(true, sampler0_shadowMap, cascade, shadowPosition, 0.0);
        // shadow far attenuation
        highp vec3 v = getWorldPosition() - getWorldCameraPosition();
        // (viewFromWorld * v).z == dot(transpose(viewFromWorld), v)
        highp float z = dot(transpose(getViewFromWorldMatrix())[2].xyz, v);
        highp vec2 p = frameUniforms.shadowFarAttenuationParams;
        visibility = 1.0 - ((1.0 - visibility) * saturate(p.x - z * z * p.y));
    }
    if ((frameUniforms.directionalShadows & 0x2) != 0 && visibility > 0.0) {
        if ((object_uniforms_flagsChannels & FILAMENT_OBJECT_CONTACT_SHADOWS_BIT) != 0) {
            visibility *= (1.0 - screenSpaceContactShadow(frameUniforms.lightDirection));
        }
    }
    color *= 1.0 - visibility;
#else
    color = vec4(0.0);
#endif
#elif defined(MATERIAL_HAS_SHADOW_MULTIPLIER)
    color = vec4(0.0);
#endif

    // addEmissive(material, color);

    return color;
}
