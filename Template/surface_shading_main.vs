layout(location = 0) in vec4 a_position;

LAYOUT_LOCATION(0) out vec2 variable_uv;

void main()
{
    variable_uv = a_position.xy * 0.5 + 0.5;
    gl_Position = vec4(a_position.xy, 1.0, 1.0);
}
