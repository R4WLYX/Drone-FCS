#pragma once

#include "sphere.hpp"
#include <memory>
#include <glm/glm.hpp>

class SphereCollider {
public:
    glm::vec3 center;
    float radius;

    explicit SphereCollider(const Mesh& mesh)
        : center(0.0f), radius(1.0f) {
        updateBounds(mesh);
    }

    SphereCollider(const glm::vec3& _center, float _radius)
        : center(_center), radius(_radius) {
        createSphereMesh();
    }

    void updateBounds(const Mesh& mesh) {
        auto bounds = mesh.getBounds();
        glm::vec3 min = bounds.first;
        glm::vec3 max = bounds.second;

        glm::vec3 transformedMin = glm::vec3(mesh.model * glm::vec4(min, 1.0f));
        glm::vec3 transformedMax = glm::vec3(mesh.model * glm::vec4(max, 1.0f));

        center = (transformedMin + transformedMax) * 0.5f;
        radius = glm::length((transformedMax - transformedMin) * 0.5f);

        if (!boundingSphere) {
            createSphereMesh();
        } else {
            boundingSphere->setPosition(center);
        }
    }

    bool intersects(const SphereCollider& other) const {
        float dist = glm::length(center - other.center);
        return dist <= (radius + other.radius);
    }

    bool intersects(const Mesh& mesh) const {
        if (mesh.vertices.empty() || mesh.indices.empty()) return false;

        for (size_t i = 0; i < mesh.indices.size(); i += 3) {
            glm::vec3 v0 = mesh.getTransformedVertex(mesh.indices[i]);
            glm::vec3 v1 = mesh.getTransformedVertex(mesh.indices[i + 1]);
            glm::vec3 v2 = mesh.getTransformedVertex(mesh.indices[i + 2]);

            if (triangleIntersectsSphere(v0, v1, v2, center, radius)) {
                return true;
            }
        }
        return false;
    }

    void render(Shader shader) {
        if (boundingSphere)
            boundingSphere->render(shader, GL_LINE);
    }

private:
    std::unique_ptr<Sphere> boundingSphere;

    void createSphereMesh() {
        boundingSphere = std::make_unique<Sphere>(radius, 32, 32, glm::vec4(glm::vec3(1.0f), 0.5f));
        boundingSphere->setPosition(center);
    }

    bool triangleIntersectsSphere(
        const glm::vec3& v0,
        const glm::vec3& v1,
        const glm::vec3& v2,
        const glm::vec3& sphereCenter,
        float sphereRadius
    ) const {
        auto closestPointOnTri = [](const glm::vec3& p, const glm::vec3& a,
                                    const glm::vec3& b, const glm::vec3& c) {
            glm::vec3 ab = b - a;
            glm::vec3 ac = c - a;
            glm::vec3 ap = p - a;

            float d1 = glm::dot(ab, ap);
            float d2 = glm::dot(ac, ap);
            if (d1 <= 0.0f && d2 <= 0.0f) return a;

            glm::vec3 bp = p - b;
            float d3 = glm::dot(ab, bp);
            float d4 = glm::dot(ac, bp);
            if (d3 >= 0.0f && d4 <= d3) return b;

            float vc = d1 * d4 - d3 * d2;
            if (vc <= 0.0f && d1 >= 0.0f && d3 <= 0.0f) {
                float v = d1 / (d1 - d3);
                return a + v * ab;
            }

            glm::vec3 cp = p - c;
            float d5 = glm::dot(ab, cp);
            float d6 = glm::dot(ac, cp);
            if (d6 >= 0.0f && d5 <= d6) return c;

            float vb = d5 * d2 - d1 * d6;
            if (vb <= 0.0f && d2 >= 0.0f && d6 <= 0.0f) {
                float w = d2 / (d2 - d6);
                return a + w * ac;
            }

            float va = d3 * d6 - d5 * d4;
            if (va <= 0.0f && (d4 - d3) >= 0.0f && (d5 - d6) >= 0.0f) {
                float w = (d4 - d3) / ((d4 - d3) + (d5 - d6));
                return b + w * (c - b);
            }

            float denom = 1.0f / (va + vb + vc);
            float v = vb * denom;
            float w = vc * denom;
            return a + ab * v + ac * w;
        };

        glm::vec3 p = closestPointOnTri(sphereCenter, v0, v1, v2);
        return glm::length(p - sphereCenter) <= sphereRadius;
    }
};