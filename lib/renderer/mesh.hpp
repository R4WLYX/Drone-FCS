#pragma once

#include <GL/glew.h>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <fstream>
#include <sstream>
#include <vector>
#include <iostream>
#include <algorithm>

#include "shader.hpp"

#define VERTEX_WIDTH 8

#ifndef INSTANCE_COUNT
    #define INSTANCE_COUNT 128
#endif

class Mesh {
public:
    std::vector<float> vertices;
    std::vector<unsigned int> indices;

    glm::vec3 position;
    glm::quat rotation;
    glm::vec3 scale;
    glm::vec4 color;

    glm::vec3 origin;
    glm::vec3 front;
    glm::vec3 right;
    glm::vec3 up;

    glm::mat4 model;

    Mesh(float _vertices[], unsigned int _vertexCount, unsigned int _indices[], unsigned int _indexCount)
        : position(glm::vec3(0.0f)), rotation(glm::vec3(0.0f)), scale(glm::vec3(1.0f)), color(glm::vec4(1.0f)), cachedModelCount(0)
    {
        loadMeshData(_vertices, _vertexCount, _indices, _indexCount);
        centerOrigin();
        generateBuffers();
        updateDirectionVectors();
    }

    Mesh(const char *model_file, const glm::vec3& _position, const glm::vec3& _rotation,
            const glm::vec3& _scale, const glm::vec4& _color)
        : position(_position), rotation(_rotation), scale(_scale), color(_color), cachedModelCount(0)
    {
        parseOBJ(model_file);
        centerOrigin();
        generateBuffers();
        updateDirectionVectors();
    }

    Mesh(const char *model_file)
        : Mesh(model_file, glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(1.0f), glm::vec4(1.0f)) {}

    Mesh(const char *model_file, const glm::vec3& _position)
        : Mesh(model_file, _position, glm::vec3(0.0f), glm::vec3(1.0f), glm::vec4(1.0f)) {}

    Mesh(const char *model_file, const glm::vec3& _position, const glm::vec3& _rotation)
        : Mesh(model_file, _position, _rotation, glm::vec3(1.0f), glm::vec4(1.0f)) {}

    Mesh(const char *model_file, const glm::vec3& _position, float _scale)
        : Mesh(model_file, _position, glm::vec3(0.0f), glm::vec3(_scale), glm::vec4(1.0f)) {}

    Mesh(const char *model_file, const glm::vec3& _position, const glm::vec3& _rotation, float _scale)
        : Mesh(model_file, _position, _rotation, glm::vec3(_scale), glm::vec4(1.0f)) {}

    Mesh(const char *model_file, glm::vec3 _position, const glm::vec3& _scale)
        : Mesh(model_file, _position, glm::vec3(0.0f), _scale, glm::vec4(1.0f)) {}

    Mesh(const char *model_file, const glm::vec3& _position, const glm::vec3& _rotation, const glm::vec4& _scale)
        : Mesh(model_file, _position, _rotation, _scale, glm::vec4(1.0f)) {}

    ~Mesh() {
        glDeleteVertexArrays(1, &vao);
        glDeleteBuffers(1, &vbo);
        glDeleteBuffers(1, &ibo);
    }

    void translate(const glm::vec3& delta) {
        position += delta;
        updateModelMatrix();
    }

    void setPosition(const glm::vec3& newPos) {
        position = newPos;
        updateModelMatrix();
    }

void rotate(const glm::vec3& eulerDelta) {
    rotation = glm::quat(eulerDelta) * rotation;
    updateModelMatrix();
    updateDirectionVectors();
}

void rotate(const glm::quat& delta) {
    rotation = delta * rotation;
    updateModelMatrix();
    updateDirectionVectors();
}

void rotateAround(glm::vec3 worldPivot, glm::vec3 eulerAngles) {
    glm::quat deltaRot = glm::quat(eulerAngles);
    
    glm::vec3 offset = position - worldPivot;
    offset = deltaRot * offset;
    position = worldPivot + offset;
    
    rotation = deltaRot * rotation;

    updateModelMatrix();
    updateDirectionVectors();
}


void rotateAround(glm::vec3 worldPivot, glm::quat deltaRot) {
    glm::vec3 offset = position - worldPivot;
    offset = deltaRot * offset;
    position = worldPivot + offset;

    rotation = deltaRot * rotation;

    updateModelMatrix();
    updateDirectionVectors();
}

void setRotation(const glm::vec3& eulerAngles) {
    rotation = glm::quat(eulerAngles);
    updateModelMatrix();
    updateDirectionVectors();
}

void setRotation(const glm::quat& newRotation) {
    rotation = newRotation;
    updateModelMatrix();
    updateDirectionVectors();
}

