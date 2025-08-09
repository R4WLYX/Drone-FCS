#version 330 core

layout(location = 0) out vec4 fragColor;

uniform vec3 objectColor;

void main()
{
    fragColor = vec4(objectColor, 1.0);
}