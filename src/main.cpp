#include "app.hpp"
#include "shader.hpp"
#include "camera.hpp"
#include "drone.hpp"

#include "sphere_collider.hpp"

// #define DEBUG_MODE

int main() {
    App app("Hello world!", 0, 0, true);
    GLFWwindow *window = app.getWindowContext();

    Shader shader("../res/shaders/light_vert.glsl", "../res/shaders/light_frag.glsl");

    Camera camera(glm::vec3(0.0f, 0.0f, -6.0f));
    glm::mat4 proj = glm::perspective(70.0f, app.getAspectRatio(), 1.0f, 3.0f);
    camera.setInputMode(window);
    
    Drone drone(glm::vec3(0.0f));

    shader.bind();
    app.run([&](float deltaTime) {
        camera.processKeyboard(window, deltaTime);
        camera.processMouse(window, deltaTime);
        camera.setUniforms(shader, proj);

        shader.setUniform3f("lightColor", glm::vec3(1.0f));
        shader.setUniform3f("lightPos", drone.position + glm::vec3(0.0f, 80.0f, -25.0f));
        shader.setUniform3f("viewPos", camera.position);

        float base = 0.65f;
        float delta = 0.4f;

        if (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS) {
            drone.setPropellerThrusts({base + delta, base + delta, base + delta, base + delta}); // Ascend
        } else if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS) {
            drone.setPropellerThrusts({base - delta, base - delta, base - delta, base - delta}); // Descend
        } else if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS) {
            drone.setPropellerThrusts({base + delta, base - delta, base - delta, base + delta}); // Roll left
        } else if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS) {
            drone.setPropellerThrusts({base - delta, base + delta, base + delta, base - delta}); // Roll right
        } else {
            drone.setPropellerThrusts({base, base, base, base});
        }

        drone.update(deltaTime);
        drone.render(shader);
    });


    return 0;
}