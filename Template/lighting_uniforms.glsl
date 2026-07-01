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
