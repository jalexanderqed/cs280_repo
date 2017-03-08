#version 330 core
layout (location = 0) out vec4 gPosition;

out vec4 color;
in vec4 worldFragPos;

void main()
{
    gPosition = worldFragPos;
} 