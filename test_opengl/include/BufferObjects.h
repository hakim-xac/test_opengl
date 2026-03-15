#pragma once
#include <glad/glad.h> // glad должен включаться первым
#include <type_traits>
#include <span>
#include <utility>
#include "Utils.h"

namespace kas::gui::concepts {
    template <typename TArrayBuffer>
    concept same_as_array_buffer = requires {
        typename TArrayBuffer::array_t;
        { TArrayBuffer::code } -> std::convertible_to<GLuint>;
    };
}

namespace kas::gui
{
    template <concepts::same_as_array_buffer TArrayBuffer>
    class BufferObjects final
    {
    public:
        explicit BufferObjects(const std::span<const typename TArrayBuffer::array_t> buffer);
        
        void bind() const;
        
        void unbind() const;

        ~BufferObjects();
    private:
        utils::GuardID<utils::guard_types::gl_uint_t> m_id;
    };
}

namespace kas::gui
{
    //-----------------

    template <concepts::same_as_array_buffer TArrayBuffer>
    BufferObjects<TArrayBuffer>::BufferObjects(const std::span<const typename TArrayBuffer::array_t> buffer) :
        m_id{}
    {
        bind();
        glBufferData(TArrayBuffer::code, std::size(buffer) * sizeof(*std::data(buffer)), std::data(buffer), GL_DYNAMIC_DRAW);
    }

    //-----------------

    template <concepts::same_as_array_buffer TArrayBuffer>
    void BufferObjects<TArrayBuffer>::bind() const
    {
        glBindBuffer(TArrayBuffer::code, m_id.getID());
    }

    //-----------------

    template <concepts::same_as_array_buffer TArrayBuffer>
    void BufferObjects<TArrayBuffer>::unbind() const
    {
        glBindBuffer(TArrayBuffer::code, 0);
    }

    //-----------------

    template <concepts::same_as_array_buffer TArrayBuffer>
    BufferObjects<TArrayBuffer>::~BufferObjects()
    {
        unbind();
    }

    //-----------------
}