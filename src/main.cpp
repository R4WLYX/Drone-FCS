#include "app.hpp"
#include "shader.hpp"
#include "camera.hpp"

#include "drone.hpp"
#include "box.hpp"

#include <random>

int main() {
    App app("Hello world!", 0, 0, false);
    GLFWwindow *window = app.getWindowContext();

    Shader shader("../res/shaders/light_vert.glsl", "../res/shaders/light_frag.glsl");

    Camera camera(glm::vec3(0.0f, 0.0f, -6.0f));
    glm::mat4 proj = glm::perspective(70.0f, app.getAspectRatio(), 1.0f, 3.0f);
    camera.setInputMode(window);
    
    Drone drone(glm::vec3(0.0f));
    Box simBounds(glm::vec3(-250.0f), glm::vec3(250.0f),
                  glm::vec4(glm::vec3(1.0f), 0.1f));
    simBounds.flipNormals();

    std::vector<std::unique_ptr<Box>> obstacles;
    int obstacleCount = 500;

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> posRnd(-200.0f, 200.0f);
    std::uniform_real_distribution<float> rotRnd(0.0f, 1.0f);
    std::uniform_real_distribution<float> scaleRnd(0.5f, 2.0f);

    for (int i = 0; i < obstacleCount; i++) {
        obstacles.push_back(std::make_unique<Box>(glm::vec3(-5.0f), glm::vec3(5.0f)));
        obstacles[i]->translate({posRnd(gen), posRnd(gen), posRnd(gen)});
        obstacles[i]->rotate({rotRnd(gen), rotRnd(gen), rotRnd(gen)});
        obstacles[i]->scaleBy(scaleRnd(gen));
    }

    shader.bind();
    app.run([&](float deltaTime) {
        camera.processKeyboard(window, deltaTime);
        camera.processMouse(window, deltaTime);
        camera.setUniforms(shader, proj);

        shader.setUniform3f("lightColor", glm::vec3(1.0f));
        shader.setUniform3f("lightPos", glm::vec3(0.0f, 80.0f, -25.0f));
        shader.setUniform3f("viewPos", camera.position);

        drone.update(deltaTime);
        drone.render(shader);
        simBounds.render(shader);

        for (const auto& obstacle : obstacles)
            obstacle->render(shader);
    });


    return 0;
}