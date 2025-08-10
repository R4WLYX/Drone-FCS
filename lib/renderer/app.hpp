#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <chrono>
#include <functional>

class App {
public:
    int windowWidth;
    int windowHeight;

    App(const char* title = "untitled", const int width = 0, const int height = 0, bool fullscreen = false)
        : windowWidth(width), windowHeight(height)
    {
        if (!glfwInit())
            std::cerr << "Error: GLFW Init Fialed.\n";

        monitor = glfwGetPrimaryMonitor();
        if (!monitor) {
            glfwTerminate();
            std::cerr << "Error: Monitor Not Found.\n";
            std::exit(-1);
        }

        if (width < 1 || height < 1 || fullscreen)
            glfwGetMonitorWorkarea(monitor, NULL, NULL, &windowWidth, &windowHeight);
        
        windowScreenRatio = float(windowWidth)/float(windowHeight);

        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_SAMPLES, 16);

        if (fullscreen)
            window = glfwCreateWindow(windowWidth, windowHeight, title, monitor, NULL);
        else
            window = glfwCreateWindow(windowWidth, windowHeight, title, NULL, NULL);

        if (!window) {
            glfwTerminate();
            std::cerr << "Error: Window Not Found.\n";
            std::exit(-1);
        }

        glfwMakeContextCurrent(window);
        glfwSwapInterval(1);

        if (glewInit() != GLEW_OK) {
            std::cerr << "Error: GLEW Init Failed.\n";
            std::exit(-1);
        }

        GLenum err = glGetError();
        if (err != GL_NO_ERROR) {
            std::cerr << "OpenGL Error: " << err << std::endl;
            std::exit(-1);
        }
        
        glEnable(GL_MULTISAMPLE);
        glEnable(GL_DEPTH_TEST);
        glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        std::cout << "OpenGL Version : " << glGetString(GL_VERSION) << '\n';
        std::cout << "OpenGL Vendor  : " << glGetString(GL_VENDOR) << '\n';
        std::cout << "OpenGL Renderer: " << glGetString(GL_RENDERER) << '\n';
    }

    ~App() {
        glfwDestroyWindow(window);
        glfwTerminate();
    }

    void run(const std::function<void(float)> &callback) {
        std::chrono::time_point T0 = std::chrono::high_resolution_clock::now();
        std::chrono::time_point T1 = T0;
        float deltaTime;

        while(!glfwWindowShouldClose(window)) {
            T1 = std::chrono::high_resolution_clock::now();
            deltaTime = (float)std::chrono::duration_cast<std::chrono::milliseconds>(T1 - T0).count()/1000.0f;

            glClearColor(0.1f, 0.1f, 0.12f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glfwPollEvents();

            callback(deltaTime);
            
            std::cout << '\r' << "FPS -- " << 1.0f/deltaTime << std::flush;

            glfwSwapBuffers(window);
            T0 = T1;
        }
    }

    GLFWwindow* getWindowContext() const { return window; }
    GLFWmonitor* getMonitorContext() const { return monitor; }
    float getAspectRatio() const { return windowScreenRatio; }

private:
    GLFWwindow *window;
    GLFWmonitor *monitor;

    float windowScreenRatio;
};