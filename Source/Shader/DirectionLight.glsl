#shader vertex
#version 330 core

layout(location = 0) in vec2 aPos;

out vec2 TexCoords;

void main()
{
    TexCoords = aPos * 0.5 + 0.5;
    gl_Position = vec4(aPos, 0.0, 1.0);
};

#shader fragment
#version 330 core

out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedo;

uniform vec3 lightPos;
uniform vec3 lightColor;

float near = 0.1; 
float far  = 100.0; 

float LinearizeDepth(float depth) 
{
    float z = depth * 2.0 - 1.0; // 转换为 NDC
    return (2.0 * near * far) / (far + near - z * (far - near));    
}

void main()
{
    vec3 FragPos = texture(gPosition, TexCoords).rgb;
    vec3 Normal = texture(gNormal, TexCoords).rgb;
    vec3 Albedo = texture(gAlbedo, TexCoords).rgb;

    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(Normal, lightDir), 0.3);
    vec3 diffuse = diff * Albedo * lightColor;
    
    FragColor = vec4(vec3(gl_FragCoord.z), 1.0);
    
    float depth = 0.5 + LinearizeDepth(gl_FragCoord.z) / far; // 为了演示除以 far
    FragColor = vec4(diffuse, 1.0);
};