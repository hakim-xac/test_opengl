#include "../include/ShaderProgram.h"

#include <iostream>
#include <stdexcept>


namespace kas::gui
{
    //-----------------

    ShaderProgram::ShaderProgram(std::vector<Shader> shaders) :
        m_id{ glCreateProgram() }
    {
        if (m_id == 0) [[unlikely]]
            throw std::runtime_error{ "Can not greate Shader program!" };

        for (const auto& shader : shaders)
        {
            std::cout << "[shader_proggram] shader: " << shader.getShaderID() << "\n";
            glAttachShader(m_id, shader.getShaderID());
        }
        glLinkProgram(m_id);
    }

    //-----------------

    void ShaderProgram::activate() const
    {
        glUseProgram(m_id);
    }

    //-----------------

    ShaderProgram::~ShaderProgram()
    {
        glDeleteProgram(m_id);
    }

    //-----------------

    [[nodiscard]]
    GLuint
    ShaderProgram::getProgramID() const& noexcept
    {
        return m_id;
    }

    //-----------------
}