#pragma once

#include <glad/glad.h> // glad должен включаться первым
#include <GLFW/glfw3.h>
#include <optional>
#include <string>
#include <utility>
#include <fstream>
#include <functional>

namespace kas::gui::utils
{
    template <typename AtDtor>
    class Guard final
    {
    public:
        template <typename AtCtor>
        explicit Guard(AtCtor&& at_ctor, AtDtor&& at_dtor) :
            m_at_dtor{ std::forward<AtDtor>(at_dtor) }
        {
            std::invoke(std::forward<AtCtor>(at_ctor));
        }

        Guard(const Guard&) = delete;
        Guard(Guard&& other) noexcept : m_at_dtor{ std::move(other.m_at_dtor) } {}
        Guard& operator=(const Guard&) = delete;
        Guard& operator=(Guard&& other) noexcept { if (std::addressof(other) != this) { std::swap(m_at_dtor, other.m_at_dtor); } return *this; }

        ~Guard()
        {
            std::invoke(m_at_dtor);
        }
    private:
        AtDtor m_at_dtor;
    };

    namespace guard_types {
        struct gl_uint_t {
            using type = GLuint;
            static inline const std::function<void(type&)> fctor{ [](type& id) { glGenBuffers(1, &id); } };
            static inline const std::function<void(type&)> fdtor{ [](type& id) { glDeleteBuffers(1, &id); } };
        };
        struct gl_texture_t {
            using type = GLuint;
            static inline const std::function<void(type&)> fctor{ [](type& id) { glGenTextures(1, &id); } };
            static inline const std::function<void(type&)> fdtor{ [](type& id) { glDeleteTextures(1, &id); } };
        };
    }
    template<typename GuardTypes>
        requires requires {
        typename GuardTypes::type;
        GuardTypes::fctor;
        GuardTypes::fdtor;
    }
    class GuardID final
    {
    public:
        GuardID() :
            m_id{}
        {
            std::invoke(GuardTypes::fctor, m_id);
        }
        ~GuardID()
        {
            std::invoke(GuardTypes::fdtor, m_id);
        }
        [[nodiscard]] GLuint getID() const { return m_id; }
    private:
        GLuint m_id;
    };

    [[nodiscard]] inline
    std::optional<std::string>
    getFileContents(const std::string& filename)
    {
        std::ifstream in{ filename, std::ios::binary };
        if (!in.is_open()) [[unlikely]]
            return std::nullopt;

        std::string contents;
        in.seekg(0, std::ios::end);
        contents.resize(in.tellg());
        in.seekg(0, std::ios::beg);
        in.read(&contents[0], contents.size());
        in.close();

        return contents;
    }
}