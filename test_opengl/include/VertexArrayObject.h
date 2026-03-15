#pragma once
#include <glad/glad.h> // glad должен включаться первым
#include "Utils.h"
#include "GLTypes.h"

namespace kas::gui::concepts {
    template <typename TBO>
    concept same_as_buffer_object = requires(TBO t) {
        t.bind();
        t.unbind();
    };
}
namespace kas::gui
{
    class VertexArrayObject
    {
    public:
        VertexArrayObject();

        ~VertexArrayObject();

        void bind() const;
        void unbind() const;

        template <concepts::same_as_buffer_object TBO>
        const VertexArrayObject& linkAttrib(const TBO& bo, GLuint layout, GLuint num_components, GLenum type, GLsizeiptr stride, void* offset) const&;

    private:
        GLuint m_id;
    };
}

namespace kas::gui
{
    //-----------------

    template <concepts::same_as_buffer_object TBO>
    const VertexArrayObject& 
    VertexArrayObject::linkAttrib(const TBO& bo, GLuint layout, GLuint num_components, GLenum type, GLsizeiptr stride, void* offset) const&
    {
        utils::Guard g{ [&bo = bo] { bo.bind();  }, [&bo = bo] { bo.unbind(); } };
        glVertexAttribPointer(layout, num_components, type, GL_FALSE, static_cast<GLsizei>(stride), offset);
        glEnableVertexAttribArray(layout);
        return *this;
    }

    //-----------------
}