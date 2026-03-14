#version 330 core

out vec4 FragColor;

// Ввод цвета из вершинного шейдера
in vec3 color;

// Ввод координат текстуры из вершинного шейдера
in vec2 texCoord;

// Получаем единицу тексnуры из основной функции
uniform sampler2D tex0;

void main()
{
    FragColor = texture(tex0, texCoord);
}