    void scaleBy(const glm::vec3& scaleFactor) {
        scale *= scaleFactor;
        updateModelMatrix();
    }

    void scaleBy(float factor) {
        scale *= glm::vec3(factor);
        updateModelMatrix();
    }

    void setScale(const glm::vec3& newScale) {
        scale = newScale;
        updateModelMatrix();
    }

    void setScale(float uniformScale) {
        scale = glm::vec3(uniformScale);
        updateModelMatrix();
    }

    void setOrigin(const glm::vec3& newOrigin) {
        origin = newOrigin;
        updateModelMatrix();
    }

    void setColor(const glm::vec3& newColor) {
        color = glm::vec4(newColor, 1.0f);
    }

    void setColor(const glm::vec4& newColor) {
        color = newColor;
    }

    void centerOrigin() {
        auto [min, max] = getBounds();
        origin = (min + max) * 0.5f;
        updateModelMatrix();
    }

    void flipNormals() {
        for (int i = 0; i < vertices.size(); i += VERTEX_WIDTH) {
            vertices[i + 5] = -vertices[i + 5];
            vertices[i + 6] = -vertices[i + 6];
            vertices[i + 7] = -vertices[i + 7];
        }

        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(float), vertices.data());
    }

    std::pair<glm::vec3, glm::vec3> getBounds() const {
        if (!vertices.size()) return {glm::vec3(0.0f), glm::vec3(0.0f)};

        glm::vec3 minPos(vertices[0], vertices[1], vertices[2]);
        glm::vec3 maxPos = minPos;

        for (int i = 0; i < vertices.size(); i += VERTEX_WIDTH) {
            glm::vec3 pos(vertices[i], vertices[i + 1], vertices[i + 2]);

            minPos = glm::min(minPos, pos);
            maxPos = glm::max(maxPos, pos);
        }

        return {minPos, maxPos};
    }

    glm::vec3 getTransformedVertex(unsigned int index) const {
        unsigned int vi = index * VERTEX_WIDTH;
        glm::vec3 localPos(
            vertices[vi+0],
            vertices[vi+1],
            vertices[vi+2]
        );
        return glm::vec3(model * glm::vec4(localPos, 1.0f));
    }

    void render(Shader shader, int mode = GL_FILL) {
        glPolygonMode(GL_FRONT_AND_BACK, mode);
        shader.bind();
        shader.setUniform4f("objectColor", color);
        shader.setUniformMat4f("models[0]", model);
        glBindVertexArray(vao);
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, nullptr);
        glBindVertexArray(0);
    }

    void renderInstanced(Shader& shader, const std::vector<glm::mat4>& models, int mode = GL_FILL) {
        if (models.size() != cachedModelCount) {
            setInstanceModels(models);
            cacheUniformLocations(shader);
            cachedModelCount = models.size();
        }

        glPolygonMode(GL_FRONT_AND_BACK, mode);
        shader.bind();
        glBindVertexArray(vao);
        shader.setUniform4f("objectColor", color);
        for (const auto& chunkModels : splitModels) {
            shader.setUniformMat4fv("models", &chunkModels[0], chunkModels.size());
            glDrawElementsInstanced(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, nullptr, chunkModels.size());
        }
        glBindVertexArray(0);
    }

