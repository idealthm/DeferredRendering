layout(location = 0) in vec3 aPos;

out vec3 vTexCoords;

uniform mat4 uView;
uniform mat4 uProjection;

void main()
{
    gl_Position = uProjection * uView * vec4(aPos, 1.0);
    vTexCoords = aPos;
};