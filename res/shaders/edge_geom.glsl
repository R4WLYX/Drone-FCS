#version 330 core

layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

out vec3 v_normal;
out vec3 v_fragPos;

void main() {
    // Get face normal in view space
    vec3 p0 = vec3(view * model * gl_in[0].gl_Position);
    vec3 p1 = vec3(view * model * gl_in[1].gl_Position);
    vec3 p2 = vec3(view * model * gl_in[2].gl_Position);
    
    vec3 normal = normalize(cross(p1 - p0, p2 - p0));
    vec3 viewDir = normalize((p0 + p1 + p2) / 3.0);

    // Edge visibility condition: face nearly perpendicular to view dir (silhouette edge)
    float ndotv = dot(normal, viewDir);
    float threshold = 0.2; // tweak for thicker/thinner edge visibility

    if (abs(ndotv) < threshold) {
        for (int i = 0; i < 3; ++i) {
            gl_Position = gl_in[i].gl_Position;
            v_fragPos = vec3(model * gl_in[i].gl_Position);
            v_normal = normal;
            EmitVertex();
        }
        EndPrimitive();
    }
}