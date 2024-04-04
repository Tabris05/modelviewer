#version 460 core

layout (location = 0) in vec3 vPos;
layout (location = 1) in vec3 vNorm;
layout (location = 2) in vec2 vUV;

out vec4 fPosLight;
out vec3 fPos;
out vec3 fNorm;
out vec2 fUV;

flat out int fMaterialIndex;

uniform mat4 camMatrix;
uniform mat4 modelMatrix;
uniform mat4 lightMatrix;
uniform mat3 normalMatrix;

void main() {
	vec3 wPos = vec3(modelMatrix * vec4(vPos, 1.0f));

	fUV = vUV;
	fPos = wPos;
	fPosLight = lightMatrix * vec4(wPos, 1.0f);
	fMaterialIndex = gl_BaseInstance; // using base instance to pass arbitrary data (material index)
	fNorm = normalize(normalMatrix * vNorm);
	gl_Position = camMatrix * vec4(wPos, 1.0f);
}