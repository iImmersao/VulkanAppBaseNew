#version 450


// Array for triangle that fills screen, forcing fragment shader to process every point
vec2 positions[3] = vec2[] (
    vec2(3.0, -1.0),
    vec2(-1.0, -1.0),
    vec2(-1.0, 3.0)
);

void main()
{
    gl_Position = vec4(positions[gl_VertexIndex], 0.0, 1.0);
}