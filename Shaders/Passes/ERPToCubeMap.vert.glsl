layout(location = 0) in vec4 aPosition;

out vec3 vTexCoords;

uniform mat4 uView;
uniform mat4 uProjection;

void main()
{
    gl_Position = uProjection * uView * vec4(aPosition.xyz, 1.0);
    vTexCoords = aPosition.xyz;
};