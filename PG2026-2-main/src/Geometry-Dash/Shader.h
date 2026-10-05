// Shader.h - PESSOA 1 (motor de renderização)
// Compila o vertex e o fragment shader, linka o programa de shader
// e oferece funções para enviar uniforms.
#pragma once

#include <glm/glm.hpp>
#include <string>

class Shader
{
public:
    unsigned int ID = 0; // identificador do programa de shader

    // Compila e linka. Retorna false (e imprime o log) se houver erro.
    bool compile(const char *vertexSource, const char *fragmentSource);

    void use() const; // glUseProgram

    // Envio de uniforms (o nome deve ser igual ao declarado no GLSL)
    void setInt(const std::string &name, int value) const;
    void setVec2(const std::string &name, const glm::vec2 &value) const;
    void setVec4(const std::string &name, const glm::vec4 &value) const;
    void setMat4(const std::string &name, const glm::mat4 &value) const;
};
