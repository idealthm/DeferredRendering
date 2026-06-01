
struct ShadowData {
    float normalBias;
    mat4 lightFromWorldMatrix;
};

struct PerRenderableData {
    mat4 worldFromModelMatrix;
    mat3 worldFromModelNormalMatrix;
    int morphTargetCount;
    int flagsChannels;                   // see packFlags() below (0x00000fll)
    int objectId;                        // used for picking
    float userData;

    vec4 reserved[8];
};