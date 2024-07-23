#version 460 core
#extension GL_ARB_bindless_texture : require

in vec2 fUV;

uniform int inputMip;
layout(bindless_sampler) uniform sampler2D inputTex;

out vec3 fCol;

void main() {
	vec2 texelSize = 1.0f / textureSize(inputTex, inputMip);

	// MUST USE TEXTURE LOD
	// Bloom targets are all just mips of the same base texture so we only want bilinear filtering when taking these taps
	vec3 a = textureLod(inputTex, fUV + texelSize * vec2(-1.0f,  1.0f), inputMip).rgb;
	vec3 b = textureLod(inputTex, fUV + texelSize * vec2( 0.0f,  1.0f), inputMip).rgb;
	vec3 c = textureLod(inputTex, fUV + texelSize * vec2( 1.0f,  1.0f), inputMip).rgb;
	vec3 d = textureLod(inputTex, fUV + texelSize * vec2(-1.0f,  0.0f), inputMip).rgb;
	vec3 e = textureLod(inputTex, fUV + texelSize * vec2( 0.0f,  0.0f), inputMip).rgb;
	vec3 f = textureLod(inputTex, fUV + texelSize * vec2( 1.0f,  0.0f), inputMip).rgb;
	vec3 g = textureLod(inputTex, fUV + texelSize * vec2(-1.0f, -1.0f), inputMip).rgb;
	vec3 h = textureLod(inputTex, fUV + texelSize * vec2( 0.0f, -1.0f), inputMip).rgb;
	vec3 i = textureLod(inputTex, fUV + texelSize * vec2( 1.0f, -1.0f), inputMip).rgb;

	fCol = ((a + c + g + i) +  (b + d + f + h) * 2.0f  +  e * 4.0f) / 16.0f;
}