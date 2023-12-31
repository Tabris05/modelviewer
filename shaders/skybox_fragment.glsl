#version 330 core

out vec3 FragColor;

in vec3 fTexCoord;

uniform samplerCube skybox;
uniform float fSunlightIntensity;

void main() {
	FragColor = texture(skybox, fTexCoord).rgb * vec3(fSunlightIntensity);
}