// GBuffer sampler bindings (G_BUFFER descriptor set)
layout(binding = 0) uniform sampler2D gDepth;
layout(binding = 1) uniform sampler2D gNormal;
layout(binding = 2) uniform sampler2D gAlbedo;
layout(binding = 3) uniform sampler2D gMaterial;

// LightData UBO (PER_VIEW set, binding 1)
struct LightInfo {
    vec4 position;    // xyz=pos, w=range
    vec4 color;       // rgb=color, w=intensity
    vec4 direction;   // xyz=dir, w=type
    vec4 params;      // x=spotInner, y=spotOuter, z=castShadow, w=unused
};

layout(binding = 1, std140) uniform LightData {
    LightInfo lights[16];
    mat4 uLightVP;
    int NumLights;
} lightData;
