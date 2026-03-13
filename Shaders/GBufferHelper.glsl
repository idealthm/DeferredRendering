#include "Structures.glsl"

layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec3 gAlbedoSpec;
layout (location = 3) out vec2 gRMAO;

void main()
{
    gPosition = texture(gPosition, gRMAO);
}