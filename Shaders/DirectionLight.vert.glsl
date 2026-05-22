layout(location = 0) in vec4 aPosition;

out vec2 vTexCoords;

void main()
{
    vTexCoords = aPosition.xy * 0.5 + 0.5;
    gl_Position = vec4(aPosition.xy, 0.0, 1.0);
};