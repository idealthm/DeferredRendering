#include <PBR/BRDF.glsl>

out vec4 FragColor;
in vec3 vTexCoords;

uniform samplerCube uCubeMap;
uniform float uRoughness; // 当前 Mip 层级对应的粗糙度 [0.0, 1.0]

// ----------------------------------------------------------------------------
// 1. 低差异序列生成 (Van der Corput sequence)
// 用于生成均匀分布在 [0, 1] 之间的准随机数
// ----------------------------------------------------------------------------
float RadicalInverse_VdC(uint bits) 
{
    bits = (bits << 16u) | (bits >> 16u);
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
    return float(bits) * 2.3283064365386963e-10; // / 0x100000000
}

// ----------------------------------------------------------------------------
// 2. 生成 Hammersley 采样点
// 结合 RadicalInverse，在二维平面上生成分布均匀的采样点 [u, v]
// ----------------------------------------------------------------------------
vec2 Hammersley(uint i, uint N)
{
    return vec2(float(i)/float(N), RadicalInverse_VdC(i));
}

// ----------------------------------------------------------------------------
// 3. GGX 重要性采样 (Importance Sampling)
// 根据指定的粗糙度和 D 项 (NDF)，将 Hammersley 点映射到半球空间上的 H 向量
// 这使得我们在反射光最强的区域采集更多的样本，极大提高收敛速度
// ----------------------------------------------------------------------------
vec3 ImportanceSampleGGX(vec2 Xi, vec3 N, float roughness)
{
    float a = roughness * roughness;
    
    // 1. 在切线空间（Tangent Space）构建微观法线 H
    float phi = 2.0 * PI * Xi.x;
    float cosTheta = sqrt((1.0 - Xi.y) / (1.0 + (a * a - 1.0) * Xi.y));
    float sinTheta = sqrt(1.0 - cosTheta * cosTheta);

    vec3 H;
    H.x = cos(phi) * sinTheta;
    H.y = sin(phi) * sinTheta;
    H.z = cosTheta; // 在切线空间中，Z轴即为宏观法线方向
    
    // 从切线空间转换到世界空间
    vec3 up        = abs(N.y) < 0.999 ? vec3(0.0, 1.0, 0.0) : vec3(0.0, 0.0, 1.0);
    vec3 tangent   = normalize(cross(up, N));
    vec3 bitangent = cross(N, tangent);
    
    vec3 sampleVec = tangent * H.x + bitangent * H.y + N * H.z;
    return normalize(sampleVec);
}

// ----------------------------------------------------------------------------
// 4. 主函数
// ----------------------------------------------------------------------------
void main()
{		
    vec3 N = normalize(vTexCoords);
    
    // 简化假设：视角 V 等于法线 N。
    // 这在 Epic 的 Split-Sum 近似中是一个非常有效的简化，使得预滤波只取决于 N 和 Roughness
    vec3 V = N;

    const uint SAMPLE_COUNT = 1; // 采样数。开发时可用 1024，发布时可设为 2048 或 4096 以获得更高质量
    vec3 prefilteredColor = vec3(0.0);
    float totalWeight = 0.0;
    
    for(uint i = 0u; i < SAMPLE_COUNT; ++i)
    {
        // 生成 Hammersley 采样点 [u, v]
        vec2 Xi = Hammersley(i, SAMPLE_COUNT);
        // 根据 GGX 分布生成微平面法线 H
        vec3 H  = ImportanceSampleGGX(Xi, N, uRoughness);
        // 根据 V 和 H 计算出入射光方向 L (理想镜面反射方向)
        vec3 L  = normalize(2.0 * dot(V, H) * H - V);

        float NdotL = max(dot(N, L), 0.0);
        if(NdotL > 0.0)
        {
            // sample from the environment's mip level based on roughness/pdf
            float D   = D_GGX(N, H, uRoughness);
            float NdotH = max(dot(N, H), 0.0);
            float HdotV = max(dot(H, V), 0.0);
            float pdf = D * NdotH / (4.0 * HdotV) + 0.0001; 

            float resolution = 512.0; // resolution of source cubemap (per face)
            float saTexel  = 4.0 * PI / (6.0 * resolution * resolution);
            float saSample = 1.0 / (float(SAMPLE_COUNT) * pdf + 0.0001);

            float mipLevel = uRoughness == 0.0 ? 0.0 : 0.5 * log2(saSample / saTexel); 

            // 在这一步，Epic 的实现引入了一个优化：
            // 利用原始 Cubemap 的 Mipmap 来减少高粗糙度下的采样噪点 (根据 PDF 选择 Mip Level)
            // 简化版：直接采样 Mip 0
            prefilteredColor += textureLod(uCubeMap, L, mipLevel).rgb * NdotL;

            totalWeight += NdotL;
        }
    }
    
    // 归一化
    prefilteredColor = prefilteredColor / totalWeight;

    FragColor = vec4(prefilteredColor, 1.0);
}