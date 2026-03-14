// Structures.glsl
struct Light {
    vec4 position;  // w: 光源类型 (0: 方向光, 1: 点光源, 2: 聚光灯)
    vec4 color;     // w: 强度 (Intensity)
    vec4 direction; // w: 影响半径 (Range / Attenuation Radius)
    vec4 params;    // x: 聚光灯内角, y: 聚光灯外角, z: 是否产生阴影, w: 预留
}; 

layout (std140, binding = 0) uniform FrameData {
    mat4 uView;
    mat4 uProjection;
    mat4 uInvView;
    mat4 uInvProjection;
    vec3 uCamPos;
    int RenderMode;
};

layout (std140, binding = 1) uniform LightData {
    Light lights[16];
    mat4 uLightVP;
	int NumLights;
};