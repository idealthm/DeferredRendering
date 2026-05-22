// GBuffer.frag
#include "../Common/Structures.glsl"
#include "../Common/Packing.glsl"

in vec3 WorldPos;
in vec2 TexCoords;
in vec3 Normal;

layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec3 gAlbedo; // RGB: Albedo,
layout (location = 3) out vec4 gRMS;    // R: Roughness, G: Metallic, B: AO

uniform sampler2D texture_albedo;
uniform sampler2D texture_metallic;
uniform sampler2D texture_roughness;

void main() {
    gPosition = WorldPos;
    gNormal = PackNormal(normalize(Normal));
    gAlbedoSpec.rgb = texture(texture_albedo, TexCoords).rgb;
    gRMS.r = texture(texture_roughness, TexCoords).r;
    gRMS.g = texture(texture_metallic, TexCoords).r;
    gRMS.b = 1.0; // 默认 AO
}