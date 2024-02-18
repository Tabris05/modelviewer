#version 460 core

in vec2 fUV;

uniform float gamma;
uniform float exposure2;
uniform sampler2D inputTex;

out vec3 FragColor;

vec3 toneMap(vec3 inputColor) {
	float oldLuminance = dot(inputColor, vec3(0.2126f, 0.7152f, 0.0722f));
	float newLuminance = oldLuminance * (1.0f + oldLuminance / exposure2) / (1.0f + oldLuminance);
	return inputColor * (newLuminance / oldLuminance);
}

vec3 gammaCorrect(vec3 inputColor) {
	return pow(inputColor, vec3(1.0f / gamma));
}

void main() {
	vec3 color = texture(inputTex, fUV).rgb;
	color = toneMap(color);
	color = gammaCorrect(color);
	FragColor = color;
}