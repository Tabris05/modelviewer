#version 460 core

#define SHADOW_MAP_TEXEL_SIZE 1.0f / 2048.0f

layout (location = 0) in vec3 vPos;
layout (location = 1) in vec3 vNorm;

uniform mat4 modelMatrix;
uniform mat4 camMatrix;
uniform mat3 normalMatrix;

void main() {
	vec4 pos = modelMatrix * vec4(vPos, 1.0f);
	vec3 normal = normalize(normalMatrix * vNorm);
	pos -= vec4(normal * SHADOW_MAP_TEXEL_SIZE, 0.0f);

	gl_Position = camMatrix * pos;
}