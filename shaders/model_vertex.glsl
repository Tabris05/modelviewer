#version 330 core
layout(location = 0) in vec3 vPos;
layout(location = 1) in vec3 vNorm;
layout(location = 2) in vec3 vTangent;
layout(location = 3) in vec2 vTexCoord;

out vec2 fTexCoord;
out vec3 fFragPos;
out vec4 fFragPosLight;
out mat3 fTBN;

uniform mat4 model;
uniform mat4 camMatrix;
uniform mat4 lightMatrix;

void main() {
	vec3 wPos = vec3(model * vec4(vPos, 1.0f));

	vec3 T = normalize(vec3(model * vec4(vTangent, 0.0f)));
	vec3 N = normalize(vec3(model * vec4(vNorm, 0.0f)));
	T = normalize(T - dot(T, N) * N); 	// reorthogonalize T
	vec3 B = cross(N, T);


	fTBN = mat3(T, B, N);
	fFragPos = wPos;
	fTexCoord = vTexCoord;
	fFragPosLight = lightMatrix * vec4(wPos, 1.0f);

	gl_Position = camMatrix * vec4(wPos, 1.0f);
}