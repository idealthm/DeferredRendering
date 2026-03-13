#include "../Common/Structures.glsl"
 
// 顶点属性（根据你的 VAO 设置调整 location）
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoords;
layout (location = 2) in vec3 aNormal;
layout (location = 3) in vec3 aTangent;   // 如果要做法线贴图需要这个

// 传给片元着色器的数据
out vec3 WorldPos;
out vec2 TexCoords;
out vec3 Normal;
// out mat3 TBN; // 如果以后要做切线空间法线贴图，在这里计算 TBN 矩阵

uniform mat4 model;
uniform mat3 normalMatrix; // 通常是 transpose(inverse(model))，用于处理非统一缩放下的法线

void main()
{
    // 1. 计算世界空间坐标
    vec4 worldPos4 = model * vec4(aPos, 1.0);
    WorldPos = worldPos4.xyz;
    
    // 2. 传递纹理坐标
    TexCoords = aTexCoords;
    
    // 3. 计算并传递世界空间法线
    // 注意：不要直接用 model 矩阵乘以法线，除非 model 矩阵只有平移和旋转
    Normal = normalize(normalMatrix * aNormal);
    
    // 4. 计算最终裁剪空间位置
    gl_Position = projection * view * worldPos4;
}