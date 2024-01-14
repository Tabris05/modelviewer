#version 330 core
layout (location = 0) in vec3 vPos;

out vec3 fPos;

uniform mat4 camMatrix;

void main() {
	fPos = vPos;
	gl_Position = camMatrix * vec4(vPos, 1.0f);
}