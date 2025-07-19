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

layout(location = 0) out vec4 FragColor;

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


struct SpotLight {
    vec3 position;
    vec3 direction;

    float cutoff;
    float outerCutoff;

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
uniform sampler2D gShadowMap;

uniform mat4 uLightSpaceVP;

uniform int uDebugMode;
uniform int uPointLightCount;
uniform int uSpotLightCount;

uniform DirLight dirLight;
uniform PointLight pointLights[NR_POINT_LIGHTS];
uniform SpotLight spotLights[NR_POINT_LIGHTS];

uniform vec3 uCamPos;

vec3 NormalDecode(vec2 e)
{
    vec2 uv = e * 2.0 - 1.0;
    float z = 1.0 - abs(uv.x) - abs(uv.y);
    
    // 关键修复：正确重建负Z值
    if (z < 0.0) {
        vec2 uv_prev = uv;
        uv.x = (1.0 - abs(uv_prev.y)) * sign(uv_prev.x);
        uv.y = (1.0 - abs(uv_prev.x)) * sign(uv_prev.y);
        z = 1.0 - abs(uv.x) - abs(uv.y); // 重新计算正Z
        z = -z; // 恢复原始负Z值
    }
    return normalize(vec3(uv, z));
}

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

mat3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir = normalize(-light.direction);

    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = light.diffuse * diff;
    
    float shininess = 64.0;
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(reflectDir, viewDir), 0.0), shininess);
    vec3 specular = light.specular * spec;

    vec3 pixelDir = normalize(light.position - fragPos);
    
    float theta = dot(pixelDir, -lightDir);
    float epsilon = light.cutoff - light.outerCutoff;
    float intensity = clamp((theta - light.outerCutoff) / epsilon, 0.0, 1.0);

    diffuse *= intensity;
    specular *= intensity;

    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

    vec3 ambient = light.ambient * attenuation;
    diffuse *= attenuation;
    specular *= attenuation;

    return mat3(ambient, diffuse, specular);
}


float calculateShadowAttenuation(vec4 fragPosLightSpace, vec3 normal, vec3 lightDir)
{
    // depth in light
    vec3 ProjCoord = fragPosLightSpace.xyz / fragPosLightSpace.w;
    ProjCoord = ProjCoord * 0.5 + 0.5;

    float currentDepth = ProjCoord.z;

    float theta = max(dot(normal, lightDir), 0.0);
    float bias = max(0.01 * (1.0 - theta), 0.001);

    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(gShadowMap, 0);
    for(int x = -1; x <= 1; ++x) {
        for(int y = -1; y <= 1; ++y) {
            float closest = texture(gShadowMap, ProjCoord.xy + vec2(x,y) * texelSize).r; 
            shadow += currentDepth - bias > closest ? 1.0 : 0.0;
        }
    }
    shadow /= 9.0;

    // if ( ProjCoord.z > 1.0)
    //     shadow = 0.0;

    return shadow;
}


vec4 CalculateLighting()
{
    vec3 FragPos = texture(gPosition, vTexCoords).rgb;
    vec3 Normal = NormalDecode(texture(gNormal, vTexCoords).rg); // 确保归一化
    vec3 Albedo = texture(gAlbedo, vTexCoords).rgb;
    vec3 viewDir = normalize(uCamPos - FragPos); // 统一视图方向
    
    // 计算方向光
    mat3 dirResult = CalcDirLight(dirLight, Normal, viewDir);
    
    // 计算点光源
    for (int i = 0; i < uPointLightCount; i++) {
        dirResult += CalcPointLight(pointLights[i], Normal, FragPos, viewDir);
    }

    for (int i = 0; i < uSpotLightCount; i++) {
        dirResult += CalcSpotLight(spotLights[i], Normal, FragPos, viewDir);
    }

    float shadowAttenuation = calculateShadowAttenuation(uLightSpaceVP * vec4(FragPos, 1.0), Normal, normalize(dirLight.direction));

    // 组合光照
    vec3 resultColor = vec3(0.0, 0.0, 0.0);
    resultColor += dirResult[0] * Albedo;
    resultColor += dirResult[1] * (1 - shadowAttenuation) * Albedo;
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
            FragColor = vec4(texture(gPosition, vTexCoords).rgb, 1.0);
            break;
        case 2: // 法线
            vec3 normal = NormalDecode(texture(gNormal, vTexCoords).rg);
            FragColor = vec4(normal * 0.5 + 0.5, 1.0);
            break;
        case 3: // 反照率
            FragColor = vec4(texture(gAlbedo, vTexCoords).rgb, 1.0);
            break;
        case 4: // diffuseColor
            FragColor = CalculateLighting();
            break;
        case 5: // Depth
            FragColor = vec4(vec3(texture(gShadowMap, vTexCoords).r), 1.0);
            break;
        default:
            FragColor = CalculateLighting();
    }
};