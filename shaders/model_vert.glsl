#version 460 core

layout (location = 0) in vec3 vPos;

uniform mat4 transform;

flat out int drawid;

void main() {
	drawid = gl_DrawID;
	gl_Position = transform * vec4(vPos, 1.0f);
}