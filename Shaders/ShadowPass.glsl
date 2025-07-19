#shader vertex
#version 330 core

layout(location = 0) in vec3 aPosition;

uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProjection;

void main()
{
    gl_Position = uProjection * uView * uModel * vec4(aPosition, 1.0);
};

#shader fragment
#version 330 core

// layout(location = 0) out vec4 color;

void main()
{
    // color = vec4(1.0, 0.0, 0.0, 1.0); 
};