#pragma once

#include "mesh.hpp"
#include "sphere_collider.hpp"

#include "propeller.hpp"

class Drone {
public:
    std::unique_ptr<Mesh> mesh;
    std::unique_ptr<SphereCollider> collider;

    glm::vec3 position;
    glm::quat rotation;

    glm::vec3 velocity;
    glm::vec3 angularVelocity;
    glm::vec3 inertia;

    Drone(const glm::vec3& _position, const glm::quat _rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f),
          const glm::vec3 _color = glm::vec3(0.6f, 0.6f, 0.65f), float _mass = 0.064f, float _gravity = 9.807f)
        : position(_position), rotation(_rotation), mass(_mass), gravity(_gravity)
    {
        mesh = std::make_unique<Mesh>("../res/models/drone.obj");
        mesh->setPosition(_position);
        mesh->setRotation(_rotation);
        mesh->setColor(_color);
        collider = std::make_unique<SphereCollider>(*mesh);

        velocity = glm::vec3(0.0f);
        angularVelocity = glm::vec3(1e-6f);

        float radius = collider->radius;
        float I = 0.4f * mass * radius * radius;
        inertia = glm::vec3(I);

        initPropellers();
    }

    void update(float deltaTime) {
        glm::vec3 netForce(0.0f);
        glm::vec3 netTorque(0.0f);

        const float maxThrust = 10.0f;
        const float spinTorqueScale = 0.1f;

        for (auto& prop : propellers) {
            glm::vec3 relPos = prop->mesh->position - mesh->position;
            float t = prop->thrust * maxThrust;
            glm::vec3 force = t * mesh->up;
            
            netForce += force;
            netTorque += glm::cross(relPos, force);
            netTorque.y += (prop->type == PROPELLER_TYPE_CW ? -1.0f : 1.0f) * prop->thrust * spinTorqueScale;
        }
        
        netForce += glm::vec3(0.0f, -mass * gravity, 0.0f) * 40.0f;
        
        glm::vec3 acceleration = netForce / mass;
        velocity += acceleration * deltaTime;
        velocity *= 0.98f;
        position += velocity * deltaTime;
        
        glm::vec3 angularAccel = netTorque / inertia;
        angularVelocity += angularAccel * deltaTime;
        angularVelocity *= 0.98f;
        
        float angularSpeed = glm::length(angularVelocity);
        glm::vec3 axis = glm::normalize(angularVelocity);
        float angle = angularSpeed * deltaTime;
        glm::quat deltaRot = glm::angleAxis(angle, axis);
        rotation = glm::normalize(deltaRot * rotation);
        
        mesh->setPosition(position);
        mesh->setRotation(rotation);
        collider->updateBounds(*mesh);

        for (auto& prop : propellers)
            prop->update(deltaTime);
    }


    void render(Shader shader) {
        mesh->render(shader);
        for (int i = 0; i < propellers.size(); ++i)
            propellers[i]->render(shader);
        
        #ifdef DEBUG_MODE
            collider->render(shader);
        #endif
    }

    void setPropellerThrusts(const std::array<float, 4>& thrusts) {
        for (int i = 0; i < 4 && i < propellers.size(); ++i) {
            propellers[i]->setTargetThrust(thrusts[i]);
        }
    }

private:
    std::vector<std::unique_ptr<Propeller>> propellers;

    float mass;
    float gravity;

    void initPropellers() {
        const float scale = 0.75f;
        const glm::vec3 color(0.2f, 0.2f, 0.25f);

        auto add = [&](unsigned int type, glm::vec3 relPos, glm::quat relRot) {
            propellers.push_back(std::make_unique<Propeller>(type, mesh, relPos, relRot, scale, color));
        };

        add(PROPELLER_TYPE_CW , {-8.48485f, 0.81592f, 8.5198f}, glm::vec3(0, 0,  0.17253f));
        add(PROPELLER_TYPE_CCW, { 8.48485f, 0.81592f, 8.5198f}, glm::vec3(0, 0, -0.17253f));
        add(PROPELLER_TYPE_CCW, {-6.91386f, 0.7962f, -8.5424f}, glm::vec3(-0.15721f, 0, 0.06731f));
        add(PROPELLER_TYPE_CW , { 6.91386f, 0.7962f, -8.5424f}, glm::vec3(-0.15721f, 0, -0.06731f));
    }
};