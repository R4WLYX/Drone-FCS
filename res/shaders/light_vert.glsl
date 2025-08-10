#version 330 core

layout(location = 0) in vec3 positionVert;
layout(location = 1) in vec3 textureVert;
layout(location = 2) in vec3 normalVert;

uniform mat4 models[128];
uniform mat4 view;
uniform mat4 proj;

out vec3 v_normal;
out vec3 v_fragPos;

void main()
{
    mat4 model = models[gl_InstanceID];
    gl_Position = vec4(positionVert, 1.0) * transpose(model) * transpose(view) * proj;
    v_fragPos = vec3(vec4(positionVert + vec3(0.0, 0.0, 50.0), 1.0) * transpose(model));
    v_normal = normalVert * mat3(inverse(model));
}