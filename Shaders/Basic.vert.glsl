#include "Common/Structures.glsl"

layout(location = 0) in vec4 aPosition;
layout(location = 1) in vec4 aNormal;
layout(location = 2) in vec4 aTangent;
layout(location = 3) in vec2 aTexCoord0;

uniform mat4 uModel;

out vec3 vPosition;
out vec2 vTexCoords;
out mat3 vTBN;

void main()
{
    vPosition = vec3(uModel * vec4(aPosition.xyz, 1.0));
    mat3 normalMatrix = mat3(transpose(inverse(uModel)));

    vec3 T = normalize(mat3(normalMatrix) * aTangent.xyz);
    vec3 N = normalize(mat3(normalMatrix) * aNormal.xyz);

    T = normalize(T - dot(T, N) * N);
    vTBN = mat3(T, cross(N, T), N);

    vTexCoords = aTexCoord0;
    gl_Position = uProjection * uView * vec4(vPosition, 1.0);
}
