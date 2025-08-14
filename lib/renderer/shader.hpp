#pragma once

#include <GL/glew.h>

#include <glm/glm.hpp>

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <unordered_map>

class Shader {
public:
    Shader(const std::string& vertex, const std::string& fragment, const std::string& geometry = "", const std::string& compute = "") {
        program = glCreateProgram();
        addShader(vertex, GL_VERTEX_SHADER);
        addShader(fragment, GL_FRAGMENT_SHADER);
        addShader(geometry, GL_GEOMETRY_SHADER);
        addShader(compute, GL_COMPUTE_SHADER);
        
        glLinkProgram(program);

        int success;
        glGetProgramiv(program, GL_LINK_STATUS, &success);
        if (!success) {
            char infoLog[512];
            glGetProgramInfoLog(program, 512, nullptr, infoLog);
            std::cout << "[!] Shader linking failed:\n" << infoLog << '\n';
            std::cin.get();
        }

        glValidateProgram(program);
    }

    ~Shader() {
        glDeleteProgram(program);
    }

    void addShader(const std::string& path, unsigned int type) {
        if (path.empty()) return;
        std::string source = readFile(path);
        const char *shaderSource = source.c_str();
        unsigned int shader = glCreateShader(type);
        glShaderSource(shader, 1, &shaderSource, nullptr);
        glCompileShader(shader);
        glAttachShader(program, shader);
        if(!validateShader(shader, shaderSource, type))
            std::exit(-1);
    }

    int validateShader(unsigned int shader, const char* shaderSource, unsigned int type) {
        int result;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &result);
        if (result == GL_FALSE) {
            int length;
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
            char *message = (char*)alloca(length * sizeof(char));
            glGetShaderInfoLog(shader, length, &length, message);
            std::cout << "[!] Error: Failed to compile fragment shader!\n" << message;
            std::cout << "Source:\n" << shaderSource << "\n\n";
            glDeleteShader(shader);
            std::cin.get();
        }
        return result;
    }

    void bind()   { glUseProgram(program); }
    void unbind() { glUseProgram(0);       }
    
#pragma region uniforms
    void setUniform1i(const std::string& name, int value) {
        glUniform1i(getUniformLocation(name), value);
    }
    void setUniform2i(const std::string& name, int v0, int v1) {
        glUniform2i(getUniformLocation(name), v0, v1);
    }
    void setUniform3i(const std::string& name, int v0, int v1, int v2) {
        glUniform3i(getUniformLocation(name), v0, v1, v2);
    }
    void setUniform4i(const std::string& name, int v0, int v1, int v2, int v3) {
        glUniform4i(getUniformLocation(name), v0, v1, v2, v3);
    }
    void setUniform2i(const std::string& name, const glm::ivec2& vector) {
        glUniform2iv(getUniformLocation(name), 1, &vector[0]);
    }
    void setUniform3i(const std::string& name, const glm::ivec3& vector) {
        glUniform3iv(getUniformLocation(name), 1, &vector[0]);
    }
    void setUniform4i(const std::string& name, const glm::ivec4& vector) {
        glUniform4iv(getUniformLocation(name), 1, &vector[0]);
    }
    void setUniform1f(const std::string& name, float value) {
        glUniform1f(getUniformLocation(name), value);
    }
    void setUniform2f(const std::string& name, float v0, float v1) {
        glUniform2f(getUniformLocation(name), v0, v1);
    }
    void setUniform3f(const std::string& name, float v0, float v1, float v2) {
        glUniform3f(getUniformLocation(name), v0, v1, v2);
    }
    void setUniform4f(const std::string& name, float v0, float v1, float v2, float v3) {
        glUniform4f(getUniformLocation(name), v0, v1, v2, v3);
    }
    void setUniform2f(const std::string& name, const glm::vec2& vector) {
        glUniform2fv(getUniformLocation(name), 1, &vector[0]);
    }
    void setUniform3f(const std::string& name, const glm::vec3& vector) {
        glUniform3fv(getUniformLocation(name), 1, &vector[0]);
    }
    void setUniform4f(const std::string& name, const glm::vec4& vector) {
        glUniform4fv(getUniformLocation(name), 1, &vector[0]);
    }
    void setUniformMat2f(const std::string& name, const glm::mat2& matrix) {
        glUniformMatrix2fv(getUniformLocation(name), 1, GL_FALSE, &matrix[0][0]);
    }
    void setUniformMat3f(const std::string& name, const glm::mat3& matrix) {
        glUniformMatrix3fv(getUniformLocation(name), 1, GL_FALSE, &matrix[0][0]);
    }
    void setUniformMat4f(const std::string& name, const glm::mat4& matrix) {
        glUniformMatrix4fv(getUniformLocation(name), 1, GL_FALSE, &matrix[0][0]);
    }
    void setUniformMat2fv(const std::string& name, const glm::mat2* matrices, size_t count) {
        glUniformMatrix2fv(getUniformLocation(name), static_cast<GLsizei>(count), GL_FALSE, &matrices[0][0][0]);
    }
    void setUniformMat3fv(const std::string& name, const glm::mat3* matrices, size_t count) {
        glUniformMatrix3fv(getUniformLocation(name), static_cast<GLsizei>(count), GL_FALSE, &matrices[0][0][0]);
    }
    void setUniformMat4fv(const std::string& name, const glm::mat4* matrices, size_t count) {
        glUniformMatrix4fv(getUniformLocation(name), static_cast<GLsizei>(count), GL_FALSE, &matrices[0][0][0]);
    }
    
    int getUniformLocation(const std::string& name) {
        if (uniformLocationCache.find(name) != uniformLocationCache.end())
            return uniformLocationCache[name];
    
        int location = glGetUniformLocation(program, name.c_str());
    
        if (location == -1)
            std::cout<<"Warning: uniform '"<<name<<"' doesn't exist!\n";
        
        uniformLocationCache[name] = location;
        return location;
    }
#pragma endregion uniforms

private:
    unsigned int program;
    std::unordered_map<std::string, int> uniformLocationCache;

    std::string readFile(const std::string& path) {
        std::ifstream file(path.c_str(), std::ios_base::binary);
        std::ostringstream buf;
        buf << file.rdbuf();
        return buf.str();
    }
};
