#include <Common/Structures.glsl>

layout(location = 0) in vec4 aPosition;

out vec3 vTexCoords;

void main()
{
    vTexCoords = aPosition.xyz;
    vec4 pos = uProjection * mat4(mat3(uView)) * vec4(aPosition.xyz, 1.0);
    gl_Position = pos.xyww;
}