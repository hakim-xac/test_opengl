#include <glad/glad.h> // glad должен включаться первым
#include <GLFW/glfw3.h>
#include <iostream>
#include <fstream>
#include <string>
#include <string_view>
#include <functional>
#include <utility>
#include <memory>
#include <optional>
#include <vector>
#include <initializer_list>
#include <type_traits>
#include <span>
#include <array>

namespace utils
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

    [[nodiscard]]
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

    class Shader final {
    public:
        explicit Shader(const std::string& filename, GLuint gl_shader_type) : m_id{}
        {
            const auto shader_code_opt{ getFileContents(filename) };
            if (!shader_code_opt) [[unlikely]]
                throw std::runtime_error{ "Can not open filename: " + filename + "\n" };

            const char* shader_code_source{ shader_code_opt.value().c_str() };
            m_id = glCreateShader(gl_shader_type);
            if(m_id == 0) [[unlikely]]
                throw std::runtime_error{ "Can not greate Shader from file: " + filename + "\n" };

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
        [[nodiscard]] GLuint getShaderID() const& noexcept
        {
            return m_id;
        }

        ~Shader()
        {
            std::cout << "sh dtor: " << m_id << "\n";
            if(m_id != 0)
            glDeleteShader(m_id);
        }

        Shader(const Shader&) = delete;

        Shader(Shader&& other) noexcept :
            m_id{ std::move(other.m_id) }
        {
        }

        Shader& operator=(const Shader&) = delete;

        Shader& operator=(Shader&& other) noexcept
        {
            if (std::addressof(other) != this)
                m_id = std::move(other.m_id);
            return *this;
        }

    private:
        GLuint m_id;
    };

    class ShaderProgram final {
    public:
        explicit ShaderProgram(std::vector<Shader> shaders) :
            m_id{ glCreateProgram() }
        {
            if (m_id == 0) [[unlikely]]
                throw std::runtime_error{ "Can not greate Shader program!\n" };

            for (const auto& shader : shaders)
            {
                std::cout << "[shader_proggram] shader: " << shader.getShaderID() << "\n";
                glAttachShader(m_id, shader.getShaderID());
            }
            glLinkProgram(m_id);
        }

        void activate() const
        {
            glUseProgram(m_id);
        }

        ~ShaderProgram()
        {
            glDeleteProgram(m_id);
        }
        [[nodiscard]] GLuint getProgramID() const& noexcept
        {
            return m_id;
        }

    private:
        GLuint m_id;
    };

    namespace gl_types
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

    class GLuintGuard final
    {
    public:
        GLuintGuard() :
            m_id{}
        {
            glGenBuffers(1, &m_id);
        }
        ~GLuintGuard()
        {
            if (m_id)
                glDeleteBuffers(1, &m_id);
        }
        GLuintGuard(const GLuintGuard&) = delete;
        GLuintGuard(GLuintGuard&& other) noexcept :
            m_id{ std::move(other.m_id) }
        {
        }
        GLuintGuard& operator=(const GLuintGuard&) = delete;
        GLuintGuard& operator=(GLuintGuard&& other) noexcept
        {
            if (std::addressof(other) != this)
                m_id = std::move(other.m_id);
            return *this;
        }

        [[nodiscard]] GLuint getID() const noexcept
        {
            return m_id;
        }
    private:
        GLuint m_id;
    };

    template <typename TArrayBuffer>
    requires requires {
        typename TArrayBuffer::array_t;
        { TArrayBuffer::code } -> std::convertible_to<GLuint>;
    }
    class BufferObjects final
    {
    public:

        explicit BufferObjects(const std::span<const typename TArrayBuffer::array_t> buffer) :
            m_id{}
        {
            bind();
            glBufferData(TArrayBuffer::code, std::size(buffer) * sizeof(*std::data(buffer)), std::data(buffer), GL_DYNAMIC_DRAW);
        }

        void bind() const
        {
            glBindBuffer(TArrayBuffer::code, m_id.getID());
        }

        void unbind() const
        {
            glBindBuffer(TArrayBuffer::code, 0);
        }

        ~BufferObjects()
        {
            unbind();
        }
    private:
        GLuintGuard m_id;
    };

    class VertexArrayObject
    {
    public:
        VertexArrayObject() :
            m_id{}
        {
            glGenVertexArrays(1, &m_id);
            bind();
        }

        ~VertexArrayObject()
        {
            glDeleteVertexArrays(1, &m_id);
        }

        template <typename TBO>
        requires requires(TBO t) {
            t.bind();
            t.unbind();
        }
        void linkAttrib(const TBO& bo, GLuint layout, GLuint num_components, GLenum type, GLsizeiptr stride, void* offset)
        {
            Guard g{ [&bo = bo] { bo.bind();  }, [&bo = bo] { bo.unbind(); } };
            glVertexAttribPointer(layout, num_components, type, GL_FALSE, static_cast<GLsizei>(stride), offset);
            glEnableVertexAttribArray(layout);
        }

        void bind()
        {
            glBindVertexArray(m_id);
        }
        void unbind()
        {
            glBindVertexArray(0);
        }
    private:
        GLuint m_id;
    };
} // namespace utils

