#version 330 core

out vec4 FragColor;

in vec3 fTexCoord;

uniform samplerCube skybox;
uniform float fSunlightIntensity;

void main() {
	FragColor = vec4(texture(skybox, fTexCoord).rgb * vec3(fSunlightIntensity), 1.0f);
}