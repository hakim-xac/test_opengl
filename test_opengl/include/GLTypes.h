#pragma once
#include <glad/glad.h> // glad должен включаться первым

namespace kas::gui::gl_types
{
    struct gl_array_buffer_t {
        using array_t = GLfloat;
        static constexpr GLuint code{ GL_ARRAY_BUFFER };
    };
    struct gl_element_array_buffer_t {
        using array_t = GLuint;
        static constexpr GLuint code{ GL_ELEMENT_ARRAY_BUFFER };
    };
}