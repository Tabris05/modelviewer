#version 460 core

out vec2 fUV;

void main() {
	fUV = vec2((gl_VertexID << 1) & 2, gl_VertexID & 2);
	gl_Position = vec4(fUV * 2.0f - 1.0f, 0.0f, 1.0f);
}