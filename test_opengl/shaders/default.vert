#version 330 core

// Координаты
layout (location = 0) in vec3 aPos;

// Цвет
layout (location = 1) in vec3 aColor;

// Координаты текстуры
layout (location = 2) in vec2 aTex;

// Вывод цвета для фрагментного шейдера
out vec3 color;

// Вывод координат тектсуры для фрагментного шейдера
out vec2 texCoord;
void main()
{
    // Получаем позиции из массива вершин
    gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);
    
    // Получаем цвета из массива вершин
    color = aColor;

    // Получаем координаты текстуры
    texCoord = aTex;
}