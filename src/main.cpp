#include "app.hpp"
#include "shader.hpp"
#include "camera.hpp"

#include "simulation.hpp"

int main() {
    App app("Hello world!", 0, 0, true);
    GLFWwindow *window = app.getWindowContext();

    Shader shader("../res/shaders/light_vert.glsl", "../res/shaders/light_frag.glsl");

    Camera camera(glm::vec3(0.0f, 0.0f, -6.0f));
    glm::mat4 proj = glm::perspective(70.0f, app.getAspectRatio(), 1.0f, 3.0f);
    camera.setInputMode(window);

    Simulation sim(glm::vec3(-250.0f), glm::vec3(250.0f), 25.0f, 50.0f, 512, 1);

    Box box(glm::vec3(-5.0f), glm::vec3(5.0f));
    box.translate({0.0f, 50.0f, 0.0f});
    box.setColor({1.0f, 0.0f, 0.0f, 0.2f});

    shader.bind();
    app.run([&](float deltaTime) {
        camera.processKeyboard(window, deltaTime);
        camera.processMouse(window, deltaTime);
        camera.setUniforms(shader, proj);

        shader.setUniform3f("lightColor", {0.96f, 0.98f, 1.0f});
        shader.setUniform3f("lightPos", {0.0f, 80.0f, 0.0f});
        shader.setUniform3f("viewPos", camera.position);

        sim.update(deltaTime, {0.0f, 50.0f, 0.0f});
        sim.render(shader);

        box.render(shader);
    });

    return 0;
}