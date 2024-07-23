#version 460 core
#extension GL_ARB_bindless_texture : require

// for lighting calculations
#define PI 3.141593f
#define EPSILON 0.000001f

// for shadow map filtering
#define WINDOWSIZE 8
#define FILTERSIZE 16
#define SAMPLERADIUS 4.0f / 2048.0f

// for material texture bitfield
#define HAS_ALBEDO 0x01u
#define HAS_NORMAL 0x02u
#define HAS_METALLIC_ROUGHNESS 0x04u
#define HAS_OCCLUSION 0x08u
#define HAS_EMISSIVE 0x10u

struct Material {
	vec4 baseColor;
	vec4 emissiveColor;
	sampler2D albedoMap;
	sampler2D normalMap;
	sampler2D occlusionMap;
	sampler2D metallicRoughnessMap;
	sampler2D emissiveMap;
	float metalness;
	float roughness;
	uint textureBitfield;
	uint _padding[3];
};

in vec4 fPosLight;
in vec3 fPos;
in vec3 fTan;
in vec3 fBitan;
in vec3 fNorm;
in vec2 fUV;

flat in int fMaterialIndex;

// direct lighting
uniform vec3 camPos;
uniform vec3 lightAngle;
uniform vec3 lightColor;
uniform float lightIntensity;

// shadow maps
layout(bindless_sampler) uniform sampler2D shadowmapTex;

// indirect lighting
uniform float maxMip;
layout(bindless_sampler) uniform samplerCube irradianceTex;
layout(bindless_sampler) uniform samplerCube envMapTex;
layout(bindless_sampler) uniform sampler2D brdfLUTex;

layout(std430, binding = 0) readonly buffer MaterialBuffer {
	Material materials[];
};

layout(std430, binding = 1) readonly buffer PoissonDiskBuffer {
	vec2 samples[];
};

out vec3 fCol;

float clampedDot(vec3 a, vec3 b) {
	return max(dot(a, b), 0.0f);
}

