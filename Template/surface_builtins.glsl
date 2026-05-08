// Stub declarations for Filament built-in functions and uniforms.
// Replace with real engine implementations as the project matures.

// --- Global uniform blocks (stubs) ---
layout(std140, binding = 1) uniform FrameUniforms {
    vec2 clipControl;
    vec4 lightDirection;
    int directionalShadows;
    int cascades;
    vec2 shadowFarAttenuationParams;
    float lodBias;
} frameUniforms;

struct ShadowData {
    float normalBias;
    mat4 lightFromWorldMatrix;
};

layout(std140, binding = 2) uniform ShadowUniforms {
    ShadowData shadows[1];
} shadowUniforms;

layout(std140, binding = 3) uniform ObjectUniforms {
    int flagsChannels;
} objectUniforms;

#define object_uniforms_flagsChannels objectUniforms.flagsChannels
#define FILAMENT_OBJECT_CONTACT_SHADOWS_BIT 1

// --- Math / utility ---
float saturate(float x) { return clamp(x, 0.0, 1.0); }
vec2 saturate(vec2 v) { return clamp(v, 0.0, 1.0); }
vec3 saturate(vec3 v) { return clamp(v, 0.0, 1.0); }
vec4 saturate(vec4 v) { return clamp(v, 0.0, 1.0); }

// --- Frame / transform helpers ---
mat3 getWorldFromModelNormalMatrix() { return mat3(1.0); }
mat4 getViewFromWorldMatrix() { return mat4(1.0); }
vec3 getWorldPosition() { return vec3(0.0); }
vec3 getWorldCameraPosition() { return vec3(0.0); }

// --- Vertex helpers ---
#ifdef VERTEX_STAGE
vec4 ComputeWorldPosition() { return mesh_position; }
#endif

void toTangentFrame(vec4 q, out vec3 n) 
{
    n = vec3( 0.0,  0.0,  1.0) +
        vec3( 2.0, -2.0, -2.0) * q.x * q.zwx +
        vec3( 2.0,  2.0, -2.0) * q.y * q.wzy;
}

void toTangentFrame(vec4 q, out vec3 n, out vec3 t) {
    toTangentFrame(q, n);
    t = vec3( 1.0,  0.0,  0.0) +
        vec3(-2.0,  2.0, -2.0) * q.y * q.yxw +
        vec3(-2.0,  2.0,  2.0) * q.z * q.zwx;
}

// --- Shadow helpers ---
int getShadowCascade() { return 0; }
vec4 getShadowPosition(int cascade) { return vec4(0.0); }
float shadow(bool, sampler2D, int, vec4, float) { return 1.0; }
float screenSpaceContactShadow(vec3) { return 0.0; }
vec4 computeLightSpacePosition(vec3 wp, vec3 wn, vec3 ld, float nb, mat4 lfm) { return vec4(0.0); }

// --- Material helpers ---
#ifndef VERTEX_STAGE
void prepareMaterial(inout MaterialInputs material) {}
#endif
vec3 inverseTonemapSRGB(vec3 c) { return c; }