protected:
    unsigned int vao, vbo, ibo;

    std::vector<std::vector<glm::mat4>> splitModels;
    unsigned int cachedModelCount;
    std::vector<std::string> uniformLocations;

    void setInstanceModels(const std::vector<glm::mat4>& models) {
        splitModels.clear();
        splitModels.reserve((models.size() + INSTANCE_COUNT - 1) / INSTANCE_COUNT);

        for (std::size_t i = 0; i < models.size(); i += INSTANCE_COUNT) {
            auto last = std::min(models.size(), i + INSTANCE_COUNT);
            splitModels.emplace_back(models.begin() + i, models.begin() + last);
        }
    }

    void cacheUniformLocations(Shader shader) {
        uniformLocations.resize(INSTANCE_COUNT);
        for (int i = 0; i < INSTANCE_COUNT; i++)
            uniformLocations[i] = "models[" + std::to_string(i) + "]";
    }

    void updateDirectionVectors() {
        glm::vec3 localFront = glm::vec3(0.0f, 0.0f, -1.0f);
        glm::vec3 localUp    = glm::vec3(0.0f, 1.0f,  0.0f);
        glm::vec3 localRight = glm::vec3(1.0f, 0.0f,  0.0f);

        glm::vec3 f = rotation * localFront;
        glm::vec3 u = rotation * localUp;
        glm::vec3 r = rotation * localRight;

        if (glm::length(f) > 0.0001f) front = glm::normalize(f);
        else front = glm::vec3(0.0f, 0.0f, -1.0f);

        if (glm::length(u) > 0.0001f) up = glm::normalize(u);
        else up = glm::vec3(0.0f, 1.0f, 0.0f);

        if (glm::length(r) > 0.0001f) right = glm::normalize(r);
        else right = glm::vec3(1.0f, 0.0f, 0.0f);
    }


    void updateModelMatrix() {
        model = glm::mat4(1.0f);
        model = glm::translate(model, position);
        model *= glm::toMat4(rotation);
        model = glm::scale(model, scale);
        model = glm::translate(model, -origin);
    }

    void parseOBJ(const char* path) {
        std::vector<float> posData, normData, texData;
        std::vector<unsigned int> posIdx, normIdx, texIdx;

        std::ifstream file(path);
        std::string line, prefix;
        std::stringstream ss;

        while (std::getline(file, line)) {
            ss.clear();
            ss.str(line);
            ss >> prefix;

            if (prefix == "v") {
                for (int i = 0; i < 3; ++i) {
                    float val; ss >> val; posData.push_back(val);
                }
            } else if (prefix == "vt") {
                for (int i = 0; i < 2; ++i) {
                    float val; ss >> val; texData.push_back(val);
                }
            } else if (prefix == "vn") {
                for (int i = 0; i < 3; ++i) {
                    float val; ss >> val; normData.push_back(val);
                }
            } else if (prefix == "f") {
                for (int i = 0; i < 3; ++i) {
                    std::string v;
                    ss >> v;
                    size_t p1 = v.find("/");
                    size_t p2 = v.find_last_of("/");

                    posIdx.push_back(std::stoi(v.substr(0, p1)) - 1);
                    texIdx.push_back(std::stoi(v.substr(p1 + 1, p2 - p1 - 1)) - 1);
                    normIdx.push_back(std::stoi(v.substr(p2 + 1)) - 1);
                }
            }
        }

        vertices.resize(posIdx.size() * VERTEX_WIDTH);
        indices.resize(posIdx.size());

        for (int i = 0; i < posIdx.size(); ++i) {
            int vi = i * VERTEX_WIDTH;

            vertices[vi+0] = posData[posIdx[i]*3 + 0];
            vertices[vi+1] = posData[posIdx[i]*3 + 1];
            vertices[vi+2] = posData[posIdx[i]*3 + 2];

            vertices[vi+3] = texData[texIdx[i]*2 + 0];
            vertices[vi+4] = texData[texIdx[i]*2 + 1];

            vertices[vi+5] = normData[normIdx[i]*3 + 0];
            vertices[vi+6] = normData[normIdx[i]*3 + 1];
            vertices[vi+7] = normData[normIdx[i]*3 + 2];

            indices[i] = i;
        }
    }

    void loadMeshData(float _vertices[], unsigned int _vertexCount, unsigned int _indices[], unsigned int _indexCount) {
        vertices.resize(_vertexCount);
        for (int i = 0; i < _vertexCount; ++i)
            vertices[i] = _vertices[i];
            
        indices.resize(_indexCount);
        for (int i = 0; i < _indexCount; ++i)
            indices[i] = _indices[i];
    }

    void generateBuffers() {
        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);

        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

        glGenBuffers(1, &ibo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, VERTEX_WIDTH * sizeof(float), (void*)0);

        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, VERTEX_WIDTH * sizeof(float), (void*)(3 * sizeof(float)));

        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, VERTEX_WIDTH * sizeof(float), (void*)(5 * sizeof(float)));
    }
};