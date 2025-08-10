#pragma once

#include "mesh.hpp"

class Box : public Mesh {
public:
    glm::vec3 min;
    glm::vec3 max;

    Box(const glm::vec3& min, const glm::vec3& max, const glm::vec4& _color = glm::vec4(1.0f))
        : min(min), max(max), Mesh(nullptr)
    {
        float vertices[] {
            min.x, min.y, max.z, 0.0f, 0.0f,  0.0f,  0.0f,  1.0f,
            max.x, min.y, max.z, 0.0f, 0.0f,  0.0f,  0.0f,  1.0f,
            min.x, max.y, max.z, 0.0f, 0.0f,  0.0f,  0.0f,  1.0f,
            max.x, max.y, max.z, 0.0f, 0.0f,  0.0f,  0.0f,  1.0f,
            min.x, min.y, min.z, 0.0f, 0.0f,  0.0f,  0.0f, -1.0f,
            max.x, min.y, min.z, 0.0f, 0.0f,  0.0f,  0.0f, -1.0f,
            min.x, max.y, min.z, 0.0f, 0.0f,  0.0f,  0.0f, -1.0f,
            max.x, max.y, min.z, 0.0f, 0.0f,  0.0f,  0.0f, -1.0f,
            min.x, max.y, min.z, 0.0f, 0.0f,  0.0f,  1.0f,  0.0f,
            max.x, max.y, min.z, 0.0f, 0.0f,  0.0f,  1.0f,  0.0f,
            min.x, max.y, max.z, 0.0f, 0.0f,  0.0f,  1.0f,  0.0f,
            max.x, max.y, max.z, 0.0f, 0.0f,  0.0f,  1.0f,  0.0f,
            min.x, min.y, min.z, 0.0f, 0.0f,  0.0f, -1.0f,  0.0f,
            max.x, min.y, min.z, 0.0f, 0.0f,  0.0f, -1.0f,  0.0f,
            min.x, min.y, max.z, 0.0f, 0.0f,  0.0f, -1.0f,  0.0f,
            max.x, min.y, max.z, 0.0f, 0.0f,  0.0f, -1.0f,  0.0f,
            max.x, min.y, min.z, 0.0f, 0.0f,  1.0f,  0.0f,  0.0f,
            max.x, max.y, min.z, 0.0f, 0.0f,  1.0f,  0.0f,  0.0f,
            max.x, min.y, max.z, 0.0f, 0.0f,  1.0f,  0.0f,  0.0f,
            max.x, max.y, max.z, 0.0f, 0.0f,  1.0f,  0.0f,  0.0f,
            min.x, min.y, min.z, 0.0f, 0.0f, -1.0f,  0.0f,  0.0f,
            min.x, max.y, min.z, 0.0f, 0.0f, -1.0f,  0.0f,  0.0f,
            min.x, min.y, max.z, 0.0f, 0.0f, -1.0f,  0.0f,  0.0f,
            min.x, max.y, max.z, 0.0f, 0.0f, -1.0f,  0.0f,  0.0f
        };

        unsigned int indices[] {
             0,  1,  2,  1,  2,  3,
             4,  5,  6,  5,  6,  7,
             8,  9, 10,  9, 10, 11,
            12, 13, 14, 13, 14, 15,
            16, 17, 18, 17, 18, 19,
            20, 21, 22, 21, 22, 23
        };

        loadMeshData(vertices, 192, indices, 36);
        generateBuffers();
        centerOrigin();
        color = _color;
    }
};