void main()
{
    MaterialVertexInputs material;
    initMaterialVertex(material);

    // Extract the normal and tangent in world space from the input quaternion
    // We encode the orthonormal basis as a quaternion to save space in the attributes
    toTangentFrame(mesh_tangents, material.worldNormal, vertex_worldTangent.xyz);

    // We don't need to normalize here, even if there's a scale in the matrix
    // because we ensure the worldFromModelNormalMatrix pre-scales the normal such that
    // all its components are < 1.0. This prevents the bitangent to exceed the range of fp16
    // in the fragment shader, where we renormalize after interpolation
    vertex_worldTangent.xyz = getWorldFromModelNormalMatrix() * vertex_worldTangent.xyz;
    vertex_worldTangent.w = mesh_tangents.w;
    material.worldNormal = getWorldFromModelNormalMatrix() * material.worldNormal;
        
    
        // Invoke user code
        materialVertex(material);
    
        // Handle built-in interpolated attributes
    #if defined(HAS_ATTRIBUTE_COLOR)
        vertex_color = material.color;
    #endif
    #if defined(HAS_ATTRIBUTE_UV0)
        vertex_uv01.xy = material.uv0;
    #endif
    #if defined(HAS_ATTRIBUTE_UV1)
        vertex_uv01.zw = material.uv1;
    #endif
    
        // Handle user-defined interpolated attributes
    #if defined(VARIABLE_CUSTOM0)
        VARIABLE_CUSTOM_AT0 = material.VARIABLE_CUSTOM0;
    #endif
    #if defined(VARIABLE_CUSTOM1)
        VARIABLE_CUSTOM_AT1 = material.VARIABLE_CUSTOM1;
    #endif
    #if defined(VARIABLE_CUSTOM2)
        VARIABLE_CUSTOM_AT2 = material.VARIABLE_CUSTOM2;
    #endif
    #if defined(VARIABLE_CUSTOM3)
        VARIABLE_CUSTOM_AT3 = material.VARIABLE_CUSTOM3;
    #endif
    
        // The world position can be changed by the user in materialVertex()
        vertex_worldPosition.xyz = material.worldPosition.xyz;
    
    #ifdef HAS_ATTRIBUTE_TANGENTS
        vertex_worldNormal = material.worldNormal;
    #endif
    
    #if defined(VARIANT_HAS_SHADOWING) && defined(VARIANT_HAS_DIRECTIONAL_LIGHTING)
        vertex_lightSpacePosition = computeLightSpacePosition(
                vertex_worldPosition.xyz, vertex_worldNormal,
                frameUniforms.lightDirection,
                shadowUniforms.shadows[0].normalBias,
                shadowUniforms.shadows[0].lightFromWorldMatrix);
    #endif
    
    #endif // !defined(USE_OPTIMIZED_DEPTH_VERTEX_SHADER)
    
        vec4 position;
    
    #if defined(VERTEX_DOMAIN_DEVICE)
        // The other vertex domains are handled in initMaterialVertex()->computeWorldPosition()
        position = getPosition();
    
        // this must happen before we compensate for vulkan below
        vertex_position = position;
    
    #if defined(TARGET_VULKAN_ENVIRONMENT)
        // In Vulkan, clip space is Y-down. In OpenGL and Metal, clip space is Y-up.
        position.y = -position.y;
    #endif
    
    #if !defined(TARGET_VULKAN_ENVIRONMENT)
        // This is not needed in Vulkan or Metal because clipControl is always (1, 0)
        // (We don't use a dot() here because it workaround a spirv-opt optimization that in turn
        //  causes a crash on PowerVR, see #5118)
        position.z = position.z * frameUniforms.clipControl.x + position.w * frameUniforms.clipControl.y;
    #endif
    
        // some PowerVR drivers crash when gl_Position is written more than once
        gl_Position = position;
#endif
}