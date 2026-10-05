// Shader.cpp - PESSOA 1 (motor de renderização)
#include "Shader.h"

#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

// Compila um shader (vertex ou fragment) e confere se deu erro
static unsigned int compileStage(GLenum type, const char *source)
{
    unsigned int shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);

    int success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        char log[1024];
        glGetShaderInfoLog(shader, sizeof(log), NULL, log);
        std::cerr << "ERRO ao compilar o "
                  << (type == GL_VERTEX_SHADER ? "vertex" : "fragment")
                  << " shader:\n" << log << std::endl;
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}

bool Shader::compile(const char *vertexSource, const char *fragmentSource)
{
    unsigned int vs = compileStage(GL_VERTEX_SHADER, vertexSource);
    unsigned int fs = compileStage(GL_FRAGMENT_SHADER, fragmentSource);
    if (vs == 0 || fs == 0)
        return false;

    // O programa de shader é o conjunto: no mínimo 1 vertex + 1 fragment shader
    ID = glCreateProgram();
    glAttachShader(ID, vs);
    glAttachShader(ID, fs);
    glLinkProgram(ID);

    int success;
    glGetProgramiv(ID, GL_LINK_STATUS, &success);
    if (!success)
    {
        char log[1024];
        glGetProgramInfoLog(ID, sizeof(log), NULL, log);
        std::cerr << "ERRO ao linkar o programa de shader:\n" << log << std::endl;
    }

    // Depois de linkados, os shaders individuais não são mais necessários
    glDeleteShader(vs);
    glDeleteShader(fs);
    return success != 0;
}

void Shader::use() const
{
    glUseProgram(ID);
}

void Shader::setInt(const std::string &name, int value) const
{
    glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
}

void Shader::setVec2(const std::string &name, const glm::vec2 &value) const
{
    glUniform2f(glGetUniformLocation(ID, name.c_str()), value.x, value.y);
}

void Shader::setVec4(const std::string &name, const glm::vec4 &value) const
{
    glUniform4f(glGetUniformLocation(ID, name.c_str()), value.x, value.y, value.z, value.w);
}

void Shader::setMat4(const std::string &name, const glm::mat4 &value) const
{
    glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, glm::value_ptr(value));
}
