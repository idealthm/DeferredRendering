layout (location = 0) out vec4 FragColor;

in vec3 vTexCoords;

uniform samplerCube uCubeMap;

void main()
{    
    FragColor = vec4(texture(uCubeMap, vTexCoords).rgb, 1.0);
}
