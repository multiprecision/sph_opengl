#version 450

layout (location = 0) in vec2 position;

out gl_PerVertex
{
    vec4 gl_Position;
    float gl_PointSize;
};

void main ()
{
    gl_Position = vec4(position.x, position.y, 0, 1);
    gl_PointSize = 3;
}
    