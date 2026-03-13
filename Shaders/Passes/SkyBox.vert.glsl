
layout (location = 0) in vec3 aPos;

out vec3 vTexCoords;

uniform mat4 uProjection;
uniform mat4 uView;

void main()
{
    vTexCoords = aPos;
    gl_Position = projection * uView * vec4(aPos, 1.0);
}