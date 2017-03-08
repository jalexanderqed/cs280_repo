#version 330 core
layout (location = 0) in vec3 position;

uniform mat4 model;

out vec2 TexCoords;
out vec3 Normal;

void main()
{
	TexCoords = vec2(0);
	Normal = vec3(Normal);
    gl_Position = model * vec4(position, 1.0f);
}