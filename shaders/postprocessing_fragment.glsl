#version 330 core

in vec2 fTexCoord;
out vec3 FragColor;

uniform sampler2D fTexID;
uniform float fGamma;
uniform float fSunlightIntensity;

float luminance(vec3 input) {
	return dot(input, vec3(0.212f, 0.7152f, 0.0722f));
}

vec3 changeLuminance(vec3 color, float newLuminance) {
	float oldLuminance = luminance(color);
	return color * (newLuminance / oldLuminance);
}

vec3 toneMap(vec3 input, float whiteLevel) {
	float oldLuminance = luminance(input);
	return changeLuminance(input, (oldLuminance * (1.0f + (oldLuminance / (whiteLevel * whiteLevel)))) / (1.0f + oldLuminance));
}

vec3 gammaCorrect(vec3 input) {
	return pow(input, vec3(1.0f/fGamma));
}

void main() {
	FragColor = texture(fTexID, fTexCoord).rgb;
	FragColor = toneMap(FragColor, fSunlightIntensity);
	FragColor = gammaCorrect(FragColor);
}
