#shader vertex
#version 330 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 FragPos;
out vec2 TexCoords;
out vec3 Normal;

void main()
{
    vec4 worldPos = model * vec4(aPos, 1.0);
    FragPos = worldPos.xyz; 
    TexCoords = aTexCoords;
    
    mat3 normalMatrix = transpose(inverse(mat3(model)));
    Normal = normalMatrix * aNormal;

    gl_Position = projection * view * worldPos;
};

#shader fragment
#version 330 core

layout(location = 0) out vec3 gPosition;
layout(location = 1) out vec3 gNormal;
layout(location = 2) out vec3 gAlbedo;

uniform sampler2D texture_diffuse0;

in vec3 FragPos;
in vec2 TexCoords;
in vec3 Normal;

void main()
{
    gPosition = FragPos;
    gNormal = normalize(Normal);
    gAlbedo = vec3(1.0);// texture(texture_diffuse0, TexCoords).rgb;
};