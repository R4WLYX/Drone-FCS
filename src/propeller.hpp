#pragma once

#include "mesh.hpp"
#include "box_collider.hpp"

#define PROPELLER_TYPE_CW  0
#define PROPELLER_TYPE_CCW 1

class Propeller {
public:
    std::unique_ptr<Mesh> mesh;
    std::unique_ptr<BoxCollider> collider;
    
    glm::vec3 relPos;
    glm::quat relRot;

    unsigned int type;
    float spinAngle;
    float targetThrust;
    float thrust;

    Propeller(unsigned int _type, const std::unique_ptr<Mesh>& _droneMesh, glm::vec3 _relPos,
              glm::quat _relRot, float _scale, glm::vec3 _color)
            : type(_type), droneMesh(_droneMesh.get()), startPos(droneMesh->position),
              startRot(_droneMesh->rotation), relPos(_relPos), relRot(_relRot)
    {
        mesh = std::make_unique<Mesh>(
            (type == PROPELLER_TYPE_CW)
            ? "../res/models/cw-propeller.obj"
            : "../res/models/ccw-propeller.obj");
        collider = std::make_unique<BoxCollider>(*mesh);
        
        mesh->setScale(_scale);
        mesh->setColor(_color);
        mesh->setPosition(startPos);
        mesh->translate(relPos);
        mesh->setRotation(startRot);
        mesh->rotate(relRot);

        spinAngle = 0.0f;
        targetThrust = 0.0f;
        thrust = 0.0f;
    }
    
    void update(float deltaTime) {
        thrust += (targetThrust - thrust) * 5.0f * deltaTime;

        spinAngle += ((type == PROPELLER_TYPE_CW)? thrust : -thrust) * 48.0f * deltaTime;
        glm::quat spinRot = glm::angleAxis(spinAngle, droneMesh->up);
        glm::quat worldRot = droneMesh->rotation * relRot;
        glm::vec3 worldPos = droneMesh->position + droneMesh->rotation * relPos;

        mesh->setRotation(worldRot);
        mesh->rotate(spinRot);
        mesh->setPosition(worldPos);
    }

    void reset() {
        spinAngle = 0.0f;
        targetThrust = 0.0f;
        thrust = 0.0f;

        glm::quat spinRot = glm::angleAxis(spinAngle, droneMesh->up);
        glm::quat worldRot = droneMesh->rotation * relRot;
        glm::vec3 worldPos = droneMesh->position + droneMesh->rotation * relPos;

        mesh->setRotation(worldRot);
        mesh->rotate(spinRot);
        mesh->setPosition(worldPos);
    }

    void render (Shader shader) {
        mesh->render(shader);
    }

    void setTargetThrust(float value) {
        targetThrust = glm::clamp(value, 0.0f, 1.0f);
    }

private:
    Mesh* droneMesh;
    glm::vec3 startPos;
    glm::quat startRot;
};