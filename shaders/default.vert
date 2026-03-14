#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;
// Вывод цвета для фрагментного шейдера
out vec3 color;
void main()
{
    // Получаем позиции из массива вершин
    gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);
    // Получаем цвета из массива вершин
    color = aColor;
}