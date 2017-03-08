#version 330 core
out vec4 color;

struct PointLight{
	vec3 position;
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

uniform sampler2D texture_diffuse1;
uniform sampler2D texture_normal1;
uniform sampler2D texture_specular1;
uniform float farClip;

uniform PointLight pointLights[3];
uniform vec3 pPos;

in vec2 TexCoordsG;
in vec3 NormalG;
in vec4 worldPos;

float ambientStrength = 0.3f;

vec3 pointLightContrib(PointLight light, vec3 normal, vec3 eyeDir, vec3 wFragPos){
	vec3 outDir = normalize(light.position - wFragPos);
	vec3 inDir = -outDir;
	float diffuse = max(dot(normal, outDir), 0);

	vec3 reflected = reflect(inDir, normal);
	float specular = pow(max(dot(eyeDir, reflected), 0), 1.5);

	float lightDist = length(light.position - wFragPos) * .05;
	float atten = 1.0f / (0.5f * lightDist + 0.5f * (lightDist * lightDist));

	vec3 diffColor = light.diffuse * diffuse * vec3(texture(texture_diffuse1, TexCoordsG));
	vec3 specColor = light.specular * specular * vec3(texture(texture_specular1, TexCoordsG));

	return (diffColor + specColor) * atten;
}

void main()
{
	vec3 wFragPos = vec3(worldPos);
	vec3 norm = normalize(NormalG);
	vec3 eyeDir = normalize(pPos - wFragPos);
	color = vec4(0, 0, 0, 0);

	color += vec4(pointLightContrib(pointLights[0], norm, eyeDir, wFragPos), 1);
	color += vec4(pointLightContrib(pointLights[1], norm, eyeDir, wFragPos), 1);
	color += vec4(pointLightContrib(pointLights[2], norm, eyeDir, wFragPos), 1);
	color += ambientStrength * vec3(texture(texture_diffuse1, TexCoordsG));
}