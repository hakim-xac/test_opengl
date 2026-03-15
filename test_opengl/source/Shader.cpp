#include "../include/Shader.h"

namespace kas::gui
{
    //-----------------

    Shader::Shader(const std::string& filename, GLuint gl_shader_type) : m_id{}
    {
        const auto shader_code_opt{ utils::getFileContents(filename) };
        if (!shader_code_opt) [[unlikely]]
            throw std::runtime_error{ "Can not open filename: " + filename };

        const char* shader_code_source{ shader_code_opt.value().c_str() };
        m_id = glCreateShader(gl_shader_type);
        if (m_id == 0) [[unlikely]]
            throw std::runtime_error{ "Can not greate Shader from file: " + filename };

        glShaderSource(m_id, 1, &shader_code_source, NULL);
        glCompileShader(m_id);

        GLint success{};
        glGetShaderiv(m_id, GL_COMPILE_STATUS, &success);
        if (!success) {
            char infoLog[512];
            glGetShaderInfoLog(m_id, 512, NULL, infoLog);
            glDeleteShader(m_id);
            throw std::runtime_error{ "Shader compilation failed for " + filename + ": " + infoLog };
        }

        std::cout << "sh ctor: " << m_id << " type: " << gl_shader_type << "\n";
    }

    //-----------------

    [[nodiscard]]
    GLuint
    Shader::getShaderID() const& noexcept
    {
        return m_id;
    }

    //-----------------

    Shader::~Shader()
    {
        std::cout << "sh dtor: " << m_id << "\n";
        if (m_id != 0)
            glDeleteShader(m_id);
    }

    //-----------------

    Shader::Shader(Shader&& other) noexcept :
        m_id{ std::move(other.m_id) }
    {
    }

    //-----------------

    Shader& Shader::operator=(Shader&& other) noexcept
    {
        if (std::addressof(other) != this)
            m_id = std::move(other.m_id);
        return *this;
    }

    //-----------------
}