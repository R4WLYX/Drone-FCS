#version 330 core
layout(location = 0) in vec3 positionVert;
layout(location = 1) in vec3 textureVert;
layout(location = 2) in vec3 normalVert;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;
uniform float outlining;

void main()
{
    gl_Position = vec4(positionVert, 1.0) * transpose(model) * transpose(view) * proj * outlining * 10.0;
}