#version 330 core
layout(triangles) in;
layout(triangle_strip, max_vertices=18) out;

uniform mat4 shadowMat[6];

in vec3 Normal[];
in vec2 TexCoords[];

out vec2 TexCoordsG;
out vec3 NormalG;

out vec4 worldPos;

void main(){
	for(int i = 0; i < 6; i++){
		gl_Layer = i;
		for(int j = 0; j < 3; j++){
			worldPos = gl_in[j].gl_Position;
			gl_Position = shadowMat[i] * worldPos;
			NormalG = Normal[i];
			TexCoordsG = TexCoords[i];
			EmitVertex();
		}
		EndPrimitive();
	}
}