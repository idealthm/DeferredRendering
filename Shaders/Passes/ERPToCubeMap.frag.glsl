
in vec3 vTexCoords;

uniform sampler2D uHDRMap;


layout(location = 0) out vec4 FragColor;

vec2 SampleSphericalMap(vec3 v) {
    vec2 uv = vec2(atan(v.z, v.x), asin(v.y));
    uv *= vec2(0.1591, 0.3183); // 归一化到 [0,1]
    uv += 0.5;
    return uv;
}

void main() {
    vec3 envVector = normalize(vTexCoords); // 立方体顶点的插值位置
    vec2 uv = SampleSphericalMap(envVector);
    vec3 color = texture(uHDRMap, uv).rgb;
    FragColor = vec4(color, 1.0);
}