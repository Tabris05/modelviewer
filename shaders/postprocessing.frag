#version 460 core
#extension GL_ARB_bindless_texture : require

in vec2 fUV;

uniform float gamma;
layout(bindless_sampler) uniform sampler2D inputTex;

out vec3 FragColor;

vec3 toneMap(vec3 inputColor, float exposure2) {
	float oldLuminance = dot(inputColor, vec3(0.2126f, 0.7152f, 0.0722f));
	float newLuminance = oldLuminance * (1.0f + oldLuminance / exposure2) / (1.0f + oldLuminance);
	return inputColor * (newLuminance / oldLuminance);
}

vec3 gammaCorrect(vec3 inputColor) {
	return pow(inputColor, vec3(1.0f / gamma));
}

void main() {
	vec3 color = texture(inputTex, fUV).rgb;
	color = toneMap(color, 25.0f);
	color = gammaCorrect(color);
	FragColor = color;
}