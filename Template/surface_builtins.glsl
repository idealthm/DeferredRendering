// Stub declarations for Filament built-in functions and uniforms.
// Replace with real engine implementations as the project matures.

// --- Global uniform blocks (stubs) ---
#define object_uniforms_flagsChannels objectUniforms.flagsChannels
#define FILAMENT_OBJECT_CONTACT_SHADOWS_BIT 1

// --- Shadow helpers ---
int getShadowCascade() { return 0; }
vec4 getShadowPosition(int cascade) { return vec4(0.0); }
float shadow(bool, sampler2D, int, vec4, float) { return 1.0; }
float screenSpaceContactShadow(vec3) { return 0.0; }
vec4 computeLightSpacePosition(vec3 wp, vec3 wn, vec3 ld, float nb, mat4 lfm) { return vec4(0.0); }

// --- Material helpers ---
vec3 inverseTonemapSRGB(vec3 c) { return c; }
