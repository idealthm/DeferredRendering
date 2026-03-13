layout (location = 0) out vec4 FragColor;
in vec2 vTexCoords;

uniform sampler2D uHdrSceneColor; // 之前 Lighting Pass 算出来的 HDR 纹理

// 简单的 ACES 拟合
vec3 ACESFilm(vec3 x) {
    float a = 2.51;
    float b = 0.03;
    float c = 2.43;
    float d = 0.59;
    float e = 0.14;
    return clamp((x * (a * x + b)) / (x * (c * x + d) + e), 0.0, 1.0);
}

void main() {
    vec3 hdrColor = texture(uHdrSceneColor, vTexCoords).rgb;

    // 1. Tone Mapping：将 [0, inf] 映射到 [0, 1]
    vec3 ldrColor = ACESFilm(hdrColor);

    // 2. Gamma Correction：输出到显示器前的“装箱”
    // 抵消显示器的 Gamma 2.2 物理特性
    vec3 finalColor = pow(ldrColor, vec3(1.0 / 2.2));

    FragColor = vec4(finalColor, 1.0);
}