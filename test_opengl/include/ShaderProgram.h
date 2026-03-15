#pragma once
#include <glad/glad.h> // glad должен включаться первым
#include <vector>

#include "Shader.h"

namespace kas::gui
{
    class ShaderProgram final {
    public:
        explicit ShaderProgram(std::vector<Shader> shaders);

        void activate() const;

        ~ShaderProgram();

        [[nodiscard]] GLuint getProgramID() const& noexcept;

    private:
        GLuint m_id;
    };
}