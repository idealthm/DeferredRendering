// GBuffer channel layout — single source of truth, included by both deferred write and lighting read
#define GBUFFER_ALBEDO       gAlbedo
#define GBUFFER_NORMAL       gNormal
#define GBUFFER_MATERIAL     gMaterial
#define GBUFFER_MAT_ROUGHNESS    r
#define GBUFFER_MAT_METALLIC     g
#define GBUFFER_MAT_AO           b
#define GBUFFER_MAT_SHADING_ID   a
#define GBUFFER_SHADING_SCALE    255.0
