#version 460 core
#define PI 3.141593f
#define EPSILON 0.000001f

in vec3 fPos;

uniform samplerCube skyboxTex;
uniform float resolution;
uniform float roughness;

out vec3 fCol;

vec2 hammersley(uint i, uint N) {
    uint bits = i;
    bits = (bits << 16u) | (bits >> 16u);
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
    return vec2(float(i)/float(N), float(bits) * 2.3283064365386963e-10);
}

float distributionGGX(vec3 normal, vec3 halfway, float roughness) {
    float alpha = roughness * roughness;
    float alphaSquared = alpha * alpha;
    float nDotH = max(dot(normal, halfway), 0.0f);
    float nDotHSquared = nDotH * nDotH;
    float bottomTerm = nDotHSquared * (alphaSquared - 1.0f) + 1.0f;
    return alphaSquared / (PI * bottomTerm * bottomTerm);
}

vec3 importanceSampleGGX(vec2 Xi, vec3 normal, float roughness) {
    float a = roughness * roughness;
	
    float phi = 2.0 * PI * Xi.x;
    float cosTheta = sqrt((1.0 - Xi.y) / (1.0 + (a*a - 1.0) * Xi.y));
    float sinTheta = sqrt(1.0 - cosTheta*cosTheta);
    vec3 H = vec3(cos(phi) * sinTheta, sin(phi) * sinTheta, cosTheta);

    vec3 up = abs(normal.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
    vec3 tangent = normalize(cross(up, normal));
    vec3 bitangent = cross(normal, tangent);
	
    vec3 sampleVec = tangent * H.x + bitangent * H.y + normal * H.z;
    return normalize(sampleVec);
} 

void main() {
    vec3 normal = normalize(fPos);
    const uint sampleCount = 1024u;
    float totalWeight = 0.0f;
    vec3 prefilterColor = vec3(0.0f);
    for(uint i = 0u; i < sampleCount; i++) {
        vec2 Xi = hammersley(i, sampleCount);
        vec3 H = importanceSampleGGX(Xi, normal, roughness);
        vec3 L = normalize(2.0f * dot(normal, H) * H - normal);
        float nDotL = max(dot(normal, L), 0.0f);
        if(nDotL > 0.0f) {
            float pdf = distributionGGX(normal, H, roughness) * max(dot(normal, H), 0.0f) / (4.0f * max(dot(H, normal), 0.0f)) + EPSILON;
            float mip = roughness == 0.0f ? 0.0f : 0.5f * log2((1.0f / (sampleCount * pdf + EPSILON)) / (4.0f * PI / (6.0f * resolution * resolution)));
            prefilterColor += textureLod(skyboxTex, L, mip).rgb * nDotL;
            totalWeight += nDotL;
        }
    }
    fCol = prefilterColor / totalWeight;
}