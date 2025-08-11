#pragma once

#include "shader.hpp"

#include "drone.hpp"
#include "neural_net.hpp"
#include "box.hpp"
#include "spatial_grid.hpp"

#include <vector>
#include <memory>
#include <iterator>
#include <random>

class Simulation {
public:
    std::vector<std::unique_ptr<Drone>> drones;
    std::unique_ptr<NeuralNet> droneAI;
    std::unique_ptr<Box> simBounds;
    std::unique_ptr<SpatialGrid> grid;

    int obstacleCount, agentCount;
    std::vector<std::unique_ptr<Box>> obstacles;
    std::vector<glm::mat4> obstacleModels;
    std::vector<std::unique_ptr<BoxCollider>> obstacleColliders;
    std::vector<float> prevDist;

    Simulation(glm::vec3 simMin, glm::vec3 simMax, float gridSize, float safeRadius, int obstacleCount, int agentCount = 1)
             : obstacleCount(obstacleCount), agentCount(agentCount)
    {
        for (int i = 0; i < agentCount; ++i)
            drones.push_back(std::make_unique<Drone>(glm::vec3(0.0f)));
        droneAI = std::make_unique<NeuralNet>(23, 4);
        droneAI->initializeHiddenLayers(4, 12);
        
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
        
        prevDist.resize(agentCount);
    }

    void update(float deltaTime, const glm::vec3& target) {
        for (int i = 0; i < agentCount; ++i) {
            prevDist[i] = glm::distance(drones[i]->position, target);

            std::vector<int> nearby = grid->queryNearby(drones[i]->position, 50.0f);
            glm::vec3 closestMin(0), closestMax(0);

            float minDistObs = std::numeric_limits<float>::max();
            for (int idx : nearby) {
                float distObs = glm::distance(drones[i]->position, obstacles[idx]->position);
                if (distObs < minDistObs) {
                    minDistObs = distObs;
                    closestMin = obstacles[idx]->min;
                    closestMax = obstacles[idx]->max;
                }
            }

            // Normalize inputs
            glm::vec3 normPos = drones[i]->position / 250.0f;
            glm::quat normRot = glm::normalize(drones[i]->rotation + glm::quat(glm::vec4(1e-6f)));
            glm::vec3 normVel = glm::normalize(drones[i]->velocity + glm::vec3(1e-6f));
            glm::quat normAngVel = glm::normalize(drones[i]->angularVelocity + glm::quat(glm::vec4(1e-6f)));
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
                o = (o + 1.0f) * 0.5f;
            }

            std::cout << "\rOutput : " << glm::to_string(glm::vec4(output[0], output[1], output[2], output[3])) << std::flush;
            
            drones[i]->setPropellerThrusts(output);
            drones[i]->update(deltaTime);
            
            // Compute reward = progress toward target
            float newDist = glm::distance(drones[i]->position, target);
            float reward = (prevDist[i] - newDist) * 10.0f; // positive if closer
            prevDist[i] = newDist;
            
            // Penalties
            if (glm::compMax(drones[i]->position) > 250.0f || glm::compMin(drones[i]->position) < -250.0f) {
                reward -= 50.0f;
                drones[i]->reset();
            }
            for (int idx : nearby) {
                if (drones[i]->collider->intersects(*obstacleColliders[idx])) {
                    reward -= 100.0f;
                    drones[i]->reset();
                }
            }
            
            // Define target outputs based on direction toward target
            std::vector<float> targetOutputs(4, 0.5f); // neutral thrust baseline

            // Calculate desired thrust bias toward target
            glm::vec3 toTarget = glm::normalize(target - drones[i]->position);
            glm::vec3 forward   = drones[i]->mesh->front;
            glm::vec3 up        = drones[i]->mesh->up;
            glm::vec3 right     = drones[i]->mesh->right;

            // Map directional alignment to propeller thrusts
            float forwardDot = glm::dot(forward, toTarget);
            float upDot      = glm::dot(up, toTarget);
            float rightDot   = glm::dot(right, toTarget);

            // Example mapping: front/back thrust pair for pitch, left/right pair for roll
            targetOutputs[0] = glm::clamp(0.5f + forwardDot * 0.5f, 0.0f, 1.0f); // front-left
            targetOutputs[1] = glm::clamp(0.5f + forwardDot * 0.5f, 0.0f, 1.0f); // front-right
            targetOutputs[2] = glm::clamp(0.5f - forwardDot * 0.5f, 0.0f, 1.0f); // back-left
            targetOutputs[3] = glm::clamp(0.5f - forwardDot * 0.5f, 0.0f, 1.0f); // back-right

            // Add reward influence
            float rewardScale = glm::clamp(reward / 10.0f, -1.0f, 1.0f);
            for (auto &t : targetOutputs) {
                t += rewardScale * 0.1f; // small adjustment based on reward
                t = glm::clamp(t, 0.0f, 1.0f);
            }

            // Train toward better outputs
            droneAI->train(inputs, targetOutputs, 0.1f);
        }
    }

    void render (Shader shader) {
        drones[0]->render(shader);
        
        simBounds->render(shader);
        obstacles[0]->renderInstanced(shader, obstacleModels);
    }
};