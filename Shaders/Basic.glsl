#shader vertex
#version 330 core

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoords;
layout(location = 3) in vec3 aTangent;

uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProjection;

out vec3 vPosition;
out vec3 vNormal;
out vec2 vTexCoords;
out mat3 vTBN;

void main()
{
    vPosition = vec3(uModel * vec4(aPosition, 1.0));
    vNormal = mat3(transpose(inverse(uModel))) * aNormal;

    mat4 modelView = uView * uModel;
    vec3 T = normalize(mat3(modelView) * aTangent);
    vec3 N = normalize(mat3(modelView) * vNormal);
    vTBN = mat3(T, cross(N, T), N);

    vTexCoords = aTexCoords;
    gl_Position = uProjection * uView * vec4(vPosition, 1.0);
};

#shader fragment
#version 330 core

in vec3 vPosition;
in vec3 vNormal;
in vec2 vTexCoords;
in mat3 vTBN;

layout(location = 0) out vec3 gPosition;
layout(location = 1) out vec3 gNormal;
layout(location = 2) out vec3 gAlbedo;
layout(location = 3) out vec4 gMaterial; // metallic & roughness
layout(location = 4) out uint gObjectID;

uniform sampler2D uAlbedoMap;
uniform sampler2D uNormalMap;
uniform sampler2D uMetallicRoughnessMap;

void main()
{
    gPosition = vPosition;
    gNormal = normalize(vNormal);
    gAlbedo = texture(uAlbedoMap, vTexCoords).rgb;
    gMaterial = vec4(1.0);
};