#include <Common/Structures.glsl>

layout (location = 0) in vec3 aPos;

out vec3 vTexCoords;

void main()
{
    vTexCoords = aPos;
    vec4 pos = uProjection * mat4(mat3(uView)) * vec4(aPos, 1.0);
    gl_Position = pos.xyww;
}