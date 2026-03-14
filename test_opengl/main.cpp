#include <glad/glad.h> // glad должен включатьс€ первым
#include <GLFW/glfw3.h>
#include <stb/stb_image.h>
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
                throw std::runtime_error{ "Can not open filename: " + filename };

            const char* shader_code_source{ shader_code_opt.value().c_str() };
            m_id = glCreateShader(gl_shader_type);
            if(m_id == 0) [[unlikely]]
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
                throw std::runtime_error{ "Can not greate Shader program!" };

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
        GuardID<guard_types::gl_uint_t> m_id;
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
        const VertexArrayObject& linkAttrib(const TBO& bo, GLuint layout, GLuint num_components, GLenum type, GLsizeiptr stride, void* offset) const&
        {
            Guard g{ [&bo = bo] { bo.bind();  }, [&bo = bo] { bo.unbind(); } };
            glVertexAttribPointer(layout, num_components, type, GL_FALSE, static_cast<GLsizei>(stride), offset);
            glEnableVertexAttribArray(layout);
            return *this;
        }

        void bind() const
        {
            glBindVertexArray(m_id);
        }
        void unbind() const
        {
            glBindVertexArray(0);
        }
    private:
        GLuint m_id;
    };

    class Texture final {
    public:
        [[nodiscard]] static Texture getTexture(const std::string& filename, GLenum type, GLenum slot, GLenum format, GLenum pixel_type)
        {
            return Texture{ filename, type, slot, format, pixel_type };
        }

        explicit Texture(const std::string& filename, GLenum type, GLenum slot, GLenum format, GLenum pixel_type) :
            m_id{},
            m_type{ type }
        {
            stbi_set_flip_vertically_on_load(true);

            int width{};
            int height{};
            int col_h{};

            std::unique_ptr < uint8_t, decltype([](uint8_t* raw_image_bytes) { stbi_image_free(raw_image_bytes); }) > raw_image_bytes_uptr {
                stbi_load(filename.c_str(), &width, &height, &col_h, 0)
            };

            const auto* bytes{ raw_image_bytes_uptr.get() };
            if(! bytes) [[unlikely]]
                throw std::runtime_error{ "Can not open texture: '" + filename };

            glActiveTexture(slot);
            Guard g{ [this] { glBindTexture(m_type, m_id.getID()); }, [this] { glBindTexture(m_type, 0); } };
            
            // Ќастраиваем тип алгоритма, который используетс€ дл€ уменьшени€ или увеличени€ изображени€
            glTexParameteri(m_type, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
            glTexParameteri(m_type, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            // Ќастраиваем способ повторени€ текстуры (если это вообще происходит)
            glTexParameteri(m_type, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(m_type, GL_TEXTURE_WRAP_T, GL_REPEAT);
            // ѕрисваиваем изображение объекту текстуры OpenGL
            glTexImage2D(m_type, 0, GL_RGBA, width, height, 0, format, pixel_type, bytes);
            // √енерируем MipMaps
            glGenerateMipmap(m_type);
        }

        void bind() const
        {
            glBindTexture(m_type, m_id.getID());
        }

        void unbind() const
        {
            glBindTexture(m_type, 0);
        }

        Texture& setupUnit(ShaderProgram& shader_program, const std::string& uniform, GLuint unit ) &
        {
            GLint tex_uni{ glGetUniformLocation(shader_program.getProgramID(), uniform.c_str()) };
            shader_program.activate();
            glUniform1i(tex_uni, unit);
            return *this;
        }

        const Texture& setupUnit(ShaderProgram& shader_program, const std::string& uniform, GLuint unit ) const &
        {
            GLint tex_uni{ glGetUniformLocation(shader_program.getProgramID(), uniform.c_str()) };
            shader_program.activate();
            glUniform1i(tex_uni, unit);
            return *this;
        }

    private:
        GuardID<guard_types::gl_texture_t> m_id;
        GLenum m_type;
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

void transform(const std::span<GLfloat> sp)
{
    // переменна€ дл€ направлени€ анимации
    static bool isStretch = true;
    //// шаг трансформации
    GLfloat step = 0.005;
    if (isStretch) {
        sp[2 * 8] += step;
        sp[3 * 8] -= step;
        if (sp[2 * 8] >= 0.7f)
            isStretch = false;
    }
    else {
        sp[2 * 8] -= step;
        sp[3 * 8] += step;
        if (sp[2 * 8] <= 0.5f)
            isStretch = true;
    }

    //// обновление данных в буфере
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 2 * 8, sizeof(GLfloat), &sp[2 * 8]);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 3 * 8, sizeof(GLfloat), &sp[3 * 8]);
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

    utils::ShaderProgram prog{ std::move(shaders) };
    std::cout << "before shader program\n";

    const auto tex{ utils::Texture::getTexture(
        "textures/Gravel_001_BaseColor.jpg", 
        GL_TEXTURE_2D,
        GL_TEXTURE0,
        GL_RGB,
        GL_UNSIGNED_BYTE )
    };

    std::array<GLfloat, 32> vertices{
        // COORDINATE / COLORS / TEXTURE COORDINATE
        -0.5f, 0.5f, 0.0f, 0.0f, 128 / 255.0f, 255 / 255.0f, 0.0f, 0.0f,
        0.5f, 0.5f, 0.0f, 255 / 255.0f, 0 / 255.0f, 0 / 255.0f, 0.0f, 1.0f,
        0.5f, -0.5f, 0.0f, 0 / 255.0f, 153 / 255.0f, 0 / 255.0f, 1.0f, 1.0f,
        -0.5f, -0.5f, 0.0f, 255 / 255.0f, 255 / 255.0f, 51 / 255.0f, 1.0f, 0.0f,
    };
    const std::array<GLuint, 4> indexes{ 0, 1, 2, 3 };

    const utils::VertexArrayObject vao{};

    const utils::BufferObjects<utils::gl_types::gl_array_buffer_t> vbo { vertices };
    const utils::BufferObjects<utils::gl_types::gl_element_array_buffer_t> ebo { indexes };

    vao.linkAttrib(vbo, 0, 3, GL_FLOAT, 8 * sizeof(float), (void*)0);
    vao.linkAttrib(vbo, 1, 3, GL_FLOAT, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    vao.linkAttrib(vbo, 2, 2, GL_FLOAT, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    
    vao.unbind();

    double prev_time{ glfwGetTime() };

    while (!glfwWindowShouldClose(window))
    {
        glClearColor(0.07f, 0.13f, 0.17f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        prog.activate();

        // анимаци€ c шагом в 0.01 секунды
        double currTime = glfwGetTime();
        if (currTime - prev_time >= 0.01)
        {
            // скажем OpenGL использовать VBO
            vbo.bind();
            // ѕроизедем шаг трансформации
            transform(vertices);
            
            // обновим данные в VBO
            vao.linkAttrib(vbo, 0, 3, GL_FLOAT, 8 * sizeof(float), (void*)0);
            // обновим врем€
            prev_time = currTime;
        }

        tex.setupUnit(prog, "tex0", 0);
        tex.bind();

        vao.bind();

        glDrawElements(GL_TRIANGLE_FAN, 4, GL_UNSIGNED_INT, 0);
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
}