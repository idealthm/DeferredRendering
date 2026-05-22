in vec3 vPosition;
in vec2 vTexCoords;
in mat3 vTBN;

layout(location = 0) out vec3 gPosition;
layout(location = 1) out vec2 gNormal;
layout(location = 2) out vec3 gAlbedo;
layout(location = 3) out vec4 gMaterial; // metallic & roughness & AO

uniform sampler2D uAlbedo;
uniform sampler2D uNormal;
uniform sampler2D uRoughness;
uniform sampler2D uMetallic;
uniform sampler2D uAO;

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
    vec3 normalSample = texture(uNormal, vTexCoords).rgb;
    vec3 tangentNormal = normalSample * 2.0 - 1.0;
    vec3 worldNormal = normalize(vTBN * tangentNormal);
    gNormal = NormalEncode(worldNormal);
    gAlbedo = texture(uAlbedo, vTexCoords).rgb;
    // gAlbedo = vec3(1.0, 0.0, 0.0);
    // gAlbedo = vec3(vTexCoords, 0.0);

    float roughness = texture(uRoughness, vTexCoords).r;
    float metallic = texture(uMetallic, vTexCoords).r;
    float ao = texture(uAO, vTexCoords).r;
    gMaterial = vec4(roughness, metallic, ao, 1.0);
}