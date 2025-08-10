#include "app.hpp"
#include "shader.hpp"
#include "camera.hpp"

#include "spatial_grid.hpp"
#include "drone.hpp"
#include "box.hpp"

#include <random>

int main() {
    App app("Hello world!", 0, 0, true);
    GLFWwindow *window = app.getWindowContext();

    Shader shader("../res/shaders/light_vert.glsl", "../res/shaders/light_frag.glsl");

    Camera camera(glm::vec3(0.0f, 0.0f, -6.0f));
    glm::mat4 proj = glm::perspective(70.0f, app.getAspectRatio(), 1.0f, 3.0f);
    camera.setInputMode(window);
    
    // Simulation stuff
    Drone drone(glm::vec3(0.0f));
    Box simBounds(glm::vec3(-250.0f), glm::vec3(250.0f),
                  glm::vec4(glm::vec3(1.0f), 0.1f));
    simBounds.flipNormals();

    int obstacleCount = 1024;
    std::vector<std::unique_ptr<Box>> obstacles;
    std::vector<glm::mat4> obstacleModels;
    std::vector<std::unique_ptr<BoxCollider>> obstacleColliders;

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> posRnd(-220.0f, 220.0f);
    std::uniform_real_distribution<float> rotRnd(0.0f, 1.0f);
    std::uniform_real_distribution<float> scaleRnd(0.5f, 2.0f);

    float safeRadius = 50.0f;
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

    SpatialGrid grid(20.0f, glm::vec3(-250.0f), glm::vec3(250.0f));

    for (int i = 0; i < obstacles.size(); i++) {
        grid.insertObstacle(i, obstacles[i]->position);
    }
    // sim stuff

    shader.bind();
    app.run([&](float deltaTime) {
        camera.processKeyboard(window, deltaTime);
        camera.processMouse(window, deltaTime);
        camera.setUniforms(shader, proj);

        shader.setUniform3f("lightColor", glm::vec3(1.0f));
        shader.setUniform3f("lightPos", glm::vec3(0.0f, 80.0f, -25.0f));
        shader.setUniform3f("viewPos", camera.position);

        std::vector<int> nearby = grid.queryNearby(drone.position, 50.0f);
        float minDist = std::numeric_limits<float>::max();
        glm::vec3 closestMin, closestMax;

        for (int idx : nearby) {
            float dist = glm::distance(drone.position, obstacles[idx]->position);
            if (dist < minDist) {
                minDist = dist;
                closestMin = obstacles[idx]->min;
                closestMax = obstacles[idx]->max;
            }
            if (drone.collider->intersects(*obstacleColliders[idx])) {
                drone.reset();
            }
        }

        drone.update(deltaTime);
        drone.render(shader);
        simBounds.render(shader);

        obstacles[0]->renderInstanced(shader, obstacleModels);
    });


    return 0;
}