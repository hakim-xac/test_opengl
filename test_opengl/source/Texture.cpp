#include "../include/Texture.h"
#include <stb/stb_image.h>
#include <GLFW/glfw3.h>
#include <memory>
#include <stdexcept>

namespace kas::gui
{
    //-----------------

    [[nodiscard]]
    Texture
    Texture::getTexture(const std::string& filename, GLenum type, GLenum slot, GLenum format, GLenum pixel_type)
    {
        return Texture{ filename, type, slot, format, pixel_type };
    }

    //-----------------
    
    Texture::Texture(const std::string& filename, GLenum type, GLenum slot, GLenum format, GLenum pixel_type) :
        m_id{},
        m_type{ type }
    {
        stbi_set_flip_vertically_on_load(true);

        int width{};
        int height{};
        int col_h{};

        std::unique_ptr < uint8_t, decltype([](uint8_t* raw_image_bytes) { stbi_image_free(raw_image_bytes); }) > raw_image_bytes_uptr{
            stbi_load(filename.c_str(), &width, &height, &col_h, 0)
        };

        const auto* bytes{ raw_image_bytes_uptr.get() };
        if (!bytes) [[unlikely]]
            throw std::runtime_error{ "Can not open texture: '" + filename };

        glActiveTexture(slot);
        utils::Guard g{ [this] { glBindTexture(m_type, m_id.getID()); }, [this] { glBindTexture(m_type, 0); } };

        // Настраиваем тип алгоритма, который используется для уменьшения или увеличения изображения
        glTexParameteri(m_type, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
        glTexParameteri(m_type, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        // Настраиваем способ повторения текстуры (если это вообще происходит)
        glTexParameteri(m_type, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(m_type, GL_TEXTURE_WRAP_T, GL_REPEAT);
        // Присваиваем изображение объекту текстуры OpenGL
        glTexImage2D(m_type, 0, GL_RGBA, width, height, 0, format, pixel_type, bytes);
        // Генерируем MipMaps
        glGenerateMipmap(m_type);
    }

    //-----------------
    
    void Texture::bind() const
    {
        glBindTexture(m_type, m_id.getID());
    }

    //-----------------

    void Texture::unbind() const
    {
        glBindTexture(m_type, 0);
    }

    //-----------------

    Texture& Texture::setupUnit(ShaderProgram& shader_program, const std::string& uniform, GLuint unit)&
    {
        const GLint tex_uni{ glGetUniformLocation(shader_program.getProgramID(), uniform.c_str()) };
        shader_program.activate();
        glUniform1i(tex_uni, unit);
        return *this;
    }

    //-----------------

    const Texture& Texture::setupUnit(ShaderProgram& shader_program, const std::string& uniform, GLuint unit) const&
    {
        const GLint tex_uni{ glGetUniformLocation(shader_program.getProgramID(), uniform.c_str()) };
        shader_program.activate();
        glUniform1i(tex_uni, unit);
        return *this;
    }

    //-----------------
}