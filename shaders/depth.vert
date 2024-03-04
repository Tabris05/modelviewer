#version 460 core

layout (location = 0) in vec3 vPos;

uniform mat4 modelMatrix;
uniform mat4 camMatrix;

void main() {
	// removing these parentheses causes heavy visual artifacting on amd drivers
	gl_Position = camMatrix * (modelMatrix * vec4(vPos, 1.0f));
}