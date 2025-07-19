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
layout(location = 1) out vec2 gNormal;
layout(location = 2) out vec3 gAlbedo;
layout(location = 3) out vec4 gMaterial; // metallic & roughness
layout(location = 4) out uint gObjectID;

uniform sampler2D uAlbedoMap;
uniform sampler2D uNormalMap;
uniform sampler2D uMetallicRoughnessMap;

vec2 NormalEncode(vec3 n)
{
    n = normalize(n); // 确保单位向量
    float l1 = abs(n.x) + abs(n.y) + abs(n.z);
    vec2 uv = n.xy / l1;
    
    if (n.z < 0.0) {
        float x = uv.x;
        float y = uv.y;
        uv.x = (1.0 - abs(y)) * (x >= 0.0 ? 1.0 : -1.0);
        uv.y = (1.0 - abs(x)) * (y >= 0.0 ? 1.0 : -1.0);
    }

    return uv * 0.5 + 0.5;
}

void main()
{
    gPosition = vPosition;
    gNormal = NormalEncode(vNormal);
    gAlbedo = texture(uAlbedoMap, vTexCoords).rgb;
    gMaterial = vec4(1.0);
};