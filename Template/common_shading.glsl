
highp mat3  shading_tangentToWorld;   // TBN matrix
highp vec3  shading_position;         // position of the fragment in world space
vec3  shading_view;             // normalized vector from the fragment to the eye
vec3  shading_normal;           // normalized transformed normal, in world space
vec3  shading_geometricNormal;  // normalized geometric normal, in world space
vec3  shading_reflected;        // reflection of view about normal
float shading_NoV;              // dot(normal, view), always strictly >= MIN_N_DOT_V

vec3  shading_bentNormal;       // normalized transformed normal, in world space

highp vec2 shading_normalizedViewportCoord;