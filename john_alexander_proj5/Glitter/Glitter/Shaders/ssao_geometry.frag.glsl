#version 330 core
out vec4 color;

in vec2 TexCoords;
in vec3 Normal;
in vec4 worldFragPos;
in vec4 viewFragPos;

uniform sampler2D positionTex;

uniform vec3 probes[30];
uniform mat4 projection;

int numProbes = 30;
float radius = 2.0f;

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

void main()
{
	vec3 norm = normalize(Normal);
	color = vec4(vec3(calcOcclusion(norm)), 1.0f);
}