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

uniform vec3 CamPos;

float near = 0.1; 
float far  = 100.0; 

float LinearizeDepth(float depth) 
{
    float z = depth * 2.0 - 1.0; // 转换为 NDC
    return (2.0 * near * far) / (far + near - z * (far - near));    
}

mat3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir)
{
    vec3 lightDir = normalize(-light.direction);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    float shinness = 5.0;
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(reflectDir, viewDir), 0.0), shinness);

    // return abmbient, diffuse, specular
    // vec3 specular = (light.specular * vec3(texture(gSpecular, TexCoords)));
    return mat3(light.ambient, light.diffuse * diff, light.specular * spec);
}

mat3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir = normalize(light.position - fragPos);
    vec3 diffuse = max(dot(normal, lightDir), 0.1) * light.diffuse;

    float shinness = 5.0;
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(reflectDir, viewDir), 0.0), shinness);

    return mat3(light.ambient, diffuse, light.specular * spec);
}

// ec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
// 
//    return vec3(0.0, 0.0, 0.0);
// 

vec4 CalculateLighting()
{
    vec3 FragPos = texture(gPosition, vTexCoords).rgb;
    vec3 Normal = texture(gNormal, vTexCoords).rgb;
    vec3 Albedo = texture(gAlbedo, vTexCoords).rgb;

    mat3 result = CalcDirLight(dirLight, Normal, CamPos);
    
    for (int i = 0; i < uLightCount; i++)
        result += CalcPointLight(pointLights[i], Normal, FragPos, CamPos);

    // result = CalculateSpotLight();
    
    vec3 diffuse = Albedo * (result[0] + result[1]);

    return vec4(diffuse, 1.0);
}

void main()
{
    switch (uDebugMode) {
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