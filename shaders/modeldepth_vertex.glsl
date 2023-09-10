#version 330 core
layout (location = 0) in vec3 vPos;

uniform mat4 model;
uniform mat4 camMatrix;

void main() {
	vec3 wPos = vec3(model * vec4(vPos, 1.0f));
	gl_Position = camMatrix * vec4(wPos, 1.0);
}