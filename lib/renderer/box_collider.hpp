#pragma once

#include "mesh.hpp"
#include "box.hpp"
#include <memory>

class BoxCollider {
public:
    glm::vec3 min;
    glm::vec3 max;

    explicit BoxCollider(const Mesh& mesh)
        : min(0.0f), max(0.0f) {
        updateBounds(mesh);
    }

    BoxCollider(const glm::vec3& _min, const glm::vec3& _max)
        : min(_min), max(_max) {
        updateBounds(_min, _max);
    }

    void updateBounds(const Mesh& mesh) {
        auto bounds = mesh.getBounds();
        auto transformed = transformAABB(bounds.first, bounds.second, mesh.model);

        if (!boundingBox) {
            boundingBox = std::make_unique<Box>(
                transformed.first,
                transformed.second,
                glm::vec4(glm::vec3(mesh.color), 0.5f)
            );
            boundingBox->scaleBy(1.01f);
        } else {
            glm::vec3 scale = (transformed.second - transformed.first) / (max - min);
            boundingBox->scaleBy(scale);
        }

        min = transformed.first;
        max = transformed.second;
        boundingBox->setPosition(mesh.position);
    }

    void updateBounds(const glm::vec3& _min, const glm::vec3& _max) {
        if (!boundingBox) {
            boundingBox = std::make_unique<Box>(min, max, glm::vec4(glm::vec3(1.0f), 0.5f));
            boundingBox->scaleBy(1.01f);
        } else {
            glm::vec3 scale = (_max - _min) / (max - min);
            boundingBox->scaleBy(scale);
        }
        min = _min;
        max = _max;
    }

    bool intersects(const Mesh& mesh) const {
        if (mesh.vertices.empty() || mesh.indices.empty()) return false;

        for (size_t i = 0; i < mesh.indices.size(); i += 3) {
            glm::vec3 v0 = mesh.getTransformedVertex(mesh.indices[i]);
            glm::vec3 v1 = mesh.getTransformedVertex(mesh.indices[i + 1]);
            glm::vec3 v2 = mesh.getTransformedVertex(mesh.indices[i + 2]);

            if (triangleIntersectsAABB(v0, v1, v2, min, max)) {
                return true;
            }
        }
        return false;
    }

    bool intersects(const BoxCollider& other) const {
        return !(max.x < other.min.x || min.x > other.max.x ||
                 max.y < other.min.y || min.y > other.max.y ||
                 max.z < other.min.z || min.z > other.max.z);
    }

    void render(Shader shader) {
        boundingBox->render(shader, GL_LINE);
    }

private:
    std::unique_ptr<Box> boundingBox;

    std::pair<glm::vec3, glm::vec3> transformAABB(
        const glm::vec3& min,
        const glm::vec3& max,
        const glm::mat4& model
    ) {
        glm::vec3 corners[8] = {
            {min.x, min.y, min.z}, {min.x, min.y, max.z},
            {min.x, max.y, min.z}, {min.x, max.y, max.z},
            {max.x, min.y, min.z}, {max.x, min.y, max.z},
            {max.x, max.y, min.z}, {max.x, max.y, max.z}
        };

        glm::vec3 newMin = glm::vec3(model * glm::vec4(corners[0], 1.0f));
        glm::vec3 newMax = newMin;

        for (int i = 1; i < 8; ++i) {
            glm::vec3 transformed = glm::vec3(model * glm::vec4(corners[i], 1.0f));
            newMin = glm::min(newMin, transformed);
            newMax = glm::max(newMax, transformed);
        }

        return { newMin, newMax };
    }

    bool triangleIntersectsAABB(
        const glm::vec3& v0,
        const glm::vec3& v1,
        const glm::vec3& v2,
        const glm::vec3& aabbMin,
        const glm::vec3& aabbMax
    ) const {
        glm::vec3 triMin = glm::min(glm::min(v0, v1), v2);
        glm::vec3 triMax = glm::max(glm::max(v0, v1), v2);

        return !(triMax.x < aabbMin.x || triMin.x > aabbMax.x ||
                 triMax.y < aabbMin.y || triMin.y > aabbMax.y ||
                 triMax.z < aabbMin.z || triMin.z > aabbMax.z);
    }
};