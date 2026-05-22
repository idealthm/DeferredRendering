#include "../Common/Structures.glsl"

layout(location = 0) in vec4 aPosition;
layout(location = 1) in vec4 aNormal;
layout(location = 2) in vec4 aTangent;
layout(location = 3) in vec2 aTexCoord0;

out vec3 WorldPos;
out vec2 TexCoords;
out vec3 Normal;

uniform mat4 model;
uniform mat3 normalMatrix;

void main()
{
    vec4 worldPos4 = model * vec4(aPosition.xyz, 1.0);
    WorldPos = worldPos4.xyz;

    TexCoords = aTexCoord0;

    Normal = normalize(normalMatrix * aNormal.xyz);

    gl_Position = uProjection * uView * worldPos4;
}