void handler();

int main()
{
    try
    {
        handler();
        return 0;
    }
    catch (const std::exception& ex)
    {
        std::cerr << "ex: " << ex.what() << "\n";
        return 2;
    }
}

void handler()
{
    int res_gl_init{ -1 };
    utils::Guard gl_init{ [&res_gl_init] { res_gl_init = glfwInit(); }, [] { glfwTerminate(); } };
    if (res_gl_init == GLFW_FALSE) [[unlikely]]
    {
        std::cerr << "GLFW_FALSE\n";
        return;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Raii
    std::unique_ptr < GLFWwindow, decltype([](GLFWwindow* window) { glfwDestroyWindow(window); }) > window_uptr{
        glfwCreateWindow(800, 800, "Simple Window", NULL, NULL)
    };

    auto window{ window_uptr.get() };
    if (!window)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        return;
    }

    glfwMakeContextCurrent(window);
    gladLoadGL();

    glViewport(0, 0, 800, 800);

    std::vector<utils::Shader> shaders{};
    shaders.reserve(2);
    shaders.emplace_back("shaders/default.vert", GL_VERTEX_SHADER);
    shaders.emplace_back("shaders/default.frag", GL_FRAGMENT_SHADER);

    utils::ShaderProgram prog{ std::move(shaders)};
    std::cout << "before shader program\n";

    utils::VertexArrayObject vao{};

    const std::array<GLfloat, 24> vertices{
        // COORDINATE / COLORS //
        -0.5f, 0.5f, 0.0f, 0.0f, 128 / 255.0f, 255 / 255.0f,
        0.5f, 0.5f, 0.0f, 255 / 255.0f, 0 / 255.0f, 0 / 255.0f,
        0.5f, -0.5f, 0.0f, 0 / 255.0f, 153 / 255.0f, 0 / 255.0f,
        -0.5f, -0.5f, 0.0f, 255 / 255.0f, 255 / 255.0f, 51 / 255.0f
    };
    const std::array<GLuint, 4> indexes{ 0, 1, 2, 3 };

    const utils::BufferObjects<utils::gl_types::gl_array_buffer_t> vbo { vertices };
    const utils::BufferObjects<utils::gl_types::gl_element_array_buffer_t> ebo { indexes };

    vao.linkAttrib(vbo, 0, 3, GL_FLOAT, 6 * sizeof(float), (void*)0);
    vao.linkAttrib(vbo, 1, 3, GL_FLOAT, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    
    vao.unbind();

    while (!glfwWindowShouldClose(window))
    {
        prog.activate();
        glClearColor(0.07f, 0.13f, 0.17f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        vao.bind();

        glDrawElements(GL_TRIANGLE_FAN, 4, GL_UNSIGNED_INT, 0);
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
}