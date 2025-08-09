#pragma once

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <algorithm>
#include <cmath>

#include "shader.hpp"

#ifndef CAMERA_DEFAULT_VELOCITY    
    #define CAMERA_DEFAULT_VELOCITY   12.0f
#endif

#ifndef CAMERA_MAX_VELOCITY    
    #define CAMERA_MAX_VELOCITY       144.0f
#endif

#ifndef CAMERA_VELOCITY_STEP    
    #define CAMERA_VELOCITY_STEP       4.0f
#endif

#ifndef CAMERA_DEFAULT_SENSITIVITY
    #define CAMERA_DEFAULT_SENSITIVITY 0.2f
#endif

class Camera {
public:
    glm::vec3 position;
    glm::vec3 front, up, right;
    glm::vec2 rotation; // pitch (x), yaw (y)

    float velocity;
    float sensitivity;
    float pitchLimit;

    Camera(const glm::vec3& _position = glm::vec3(0.0f))
        : position(_position), rotation(0.0f), velocity(CAMERA_DEFAULT_VELOCITY), sensitivity(CAMERA_DEFAULT_SENSITIVITY), pitchLimit(glm::half_pi<float>() - 0.01f), firstMouse(true)
    {
        updateVectors();
        updateViewMatrix();
    }

    void setInputMode(GLFWwindow* window) {
        glfwSetWindowUserPointer(window, this);
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        glfwSetScrollCallback(window, scrollCallback);
    }

    void processKeyboard(GLFWwindow* window, float deltaTime) {
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            position += front * velocity * deltaTime;
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            position -= front * velocity * deltaTime;
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            position += right * velocity * deltaTime;
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            position -= right * velocity * deltaTime;
        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
            position += up * velocity * deltaTime;
        if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
            position -= up * velocity * deltaTime;
        
        updateViewMatrix();
    }

    void processMouse(GLFWwindow* window, float deltaTime) {
        double mouseX, mouseY;
        glfwGetCursorPos(window, &mouseX, &mouseY);

        if (firstMouse) {
            lastMouse = {mouseX, mouseY};
            firstMouse = false;
        }

        float offsetX = float(mouseX - lastMouse.x);
        float offsetY = float(mouseY - lastMouse.y);
        lastMouse = {mouseX, mouseY};

        rotation.x -= offsetY * sensitivity      * deltaTime;
        rotation.y += offsetX * sensitivity/2.0f * deltaTime;

        rotation.x = std::clamp(rotation.x, -pitchLimit, +pitchLimit);

        updateVectors();
        updateViewMatrix();
    }

    void setUniforms(Shader shader, const glm::mat4& projection) {
        shader.setUniformMat4f("view", view);
        shader.setUniformMat4f("proj", projection);
    }

private:
    glm::vec2 lastMouse;
    bool firstMouse;

    glm::mat4 view;

    static void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);

    void updateVectors() {
        front.x = cosf(rotation.x) * cosf(rotation.y);
        front.y = sinf(rotation.x);
        front.z = cosf(rotation.x) * sinf(rotation.y);
        front = glm::normalize(front);
        right = glm::normalize(glm::cross(front, glm::vec3(0, 1, 0)));
        up    = glm::normalize(glm::cross(right, front));
    }

    void updateViewMatrix() {
        view = glm::lookAt(position, position + front, glm::vec3(0, 1, 0));
    }
};

void Camera::scrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
    Camera* cam = static_cast<Camera*>(glfwGetWindowUserPointer(window));
    if (yoffset < 0) cam->velocity -= CAMERA_VELOCITY_STEP;
    else if (yoffset > 0) cam->velocity += CAMERA_VELOCITY_STEP;
    cam->velocity = std::clamp(cam->velocity, CAMERA_VELOCITY_STEP, CAMERA_MAX_VELOCITY);
}