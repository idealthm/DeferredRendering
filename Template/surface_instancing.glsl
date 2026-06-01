

mat4 object_uniforms_worldFromModelMatrix;
mat3 object_uniforms_worldFromModelNormalMatrix;
// int object_uniforms_morphTargetCount;
// int object_uniforms_flagsChannels;                   // see packFlags() below (0x00000fll)
// int object_uniforms_objectId;                        // used for picking
// float object_uniforms_userData;


void initObjectUniforms()
{
    int i = logical_instance_index;
    
    object_uniforms_worldFromModelMatrix        = objectUniforms.data[i].worldFromModelMatrix;
    object_uniforms_worldFromModelNormalMatrix  = objectUniforms.data[i].worldFromModelNormalMatrix;
}

highp int getInstanceIndex() {
    return logical_instance_index;
}