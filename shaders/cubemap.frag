#version 460 core
#extension GL_ARB_bindless_texture : require

in vec3 fPos;

layout(bindless_sampler) uniform samplerCube skyboxTex;

out vec3 FragColor;

void main() {
	FragColor = texture(skyboxTex, fPos).rgb;
}