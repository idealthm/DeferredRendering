#include "Common/Structures.glsl"

layout(location = 0) in vec4 aPosition;

uniform mat4 uModel;

void main()
{
    gl_Position = uLightVP * uModel * vec4(aPosition.xyz, 1.0);
}
