#version 330 core

in vec2 fTexCoord;
out vec3 FragColor;

uniform sampler2D fTexID;
uniform float fGamma;
uniform float fWhiteLevel;

vec3 toneMap(vec3 inputVal, float whiteLevel) {
	float oldLuminance = dot(inputVal, vec3(0.212f, 0.7152f, 0.0722f));
	return inputVal * (oldLuminance * (1.0f + (oldLuminance / (whiteLevel * whiteLevel)))) / (1.0f + oldLuminance) / oldLuminance;
}

vec3 gammaCorrect(vec3 inputVal) {
	return pow(inputVal, vec3(1.0f/fGamma));
}

void main() {
	vec3 color = texture(fTexID, fTexCoord).rgb;
	color = toneMap(color, fWhiteLevel);
	color = gammaCorrect(color);
	FragColor = color;
}
