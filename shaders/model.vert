#version 460 core

layout (location = 0) in vec3 vPos;
layout (location = 1) in vec3 vNorm;
layout (location = 2) in vec2 vUV;

out vec3 fPos;
out vec3 fNorm;
out vec2 fUV;

flat out int fDrawID;

uniform mat4 modelMatrix;
uniform mat4 camMatrix;

void main() {
	vec3 wPos = vec3(modelMatrix * vec4(vPos, 1.0f));

	fUV = vUV;
	fPos = wPos;
	fDrawID = gl_DrawID;
	fNorm = normalize(vec3(modelMatrix * vec4(vNorm, 0.0f)));
	gl_Position = camMatrix * vec4(wPos, 1.0f);
}