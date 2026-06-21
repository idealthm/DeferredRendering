in vec3 vPosition;
in vec2 vTexCoords;
in mat3 vTBN;
in vec2 vUv;

layout(location = 0) out vec3 gPosition;
layout(location = 1) out vec2 gNormal;
layout(location = 2) out vec3 gAlbedo;
layout(location = 3) out vec4 gMaterial;

// MaterialParams — matches Lit.mat property layout
// baseColor(float3), roughness(float), textureTransform(mat4)
layout(std140, binding = 5) uniform MaterialParams
{
    vec3 baseColor;
    float roughness;
    mat4 textureTransform;
} materialParams;

layout(binding = 6) uniform sampler2D materialParams_texture;

vec2 NormalEncode(vec3 n)
{
    n = normalize(n);
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

float srgbToLinear(float c)
{
    return c <= 0.04045 ? c / 12.92 : pow((c + 0.055) / 1.055, 2.4);
}

vec3 inverseTonemapSRGB(vec3 color)
{
    return vec3(srgbToLinear(color.r), srgbToLinear(color.g), srgbToLinear(color.b));
}

void main()
{
    gPosition = vPosition;
    vec3 worldNormal = normalize(vTBN[2]);
    gNormal = NormalEncode(worldNormal);

    // Lit.mat material logic
    vec2 uv = (materialParams.textureTransform * vec4(vUv, 0.0, 1.0)).xy;
    vec3 color = inverseTonemapSRGB(texture(materialParams_texture, uv).rgb);

    gAlbedo = color;
    float matRoughness = materialParams.roughness;
    gMaterial = vec4(matRoughness, 0.0, 1.0, 1.0);
}
