#pragma once

#include "mesh.hpp"
#include <vector>
#include <cmath>

class Sphere : public Mesh {
public:
    Sphere(float radius, unsigned int sectors = 16, unsigned int stacks = 16, const glm::vec4& _color = glm::vec4(1.0f))
         : Mesh(nullptr) {
        std::vector<float> verts;
        std::vector<unsigned int> inds;

        const float PI = 3.14159265359f;

        for (unsigned int i = 0; i <= stacks; ++i) {
            float stackAngle = PI / 2 - (float)i * (PI / stacks);
            float xy = radius * cosf(stackAngle);
            float z = radius * sinf(stackAngle);

            for (unsigned int j = 0; j <= sectors; ++j) {
                float sectorAngle = (float)j * (2 * PI / sectors);

                float x = xy * cosf(sectorAngle);
                float y = xy * sinf(sectorAngle);

                glm::vec3 pos(x, y, z);
                glm::vec3 normal = glm::normalize(pos);

                verts.insert(verts.end(), {
                    pos.x, pos.y, pos.z,
                    0.0f, 0.0f,
                    normal.x, normal.y, normal.z
                });
            }
        }

        for (unsigned int i = 0; i < stacks; ++i) {
            unsigned int k1 = i * (sectors + 1);
            unsigned int k2 = k1 + sectors + 1;

            for (unsigned int j = 0; j < sectors; ++j, ++k1, ++k2) {
                if (i != 0) {
                    inds.push_back(k1);
                    inds.push_back(k2);
                    inds.push_back(k1 + 1);
                }

                if (i != (stacks - 1)) {
                    inds.push_back(k1 + 1);
                    inds.push_back(k2);
                    inds.push_back(k2 + 1);
                }
            }
        }

        loadMeshData(verts.data(), verts.size(), inds.data(), inds.size());
        generateBuffers();
        centerOrigin();
        color = _color;
    }
};