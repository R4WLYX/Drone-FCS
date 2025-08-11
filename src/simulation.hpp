#pragma once

#include "shader.hpp"

#include "drone.hpp"
#include "neural_net_test.hpp"
#include "box.hpp"
#include "spatial_grid.hpp"

#include <vector>
#include <memory>
#include <iterator>
#include <random>

class Simulation {
public:
    std::unique_ptr<Drone> drone;
    std::unique_ptr<NeuralNet> droneAI;
    std::unique_ptr<Box> simBounds;
    std::unique_ptr<SpatialGrid> grid;

    int obstacleCount;
    std::vector<std::unique_ptr<Box>> obstacles;
    std::vector<glm::mat4> obstacleModels;
    std::vector<std::unique_ptr<BoxCollider>> obstacleColliders;

    Simulation(glm::vec3 simMin, glm::vec3 simMax, float gridSize, float safeRadius, int obstacleCount)
             : obstacleCount(obstacleCount)
    {
        drone = std::make_unique<Drone>(glm::vec3(0.0f));
        droneAI = std::make_unique<NeuralNet>(23, 4);
        droneAI->initializeHiddenLayers(8, 12);
        
        simBounds = std::make_unique<Box>(
            glm::vec3(-250.0f), glm::vec3(250.0f),
            glm::vec4(glm::vec3(1.0f), 0.1f));
        simBounds->flipNormals();

        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<float> posRnd(
            glm::compMin(simMin) + 20.0f, glm::compMin(simMax) - 20.0f);
        std::uniform_real_distribution<float> rotRnd(0.0f, 1.0f);
        std::uniform_real_distribution<float> scaleRnd(0.5f, 2.0f);

        glm::vec3 pos;
        for (int i = 0; i < obstacleCount; i++) {
            obstacles.push_back(std::make_unique<Box>(glm::vec3(-5.0f), glm::vec3(5.0f)));
            do { pos = { posRnd(gen), posRnd(gen), posRnd(gen) };
            } while (glm::length(pos) < safeRadius);
            obstacles[i]->translate(pos);
            obstacles[i]->rotate({rotRnd(gen), rotRnd(gen), rotRnd(gen)});
            obstacles[i]->scaleBy(scaleRnd(gen));
            obstacles[i]->setColor({glm::vec3(1.0f), 0.8f});
            obstacleModels.push_back(obstacles[i]->model);
            obstacleColliders.push_back(std::make_unique<BoxCollider>(*obstacles[i]));
        }

        grid = std::make_unique<SpatialGrid>(gridSize, simMin, simMax);

        for (int i = 0; i < obstacles.size(); i++)
            grid->insertObstacle(i, obstacles[i]->position);
    }

    void update(float deltaTime, const glm::vec3& target) {
        static float prevDist = glm::distance(drone->position, target);

        std::vector<int> nearby = grid->queryNearby(drone->position, 50.0f);
        glm::vec3 closestMin(0), closestMax(0);

        float minDistObs = std::numeric_limits<float>::max();
        for (int idx : nearby) {
            float distObs = glm::distance(drone->position, obstacles[idx]->position);
            if (distObs < minDistObs) {
                minDistObs = distObs;
                closestMin = obstacles[idx]->min;
                closestMax = obstacles[idx]->max;
            }
        }

        // Normalize inputs
        glm::vec3 normPos = drone->position / 250.0f;
        glm::quat normRot = glm::normalize(drone->rotation + glm::quat(glm::vec4(1e-6f)));
        glm::vec3 normVel = glm::normalize(drone->velocity + glm::vec3(1e-6f));
        glm::quat normAngVel = glm::normalize(drone->angularVelocity + glm::quat(glm::vec4(1e-6f)));
        glm::vec3 normMin = closestMin / 250.0f;
        glm::vec3 normMax = closestMax / 250.0f;
        glm::vec3 normTarget = target / 250.0f;
        
        std::vector<float> inputs;
        inputs.insert(inputs.end(), &normPos[0], &normPos[0] + 3);
        inputs.insert(inputs.end(), &normRot[0], &normRot[0] + 4);
        inputs.insert(inputs.end(), &normVel[0], &normVel[0] + 3);
        inputs.insert(inputs.end(), &normAngVel[0], &normAngVel[0] + 4);
        inputs.insert(inputs.end(), &normMin[0], &normMin[0] + 3);
        inputs.insert(inputs.end(), &normMax[0], &normMax[0] + 3);
        inputs.insert(inputs.end(), &normTarget[0], &normTarget[0] + 3);
        
        // Forward pass with noise for exploration
        droneAI->setInput(inputs);
        auto output = droneAI->forward();
        std::random_device rd;
        std::mt19937 gen(rd());
        std::normal_distribution<float> noise(0.0f, 0.05f);
        for (float &o : output) {
            o += noise(gen);
            o = glm::clamp(o, 0.0f, 1.0f);
        }
        
        drone->setPropellerThrusts(output);
        drone->update(deltaTime);
        
        // Compute reward = progress toward target
        float newDist = glm::distance(drone->position, target);
        float reward = (prevDist - newDist) * 10.0f; // positive if closer
        prevDist = newDist;
        
        // Penalties
        if (glm::compMax(drone->position) > 250.0f || glm::compMin(drone->position) < -250.0f) {
            reward -= 50.0f;
            drone->reset();
        }
        for (int idx : nearby) {
            if (drone->collider->intersects(*obstacleColliders[idx])) {
                reward -= 100.0f;
                drone->reset();
            }
        }
        
        // Define target outputs: if reward positive, keep current thrusts, else randomize
        std::vector<float> targetOutputs = output;
        if (reward < 0.0f) {
            std::uniform_real_distribution<float> thrustDist(0.0f, 1.0f);
            for (auto &t : targetOutputs) t = thrustDist(gen);
        }

        // Train toward better outputs
        droneAI->train(inputs, targetOutputs, 0.01f);
    }

    void render (Shader shader) {
        drone->render(shader);
        simBounds->render(shader);

        obstacles[0]->renderInstanced(shader, obstacleModels);
    }
};