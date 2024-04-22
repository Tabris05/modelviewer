#version 460 core
#extension GL_ARB_bindless_texture : require

#define KERNEL_SIZE 64
#define NOISE_SIZE 4

in vec2 fUV;

uniform mat4 projMatrix;
uniform mat4 invProjMatrix;
uniform vec3 kernel[KERNEL_SIZE];
layout(bindless_sampler) uniform sampler2D inputTex;
layout(bindless_sampler) uniform sampler2D noiseTex;

out float fCol;

const float radius = 0.5f;

vec3 derivePos(vec2 uv) {
	float depth = texture(inputTex, uv).r;

	vec4 ndc = vec4(uv * 2.0f - 1.0f, depth, 1.0f);
	vec4 viewSpace = invProjMatrix * ndc;

	return viewSpace.xyz / viewSpace.w;
}

void main() {
	vec2 scale = textureSize(inputTex, 0).xy / 2 / NOISE_SIZE;
	vec3 fragPos = derivePos(fUV);
	vec3 normal = normalize(-cross(dFdy(fragPos), dFdx(fragPos)));
	vec3 rotation = texture(noiseTex, fUV * scale).xyz;
	vec3 tangent = normalize(rotation - normal * dot(rotation, normal));
	vec3 bitangent = cross(normal, tangent);
	mat3 tbn = mat3(tangent, bitangent, normal);

	float occlusion = 0.0f;
	for(int i = 0; i < KERNEL_SIZE; i++) {
		vec3 samplePos = fragPos + tbn * kernel[i] * radius;
		vec4 samplePosNDC = projMatrix * vec4(samplePos, 1.0f);
		samplePosNDC.xy /= samplePosNDC.w;
		samplePosNDC.xy = samplePosNDC.xy * 0.5f + 0.5f;

		float sampleDepth = derivePos(samplePosNDC.xy).z;
		float rangeCheck = smoothstep(0.0, 1.0, radius / abs(fragPos.z - sampleDepth));
		occlusion += float(sampleDepth >= samplePos.z) * rangeCheck;
	}
	fCol = 1.0f - occlusion / KERNEL_SIZE;
}