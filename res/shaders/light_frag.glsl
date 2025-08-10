#version 330 core

layout(location = 0) out vec4 fragColor;

uniform vec4 objectColor;
uniform vec3 lightColor;
uniform vec3 lightPos;
uniform vec3 viewPos;

in vec3 v_normal;
in vec3 v_fragPos;

void main()
{
    float ambientStrength = 0.3;
    vec3 ambient = ambientStrength * lightColor;

    vec3 norm = normalize(v_normal);
    vec3 lightDir = normalize(lightPos - v_fragPos);

    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    float specularStrength = 0.6;
    vec3 viewDir = normalize(viewPos - v_fragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 64);
    vec3 specular = specularStrength * spec * lightColor;

    vec3 result = (ambient + diffuse) * objectColor;
    fragColor = vec4(result, objectColor.a);
}