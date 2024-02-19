#version 460 core
#extension GL_ARB_bindless_texture : require

#define PI 3.141593f
#define EPSILON 0.000001f

struct Material {
	vec4 baseColor;
	vec4 metallicRoughness;
	sampler2D albedoMap;
	sampler2D normalMap;
	sampler2D occlusionMap;
	sampler2D metallicRoughnessMap;
};

in vec3 fPos;
in vec3 fNorm;
in vec2 fUV;

flat in int fDrawID;

uniform vec3 camPos;
uniform vec3 lightAngle;
uniform vec3 lightColor;
uniform float lightIntensity;

layout(std430, binding = 0) readonly buffer MaterialBuffer {
	Material materials[];
};

layout(std430, binding = 1) readonly buffer MaterialIndexBuffer {
	uint materialIndices[];
};

out vec3 fCol;

float clampedDot(vec3 a, vec3 b) {
	return max(dot(a, b), 0.0f);
}

mat3 makeTBN(vec3 N, vec3 invViewDir, vec2 uv) {
	vec3 dvdx = dFdx(invViewDir);
	vec3 dvdy = dFdy(invViewDir);
	vec2 dudx = dFdx(uv);
	vec2 dudy = dFdy(uv);
	vec3 dvdxPerp = cross(N, dvdx);
	vec3 dvdyPerp = cross(dvdy, N);
	vec3 T = dvdyPerp * dudx.x + dvdxPerp * dudy.x;
	vec3 B = dvdyPerp * dudx.y + dvdxPerp * dudy.y;
	float invmax = inversesqrt(max(dot(T, T), dot(B, B)));
	return mat3(T * invmax, B * invmax, N);
}

float isotrophicNDFFilter(vec3 normal, float roughness) {
	const float SIGMA2 = 0.15915494f;
	const float KAPPA = 0.18f;

	float roughness2 = roughness * roughness;
	vec3 dndu = dFdx(normal);
	vec3 dndv = dFdy(normal);
	float kernalRoughness2 = 2.0f * SIGMA2 * (dot(dndu, dndu) + dot(dndv, dndv));
	kernalRoughness2 = min(kernalRoughness2, KAPPA);
	return sqrt(clamp(roughness2 + kernalRoughness2, 0.0f, 1.0f));
}

float ggxNDF(vec3 normal, vec3 halfway, float alpha) {
	float alpha2 = alpha * alpha;
	float nDotH = clampedDot(normal, halfway);
	float denom = nDotH * nDotH * (alpha2 - 1.0f) + 1.0f;
	return alpha2 / (PI * denom * denom);
}

float geometrySmith(vec3 normal, vec3 viewDir, vec3 lightAngle, float roughness) {
	float alpha = roughness + 1;
	float k = alpha * alpha / 8.0f;
	float nDotV = clampedDot(normal, viewDir);
	float nDotL = clampedDot(normal, lightAngle);
	float gSub1 = nDotV / (nDotV * (1.0f - k) + k);
	float gSub2 = nDotL / (nDotL * (1.0f - k) + k);
	return gSub1 * gSub2;
}

vec3 fresnelSchlick(vec3 halfway, vec3 viewDir, vec3 F0) {
	return F0 + (1.0f - F0) * pow(clamp(1.0f - clampedDot(halfway, viewDir), 0.0f, 1.0f), 5.0f);
}

vec3 directionalLight(vec3 viewDir, vec3 albedo, vec3 normal, float metalness, float roughness) {
	if(lightColor == vec3(0.0f, 0.0f, 0.0f)) return vec3(0.0f, 0.0f, 0.0f);
	vec3 halfway = normalize(viewDir + lightAngle);

	vec3 F0 = mix(vec3(0.04f), albedo, metalness);

	float distribution = ggxNDF(normal, halfway, roughness * roughness);
	float geometry = geometrySmith(normal, viewDir, lightAngle, roughness);
	vec3 fresnel = fresnelSchlick(halfway, viewDir, F0);

	vec3 numerator = distribution * geometry * fresnel;
	float denominator = 4.0f * clampedDot(normal, viewDir) * clampedDot(normal, lightAngle) + EPSILON;
	
	vec3 specular = numerator / denominator;
	vec3 diffuse = (vec3(1.0f) - fresnel) * (1.0f - metalness) * albedo / PI;
	return (diffuse + specular) * clampedDot(normal, lightAngle) * lightColor * lightIntensity;
}

vec3 ambientLight(vec3 albedo, float occlusion) {
	return vec3(0.05f) * albedo * occlusion;
}
 
void main() {
	uint i = materialIndices[fDrawID];

	vec3 viewDir = normalize(camPos - fPos);
	
	vec4 materialColor = texture(materials[i].albedoMap, fUV) * materials[i].baseColor;
	vec3 normal = normalize(fNorm);
	normal = normalize(makeTBN(normal, -viewDir, fUV) * (texture(materials[i].normalMap, fUV).rgb * 2.0f - 1.0f));
	
	float occlusion = texture(materials[i].occlusionMap, fUV).r;
	float metalness = texture(materials[i].metallicRoughnessMap, fUV).b * materials[i].metallicRoughness.b;
	float roughness = texture(materials[i].metallicRoughnessMap, fUV).g * materials[i].metallicRoughness.g;
	roughness = isotrophicNDFFilter(normal, roughness);
	
	fCol = directionalLight(viewDir, materialColor.rgb, normal, metalness, roughness) + ambientLight(materialColor.rgb, occlusion);
}
