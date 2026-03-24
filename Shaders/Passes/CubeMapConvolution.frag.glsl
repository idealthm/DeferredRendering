layout (location = 0) out vec4 FragColor;

in vec3 vTexCoords;

uniform samplerCube uCubeMap;

const float PI = 3.14159265359;

void main()
{
    vec3 normal = normalize(vTexCoords);
    vec3 irradiance = vec3(0.0);

    vec3 up    = vec3(0.0, 1.0, 0.0);
    vec3 right = normalize(cross(up, normal));
    up         = normalize(cross(normal, right));

    mat3 TVB = mat3(right, up, normal);

    float sampleCount = 60;
    float sampleStep = 1.0 / sampleCount;
    float nrSample = 0;

    for (float phi = 0.0f; phi < 2.0 * PI; phi += sampleStep)
    {
        for (float theta = 0.0f; theta < 0.5 * PI; theta += sampleStep)
        {
            vec3 tangentDir = vec3(cos(phi) * sin(theta), sin(phi) * sin(theta), cos(theta));
            vec3 sampleDir = TVB * tangentDir;
            irradiance += texture(uCubeMap, sampleDir).rgb * cos(theta) * sin(theta);
            nrSample += 1;
        }
    }

    FragColor = vec4(PI * irradiance / nrSample, 1.0);
}
