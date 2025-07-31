#include "renderer.hpp"
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <fstream>
#include <sstream>
#include <iostream>

namespace {
    GLuint shaderProgram;
    GLuint vao, vbo, instanceVBO, radiusVBO;
    GLuint uProjectionLoc;

    glm::mat4 projection = glm::mat4(1.0f);

    std::string LoadShaderSource(const char* filepath) {
        std::ifstream file(filepath);
        std::stringstream buffer;
        buffer << file.rdbuf();
        return buffer.str();
    }

    GLuint CompileShader(const char* path, GLenum type) {
        std::string source = LoadShaderSource(path);
        const char* src = source.c_str();
        GLuint shader = glCreateShader(type);
        glShaderSource(shader, 1, &src, nullptr);
        glCompileShader(shader);

        GLint success;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            char log[512];
            glGetShaderInfoLog(shader, 512, nullptr, log);
            std::cerr << "Shader compilation error (" << path << "): " << log << "\n";
        }

        return shader;
    }
}

void Renderer::Init() {
    GLuint vs = CompileShader("../../src/shaders/circle.vs.glsl", GL_VERTEX_SHADER);
    GLuint fs = CompileShader("../../src/shaders/circle.fs.glsl", GL_FRAGMENT_SHADER);
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vs);
    glAttachShader(shaderProgram, fs);
    glLinkProgram(shaderProgram);
    glDeleteShader(vs);
    glDeleteShader(fs);

    uProjectionLoc = glGetUniformLocation(shaderProgram, "u_projection");

    float quadVertices[] = {
        -1, -1,   1, -1,
        -1,  1,   1,  1
    };

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glGenBuffers(1, &instanceVBO);
    glGenBuffers(1, &radiusVBO);
}

void Renderer::UpdateProjection(int width, int height) {
    // Map 0..width and 0..height directly to NDC
    projection = glm::ortho(
        0.0f, static_cast<float>(width),
        0.0f, static_cast<float>(height),
        -1.0f, 1.0f
    );
    glUseProgram(shaderProgram);
    glUniformMatrix4fv(uProjectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
}

void Renderer::RenderFrame(const std::vector<glm::vec2>& positions, const std::vector<float>& radii) {
    glUseProgram(shaderProgram);
    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
    glBufferData(GL_ARRAY_BUFFER, positions.size() * sizeof(glm::vec2), positions.data(), GL_DYNAMIC_DRAW);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribDivisor(1, 1);

    glBindBuffer(GL_ARRAY_BUFFER, radiusVBO);
    glBufferData(GL_ARRAY_BUFFER, radii.size() * sizeof(float), radii.data(), GL_DYNAMIC_DRAW);
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(float), (void*)0);
    glEnableVertexAttribArray(2);
    glVertexAttribDivisor(2, 1);

    glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, positions.size());
}

void Renderer::Cleanup() {
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &instanceVBO);
    glDeleteBuffers(1, &radiusVBO);
    glDeleteProgram(shaderProgram);
}
