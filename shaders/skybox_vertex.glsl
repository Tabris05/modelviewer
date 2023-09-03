#version 330 core
layout(location = 0) in vec3 vPos;

out vec3 fTexCoord;

uniform mat4 camMatrix;

void main() {
	fTexCoord = vPos;
	gl_Position = (camMatrix * vec4(vPos, 1.0f)).xyww;
}