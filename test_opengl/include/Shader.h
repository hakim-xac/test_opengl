#pragma once
#include <glad/glad.h> // glad должен включаться первым
#include <GLFW/glfw3.h>

#include <string>
#include <iostream>
#include <stdexcept>

#include "Utils.h"

namespace kas::gui
{
    class Shader final {
    public:
        explicit Shader(const std::string& filename, GLuint gl_shader_type);

        ~Shader();
        Shader(const Shader&) = delete;
        Shader(Shader&& other) noexcept;
        Shader& operator=(const Shader&) = delete;
        Shader& operator=(Shader&& other) noexcept;

        [[nodiscard]] GLuint getShaderID() const& noexcept;
    private:
        GLuint m_id;
    };
}