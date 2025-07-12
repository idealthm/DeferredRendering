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

out vec4 FragColor;

struct DirLight {
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct PointLight {
    vec3 position;
    float constant;
    float linear;
    float quadratic;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

in vec2 vTexCoords;

#define NR_POINT_LIGHTS 4

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedo;
uniform sampler2D gMaterial;

uniform int uDebugMode;
uniform int uLightCount;

uniform DirLight dirLight;
uniform PointLight pointLights[NR_POINT_LIGHTS];

uniform vec3 uCamPos;

// 计算光照贡献（返回环境光、漫反射、高光）
mat3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir)
{
    vec3 lightDir = normalize(-light.direction);
    
    // 漫反射
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = light.diffuse * diff;
    
    // 高光 - 修正反射方向
    float shininess = 64.0;
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(reflectDir, viewDir), 0.0), shininess);
    vec3 specular = light.specular * spec;
    
    // 环境光
    return mat3(light.ambient, diffuse, specular);
}

mat3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir = normalize(light.position - fragPos);
    
    // 漫反射
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = light.diffuse * diff;
    
    // 高光
    float shininess = 5.0;
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(reflectDir, viewDir), 0.0), shininess);
    vec3 specular = light.specular * spec;
    
    // 衰减
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + 
                             light.quadratic * (distance * distance));
    
    return mat3(light.ambient, diffuse * attenuation, specular * attenuation);
}

vec4 CalculateLighting(bool withSpecular)
{
    vec3 FragPos = texture(gPosition, vTexCoords).rgb;
    vec3 Normal = normalize(texture(gNormal, vTexCoords).rgb); // 确保归一化
    vec3 Albedo = texture(gAlbedo, vTexCoords).rgb;
    vec3 viewDir = normalize(uCamPos - FragPos); // 统一视图方向
    
    // 计算方向光
    mat3 dirResult = CalcDirLight(dirLight, Normal, viewDir);
    
    // 计算点光源
    for (int i = 0; i < uLightCount; i++) {
        dirResult += CalcPointLight(pointLights[i], Normal, FragPos, viewDir);
    }
    
    // 组合光照
    vec3 ambientColor = dirResult[0] * Albedo;
    vec3 diffuseColor = dirResult[1] * Albedo;
    vec3 resultColor = ambientColor + diffuseColor;
    if (withSpecular == true)
        resultColor += dirResult[2];
    
    return vec4(resultColor, 1.0);
}

vec4 CammeraToFragPos()
{
    vec3 FragPos = texture(gPosition, vTexCoords).rgb;
    return vec4(FragPos / 100, 1.0);
}

void main()
{
    switch (uDebugMode) {
        case 1: // 位置
            FragColor = vec4(texture(gPosition, vTexCoords).rb, 1.0, 1.0);
            break;
        case 2: // 法线
            vec3 normal = texture(gNormal, vTexCoords).rgb;
            FragColor = vec4(normal * 0.5 + 0.5, 1.0);
            break;
        case 3: // 反照率
            FragColor = vec4(texture(gAlbedo, vTexCoords).rgb, 1.0);
            break;
        case 4: // 粗糙度
            float roughness = texture(gMaterial, vTexCoords).g; // 假设粗糙度存储在g通道
            FragColor = vec4(vec3(roughness), 1.0);
            break;
        case 5: // Depth
            FragColor = CalculateLighting(false);
            break;
        default:
            FragColor = CalculateLighting(true);
    }
};