bool bitmaskGet(uint mask, uint value) {
	return bool(mask & value);
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

float inShadow() {
	float result = 0.0f;
	float bias = mix(0.02f, 0.0f, dot(fNorm, lightAngle));
	vec3 projectedPos = fPosLight.xyz / fPosLight.w;
	projectedPos.xy = projectedPos.xy * 0.5f + 0.5f;
	ivec3 offset = ivec3(0, ivec2(mod(gl_FragCoord.xy, ivec2(WINDOWSIZE))));
	for(int i = 0; i < FILTERSIZE; i++) {
		float cur = 0.0f;
		for(int j = 0; j < FILTERSIZE; j++) {

			// filter is 2d data stored in 1d, so offset must be a flattened 2d coordinate
			offset.x = i * FILTERSIZE + j;

			// and then flattened further to be used as index into SSBO
			uint idx = offset.z + WINDOWSIZE * (offset.y + WINDOWSIZE * offset.x);
			vec2 samplePoint = projectedPos.xy + samples[idx] * SAMPLERADIUS;

			// reverse z
			cur += float(projectedPos.z + bias < texture(shadowmapTex, samplePoint).r);
		}

		// if all samples produced the same result we assume any subsequent samples will as well and break early
		if(cur == 0.0f || cur == 1.0f) {
			result += cur * (FILTERSIZE - i - 1);
			break;
		}
		else {
			result += cur;
		}
	}
	return 1.0f - result / (FILTERSIZE * FILTERSIZE);
}

float ggxNDF(vec3 normal, vec3 halfway, float alpha) {
	float alpha2 = alpha * alpha;
	float nDotH = clampedDot(normal, halfway);
	float denom = nDotH * nDotH * (alpha2 - 1.0f) + 1.0f;
	return alpha2 / (PI * denom * denom + EPSILON);
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

vec3 fresnelSchlick(float cosTheta, vec3 F0) {
	return F0 + (1.0f - F0) * pow(clamp(1.0f - cosTheta, 0.0f, 1.0f), 5.0f);
}

vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness) {
	return F0 + (max(vec3(1.0f - roughness), F0) - F0) * pow(clamp(1.0f - cosTheta, 0.0f, 1.0f), 5.0f);
}

vec3 directionalLight(vec3 viewDir, vec3 normal, vec3 albedo, float metalness, float roughness) {
	if(lightColor == vec3(0.0f)) return vec3(0.0f);
	float shadow = inShadow();
	if(shadow == 0.0f) return vec3(0.0f);

	vec3 halfway = normalize(viewDir + lightAngle);

	float distribution = ggxNDF(normal, halfway, roughness * roughness);
	float geometry = geometrySmith(normal, viewDir, lightAngle, roughness);
	vec3 fresnel = fresnelSchlick(clampedDot(halfway, viewDir), mix(vec3(0.04f), albedo, metalness));
	
	vec3 numerator = distribution * geometry * fresnel;
	float denominator = 4.0f * clampedDot(normal, viewDir) * clampedDot(normal, lightAngle) + EPSILON;
	
	vec3 specular = numerator / denominator;
	vec3 diffuse = (1.0f - fresnel) * (1.0f - metalness) * albedo / PI;
	return (diffuse + specular) * clampedDot(normal, lightAngle) * lightColor * lightIntensity * shadow;
}

vec3 ambientLight(vec3 viewDir, vec3 normal, vec3 albedo, float metalness, float roughness, float occlusion) {
	vec3 fresnel = fresnelSchlickRoughness(clampedDot(normal, viewDir), mix(vec3(0.04f), albedo, metalness), roughness);
	vec3 envMap = textureLod(envMapTex, reflect(-viewDir, normal), roughness * maxMip).rgb;
	vec2 brdf = texture(brdfLUTex, vec2(clampedDot(normal, viewDir), roughness)).rg;
	vec3 diffuse = (1.0f - fresnel) * (1.0f - metalness) * albedo * texture(irradianceTex, normal).rgb;
	vec3 specular = envMap * (fresnel * brdf.x + brdf.y);
	return (diffuse + specular) * occlusion;
}
 
void main() {
	vec3 viewDir = normalize(camPos - fPos);
	
	Material mat = materials[fMaterialIndex];
	
	vec4 materialColor = mat.baseColor;
	vec3 emissiveColor = mat.emissiveColor.xyz * mat.emissiveColor.w;
	vec3 normal = normalize(fNorm);
	float occlusion = 1.0f;
	float metalness = mat.metalness;
	float roughness = mat.roughness;

	if(bitmaskGet(mat.textureBitfield, HAS_ALBEDO)) materialColor *= texture(mat.albedoMap, fUV);
	if(bitmaskGet(mat.textureBitfield, HAS_EMISSIVE)) emissiveColor *= texture(mat.emissiveMap, fUV).rgb;
	if(bitmaskGet(mat.textureBitfield, HAS_OCCLUSION)) occlusion *= texture(mat.occlusionMap, fUV).r;
	if(bitmaskGet(mat.textureBitfield, HAS_NORMAL)) normal = normalize(mat3(normalize(fTan), normalize(fBitan), normal) * (texture(mat.normalMap, fUV).rgb * 2.0f - 1.0f));
	if(bitmaskGet(mat.textureBitfield, HAS_METALLIC_ROUGHNESS)) {
		metalness *= texture(mat.metallicRoughnessMap, fUV).b;
		roughness *= texture(mat.metallicRoughnessMap, fUV).g;	
	}

	roughness = isotrophicNDFFilter(normal, roughness);

	fCol = directionalLight(viewDir, normal, materialColor.rgb, metalness, roughness)
		 + ambientLight(viewDir, normal, materialColor.rgb, metalness, roughness, occlusion)
		 + emissiveColor;
}
