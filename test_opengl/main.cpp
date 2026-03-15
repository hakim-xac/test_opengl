#include <glad/glad.h> // glad должен включатьс€ первым
#include <GLFW/glfw3.h>
#include <iostream>
#include <array>

#include "include/Utils.h"
#include "include/Shader.h"
#include "include/ShaderProgram.h"
#include "include/Texture.h"
#include "include/VertexArrayObject.h"
#include "include/BufferObjects.h"

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
    static bool is_stretch{ true };
    //// шаг трансформации
    const GLfloat step{ 0.005f };
    if (is_stretch)
    {
        sp[2 * 8] += step;
        sp[3 * 8] -= step;
        if (sp[2 * 8] >= 0.8f)
            is_stretch = false;
    }
    else
    {
        sp[2 * 8] -= step;
        sp[3 * 8] += step;
        if (sp[2 * 8] <= 0.3f)
            is_stretch = true;
    }

    //// обновление данных в буфере
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 2 * 8, sizeof(GLfloat), &sp[2 * 8]);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 3 * 8, sizeof(GLfloat), &sp[3 * 8]);
}

void handler()
{
    int res_gl_init{ -1 };
    kas::gui::utils::Guard gl_init{ [&res_gl_init] { res_gl_init = glfwInit(); }, [] { glfwTerminate(); } };
    if (res_gl_init == GLFW_FALSE) [[unlikely]]
    {
        std::cerr << "GLFW_FALSE\n";
        return;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Raii
    std::unique_ptr < GLFWwindow, decltype([](GLFWwindow* window) { glfwDestroyWindow(window); }) > window_uptr
    {
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

    std::vector<kas::gui::Shader> shaders{};
    shaders.reserve(2);
    shaders.emplace_back("shaders/default.vert", GL_VERTEX_SHADER);
    shaders.emplace_back("shaders/default.frag", GL_FRAGMENT_SHADER);

    kas::gui::ShaderProgram prog{ std::move(shaders) };
    std::cout << "before shader program\n";

    const auto tex{ 
        kas::gui::Texture::getTexture(
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

    const kas::gui::VertexArrayObject vao{};

    const kas::gui::BufferObjects<kas::gui::gl_types::gl_array_buffer_t> vbo { vertices };
    const kas::gui::BufferObjects<kas::gui::gl_types::gl_element_array_buffer_t> ebo { indexes };

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
        const double curr_time{ glfwGetTime() };
        if (curr_time - prev_time >= 0.01)
        {
            // скажем OpenGL использовать VBO
            vbo.bind();
            // ѕроизедем шаг трансформации
            transform(vertices);
            
            // обновим данные в VBO
            vao.linkAttrib(vbo, 0, 3, GL_FLOAT, 8 * sizeof(float), (void*)0);
            // обновим врем€
            prev_time = curr_time;
        }

        tex.setupUnit(prog, "tex0", 0);
        tex.bind();

        vao.bind();

        glDrawElements(GL_TRIANGLE_FAN, 4, GL_UNSIGNED_INT, 0);
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
}