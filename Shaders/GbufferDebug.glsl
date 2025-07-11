#shader vertex
#version 330 core

layout(location = 0) in vec2 aPos;

out vec2 vTexCoords;

void main()
{
    vTexCoords = aPos * 0.5 + 0.5;
    gl_Position = vec4(aPos, 0.0, 1.0);
};

#shader fragment
#version 330 core

in vec2 vTexCoords

out vec4 FragColor;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedo;
uniform sampler2D gMaterial;

uniform int uDebugMode;

vec4 CalculateLighting()
{
    vec3 FragPos = texture(gPosition, TexCoords).rgb;
    vec3 Normal = texture(gNormal, TexCoords).rgb;
    vec3 Albedo = texture(gAlbedo, TexCoords).rgb;

    mat3 result = CalcDirLight(dirLight, Normal, CamPos);
    
    for (int i = 0; i < NR_POINT_LIGHTS; i++)
        result += CalcPointLight(pointLights[i], Normal, FragPos, CamPos);

    vec3 diffuse = Albedo * (result[0] + result[1]) + result[2] * 0.5;
    return diffuse;
}

void main()
{
    switch (debugMode) {
        case 1: // 位置
            FragColor = vec4(texture(gPosition, vTexCoords).rgb, 1.0);
            break;
        case 2: // 法线
            vec3 normal = texture(gNormal, vTexCoords).rgb;
            FragColor = vec4(normal * 0.5 + 0.5, 1.0);
            break;
        case 3: // 反照率
            FragColor = vec4(texture(gAlbedo, vTexCoords).rgb, 1.0);
            break;
        case 4: // 粗糙度
            float roughness = texture(gNormal, vTexCoords).a;
            FragColor = vec4(vec3(roughness), 1.0);
            break;
        // ... 其他通道
        default:
            FragColor = CalculateLighting();
    }
};