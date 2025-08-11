#pragma once

#include "shader.hpp"

#include "spatial_grid.hpp"
#include "drone.hpp"
#include "box.hpp"

#include <vector>
#include <memory>
#include <random>

class Simulation {
public:
    std::unique_ptr<Drone> drone;
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
            obstacleModels.push_back(obstacles[i]->model);
            obstacleColliders.push_back(std::make_unique<BoxCollider>(*obstacles[i]));
        }

        grid = std::make_unique<SpatialGrid>(gridSize, simMin, simMax);

        for (int i = 0; i < obstacles.size(); i++)
            grid->insertObstacle(i, obstacles[i]->position);
    }

    void update(float deltaTime) {
        std::vector<int> nearby = grid->queryNearby(drone->position, 50.0f);
        float minDist = std::numeric_limits<float>::max();
        glm::vec3 closestMin, closestMax;

        for (int idx : nearby) {
            float dist = glm::distance(drone->position, obstacles[idx]->position);
            if (dist < minDist) {
                minDist = dist;
                closestMin = obstacles[idx]->min;
                closestMax = obstacles[idx]->max;
            }
            if (drone->collider->intersects(*obstacleColliders[idx])) {
                drone->reset();
            }
        }
        
        drone->setPropellerThrusts({0.8f, 0.8f, 0.8f, 0.8f});
        drone->update(deltaTime);
    }

    void render (Shader shader) {
        drone->render(shader);
        simBounds->render(shader);

        obstacles[0]->renderInstanced(shader, obstacleModels);
    }
};