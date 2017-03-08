#version 330 core
in vec4 worldPos;

uniform vec3 lightPos;
uniform float farClip;

void main()
{
    float lightDist = length(worldPos.xyz - lightPos);
    lightDist = lightDist / farClip;
    gl_FragDepth = lightDist;
} 