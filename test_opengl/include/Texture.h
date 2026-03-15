#pragma once
#include <glad/glad.h> // glad должен включаться первым
#include <string>
#include <cstdint>
#include "Utils.h"
#include "ShaderProgram.h"

namespace kas::gui
{
    class Texture final {
    public:
        [[nodiscard]] static Texture getTexture(const std::string& filename, GLenum type, GLenum slot, GLenum format, GLenum pixel_type);

        explicit Texture(const std::string& filename, GLenum type, GLenum slot, GLenum format, GLenum pixel_type);

        void bind() const;

        void unbind() const;

        Texture& setupUnit(ShaderProgram& shader_program, const std::string& uniform, GLuint unit)&;

        const Texture& setupUnit(ShaderProgram& shader_program, const std::string& uniform, GLuint unit) const&;

    private:
        utils::GuardID<utils::guard_types::gl_texture_t> m_id;
        GLenum m_type;
    };
}