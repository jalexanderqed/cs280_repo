#version 330 core
out vec4 color;

struct PointLight{
	vec3 position;
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

uniform samplerCube shadowMap0;
uniform samplerCube shadowMap1;
uniform samplerCube shadowMap2;

uniform sampler2D positionTex;
uniform sampler2D texture_diffuse1;
uniform sampler2D texture_normal1;
uniform sampler2D texture_specular1;
uniform float farClip;

uniform PointLight pointLights[3];
uniform vec3 pPos;

in vec2 TexCoords;
in vec3 Normal;
in vec4 worldFragPos;
in vec4 viewFragPos;

uniform vec3 probes[15];
uniform mat4 projection;

int numProbes = 15;
float radius = 2.0f;

float ambientStrength = 0.3f;

float calcOcclusion(vec3 norm){
	vec3 randomVec = normalize(vec3(probes[int(gl_FragCoord.x * gl_FragCoord.y * 1103515245 + 12345) % numProbes].xy, 0));
	vec3 tan = normalize(randomVec - norm * dot(randomVec, norm));
	vec3 bitan = cross(norm, tan);
	mat3 tanTransform = mat3(tan, bitan, norm);
	
	float res = 0;
	for(int i = 0; i < numProbes; i++){
		vec3 probePos = tanTransform * probes[i];
		probePos = viewFragPos.xyz + probePos * radius;
		vec4 projProbePos = projection * vec4(probePos, 1.0f);
		projProbePos /= projProbePos.w;
		projProbePos.xyz = projProbePos.xyz * 0.5f + 0.5f;
		float depth = texture(positionTex, projProbePos.xy).z;
		res += (depth >= probePos.z + .1 ? 1.0 : 0.0);
	}
	return 1 - (res / numProbes);
}

vec3 pointLightContrib(PointLight light, samplerCube shadowMap, vec3 normal, vec3 eyeDir, vec3 wFragPos){
	vec3 outDir = normalize(light.position - wFragPos);
	vec3 inDir = -outDir;
	float diffuse = max(dot(normal, outDir), 0);

	vec3 reflected = reflect(inDir, normal);
	float specular = pow(max(dot(eyeDir, reflected), 0), 1.5);

	float lightDist = length(light.position - wFragPos) * .05;
	float atten = 1.0f / (0.5f * lightDist + 0.5f * (lightDist * lightDist));

	vec3 diffColor = light.diffuse * diffuse * vec3(texture(texture_diffuse1, TexCoords));
	vec3 specColor = light.specular * specular * vec3(texture(texture_specular1, TexCoords));

	float shadowDepth = texture(shadowMap, inDir).r;
	shadowDepth *= farClip;
	float myDepth = length(wFragPos - light.position);
	float shadow = myDepth - 0.1f < shadowDepth ? 1.0f : 0.0f;

	return (diffColor + specColor) * atten * shadow;
}

void main()
{
	vec3 wFragPos = vec3(worldFragPos);
	vec3 norm = normalize(Normal);
	vec3 eyeDir = normalize(pPos - wFragPos);
	color = vec4(0, 0, 0, 0);

	color += vec4(pointLightContrib(pointLights[0], shadowMap0, norm, eyeDir, wFragPos), 1);
	color += vec4(pointLightContrib(pointLights[1], shadowMap1, norm, eyeDir, wFragPos), 1);
	color += vec4(pointLightContrib(pointLights[2], shadowMap2, norm, eyeDir, wFragPos), 1);
	color += ambientStrength * vec3(texture(texture_diffuse1, TexCoords)) * calcOcclusion(norm);
}