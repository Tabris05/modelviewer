#version 460 core
#extension GL_ARB_bindless_texture : require

#define EPSILON 0.000001f

in vec2 fUV;

uniform int inputMip;

layout(bindless_sampler) uniform sampler2D inputTex;

out vec3 fCol;

float karis(vec3 inputColor) {
	return 1.0f / (1.0f + dot(inputColor, vec3(0.2126f, 0.7152f, 0.0722f)));
}

vec3 fireflyFilter(vec3 group1, vec3 group2, vec3 group3, vec3 group4, vec3 group5) {
	return group1 * karis(group1) + group2 * karis(group2) + group3 * karis(group3) + group4 * karis(group4) + group5 * karis(group5);
}

// apparently its not physically based to threshold bloom,
// but I don't think its very physically plausible for wood to bloom under an average brightness light so I'm doing it anyways
vec3 threshold(vec3 color) {
	float brightness = max(color.r, max(color.g, color.b));
	float contribution = max(0.0f, brightness - 1.0f) / max(brightness, EPSILON);
	return color * contribution;
}

void main() {
	vec2 texelSize = 1.0f / textureSize(inputTex, inputMip);

	// MUST USE TEXTURE LOD
	// Bloom targets are all just mips of the same base texture so we only want bilinear filtering when taking these taps
	vec3 a = textureLod(inputTex, fUV + texelSize * vec2(-2.0f,  2.0f), inputMip).rgb;
	vec3 b = textureLod(inputTex, fUV + texelSize * vec2( 0.0f,  2.0f), inputMip).rgb;
	vec3 c = textureLod(inputTex, fUV + texelSize * vec2( 2.0f,  2.0f), inputMip).rgb;
	vec3 d = textureLod(inputTex, fUV + texelSize * vec2(-2.0f,  0.0f), inputMip).rgb;
	vec3 e = textureLod(inputTex, fUV + texelSize * vec2( 0.0f,  0.0f), inputMip).rgb;
	vec3 f = textureLod(inputTex, fUV + texelSize * vec2( 2.0f,  0.0f), inputMip).rgb;
	vec3 g = textureLod(inputTex, fUV + texelSize * vec2(-2.0f, -2.0f), inputMip).rgb;
	vec3 h = textureLod(inputTex, fUV + texelSize * vec2( 0.0f, -2.0f), inputMip).rgb;
	vec3 i = textureLod(inputTex, fUV + texelSize * vec2( 2.0f, -2.0f), inputMip).rgb;
	vec3 j = textureLod(inputTex, fUV + texelSize * vec2(-1.0f,  1.0f), inputMip).rgb;
	vec3 k = textureLod(inputTex, fUV + texelSize * vec2( 1.0f,  1.0f), inputMip).rgb;
	vec3 l = textureLod(inputTex, fUV + texelSize * vec2(-1.0f, -1.0f), inputMip).rgb;
	vec3 m = textureLod(inputTex, fUV + texelSize * vec2( 1.0f, -1.0f), inputMip).rgb;

	switch(inputMip) {
		case 0:
			a = threshold(a);
			b = threshold(b); c = threshold(c); d = threshold(d); e = threshold(e);
			f = threshold(f); g = threshold(g); h = threshold(h); i = threshold(i);
			j = threshold(j); k = threshold(k); l = threshold(l); m = threshold(m);
			fCol = fireflyFilter(
				(a + b + d + e) * 0.03125f,
				(b + c + e + f) * 0.03125f,
				(d + e + g + h) * 0.03125f,
				(e + f + h + i) * 0.03125f,
				(j + k + l + m) * 0.125f
			);
			break;
		default:
			fCol = (a + c + g + i) * 0.03125f + (b + d + f + h) * 0.0625f + (e + j + k + l + m) * 0.125f;
			break;
	}
}