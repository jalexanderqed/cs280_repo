#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texCoords;

out vec2 TexCoords;
out vec3 Normal;
out vec4 worldFragPos;
out vec4 viewFragPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    worldFragPos = model * vec4(position, 1.0f);
	viewFragPos =  view * worldFragPos;
    gl_Position = projection * viewFragPos;
    TexCoords = texCoords;

    mat3 normalMatrix = transpose(inverse(mat3(model)));
    Normal = normalMatrix * normal;
}