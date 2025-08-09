#pragma once

#include "mesh.hpp"

class Box : public Mesh {
public:
    Box(const glm::vec3& min, const glm::vec3& max, const glm::vec4& _color = glm::vec4(1.0f))
      : Mesh(nullptr) {
        glm::vec3 corners[8] = {
            {min.x, min.y, min.z},
            {max.x, min.y, min.z},
            {min.x, max.y, min.z},
            {max.x, max.y, min.z},
            {min.x, min.y, max.z},
            {max.x, min.y, max.z},
            {min.x, max.y, max.z},
            {max.x, max.y, max.z}
        };

        struct Face {
            glm::vec3 normal;
            unsigned int idx[4];
        };

        Face faces[6] = {
            {{ 0,  0,  1}, {4, 5, 6, 7}},
            {{ 0,  0, -1}, {0, 1, 2, 3}},
            {{ 0,  1,  0}, {2, 3, 6, 7}},
            {{ 0, -1,  0}, {0, 1, 4, 5}},
            {{ 1,  0,  0}, {1, 3, 5, 7}},
            {{-1,  0,  0}, {0, 2, 4, 6}}
        };

        std::vector<float> verts;
        std::vector<unsigned int> inds;

        for (int f = 0; f < 6; ++f) {
            unsigned int baseIndex = verts.size() / 8;

            for (int v = 0; v < 4; ++v) {
                glm::vec3 pos = corners[faces[f].idx[v]];
                glm::vec3 normal = faces[f].normal;
                verts.insert(verts.end(), {
                    pos.x, pos.y, pos.z,
                    0.0f, 0.0f,
                    normal.x, normal.y, normal.z
                });
            }

            inds.insert(inds.end(), {
                baseIndex, baseIndex + 1, baseIndex + 2,
                baseIndex + 1, baseIndex + 2, baseIndex + 3
            });
        }

        loadMeshData(verts.data(), verts.size(), inds.data(), inds.size());
        generateBuffers();
        centerOrigin();
        color = _color;
    }
};