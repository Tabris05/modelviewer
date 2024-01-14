#version 330 core

in vec2 fTexCoord;

out vec2 FragColor;

#define PI 3.141593f

vec2 hammersley(uint i, uint N) {
	uint bits = i;
	bits = (bits << 16u) | (bits >> 16u);
	bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
	bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
	bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
	bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
	return vec2(float(i) / float(N), float(bits) * 2.3283064365386963e-10);
}

vec3 importanceSampleGGX(vec2 Xi, vec3 normal, float roughness) {
	float a = roughness * roughness;

	float phi = 2.0 * PI * Xi.x;
	float cosTheta = sqrt((1.0 - Xi.y) / (1.0 + (a * a - 1.0) * Xi.y));
	float sinTheta = sqrt(1.0 - cosTheta * cosTheta);
	vec3 H = vec3(cos(phi) * sinTheta, sin(phi) * sinTheta, cosTheta);

	vec3 up = abs(normal.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
	vec3 tangent = normalize(cross(up, normal));
	vec3 bitangent = cross(normal, tangent);

	vec3 sampleVec = tangent * H.x + bitangent * H.y + normal * H.z;
	return normalize(sampleVec);
}

float geometrySmith(vec3 normal, vec3 viewDir, vec3 lightDir, float roughness) {
	float k = roughness * roughness / 2.0f;
	float nDotV = max(dot(normal, viewDir), 0.0f);
	float nDotL = max(dot(normal, lightDir), 0.0f);
	float ggx1 = nDotV / (nDotV * (1.0f - k) + k);
	float ggx2 = nDotL / (nDotL * (1.0f - k) + k);
	return ggx1 * ggx2;
}

void main() {
	const uint sampleCount = 1024u;
	vec3 V = vec3(1.0f - fTexCoord.x * fTexCoord.x, 0.0f, fTexCoord.x);
	vec3 normal = vec3(0.0f, 0.0f, 1.0f);
	vec2 result = vec2(0.0f);

	for (uint i = 0u; i < sampleCount; i++) {
		vec2 Xi = hammersley(i, sampleCount);
		vec3 H = importanceSampleGGX(Xi, normal, fTexCoord.y);
		vec3 L = normalize(2.0f * dot(V, H) * H - V);

		float nDotL = max(L.z, 0.0f);

		if (nDotL > 0.0f) {
			float vDotH = max(dot(V, H), 0.0f);
			float gVis = (geometrySmith(normal, V, L, fTexCoord.y) * vDotH) / (max(H.z, 0.0f) * fTexCoord.x);
			float fC = pow(1.0f - vDotH, 5.0f);

			result += vec2((1.0f - fC) * gVis, fC * gVis);
		}
	}
	result /= sampleCount;
	FragColor = result;
}