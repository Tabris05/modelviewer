#version 330 core

out vec3 FragColor;

in vec2 fTexCoord;
in vec3 fLightDir;
in vec3 fCamPos;
in vec3 fFragPos;
in vec4 fFragPosLight;
in mat3 fTBN;

uniform samplerCube irradianceTexID;
uniform samplerCube prefilterTexID;
uniform sampler2D brdfLUTTexID;
uniform sampler2D diffuseTexID;
uniform sampler2D normalTexID;
uniform sampler2D aoTexID;
uniform sampler2D metalnessTexID;
uniform sampler2D roughnessTexID;
uniform sampler2D shadowMapTexID;
uniform sampler3D shadowNoiseTexID;

uniform vec3 lightCol;
uniform vec3 meshColor;
uniform float texelSize;
uniform float sampleRadius;
uniform int shadowNoiseWindowSize;
uniform int shadowNoiseFilterSize;
uniform float meshAO;
uniform float meshMetalness;
uniform float meshRoughness;
uniform float maxLOD;

uniform bool hasDiffuse;
uniform bool hasNormal;
uniform bool hasAO;
uniform bool hasMetalness;
uniform bool hasRoughness;
uniform bool iblEnabled;

#define PI 3.141593f
#define EPSILON 0.000001f

float inShadow() {
	float result = 0.0f;
	float bias = mix(0.005f, 0.0f, dot(vec3(0.0f, 0.0f, 1.0f), -fLightDir));
	vec3 projectedCoords = fFragPosLight.xyz / fFragPosLight.w * 0.5f + 0.5f;
	ivec3 offset = ivec3(0, ivec2(mod(gl_FragCoord.xy, vec2(shadowNoiseWindowSize))));
	for(int i = 0; i < clamp(shadowNoiseFilterSize / gl_FragCoord.z, 1, shadowNoiseFilterSize); i++) {
		float cur = 0.0f; 
		for(int j = i * shadowNoiseFilterSize; j < (i + 1) * shadowNoiseFilterSize; j++) {
			offset.x = j;
			vec2 samplePoint = projectedCoords.xy + texelFetch(shadowNoiseTexID, offset, 0).rg * sampleRadius * texelSize;
			cur += float(projectedCoords.z - bias > texture(shadowMapTexID, samplePoint).r);
		}
		if(cur == 0.0f || cur == 1.0f) {
			result += cur * (shadowNoiseFilterSize - i - 1);
			break;
		}
		else {
			result += cur;
		}
	}
	return 1.0f - (result / (shadowNoiseFilterSize * shadowNoiseFilterSize));
}

float distributionGGX(vec3 normal, vec3 halfway, float roughness) {
	float alpha = roughness * roughness;
	float alphaSquared = alpha * alpha;
	float nDotH = max(dot(normal, halfway), 0.0f);
	float nDotHSquared = nDotH * nDotH;
	float bottomTerm = nDotHSquared * (alphaSquared - 1.0f) + 1.0f;
	return alphaSquared / (PI * bottomTerm * bottomTerm);
}

float geometrySmith(vec3 normal, vec3 viewDir, vec3 lightDir, float roughness) {
	float a = roughness + 1.0f;
	float k = a * a / 8.0f;
	float nDotV = max(dot(normal, viewDir), 0.0f);
	float nDotL = max(dot(normal, lightDir), 0.0f);
	float ggx1 = nDotV / (nDotV * (1.0f - k) + k);
	float ggx2 = nDotL / (nDotL * (1.0f - k) + k);
	return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 f0) {
	return f0 + (1.0f - f0) * pow(clamp(1.0f - cosTheta, 0.0f, 1.0f), 5.0f);
}

vec3 fresnelSchlickRoughness(float cosTheta, vec3 f0, float roughness) {
	return f0 + (max(vec3(1.0f - roughness), f0) - f0) * pow(clamp(1.0f - cosTheta, 0.0f, 1.0f), 5.0f);
}

vec3 directionalLight(vec3 albedo) {
	vec3 normal = hasNormal ? normalize(texture(normalTexID, fTexCoord).rgb * 2.0f - 1.0f) : vec3(0.0f, 0.0f, 1.0f);
	float ao = hasAO ? texture(aoTexID, fTexCoord).r : meshAO;
	float roughness = hasRoughness ? texture(roughnessTexID, fTexCoord).g : meshRoughness;
	float metalness = hasMetalness ? texture(metalnessTexID, fTexCoord).b : meshMetalness;
	
	
	vec3 viewDir = normalize(fCamPos - fFragPos);
	vec3 lightDir = -normalize(fLightDir);
	vec3 halfway =  normalize(viewDir + lightDir);
	vec3 f0 = mix(vec3(0.04), albedo, metalness);

	float distribution = distributionGGX(normal, halfway, roughness);
	float geometry = geometrySmith(normal, viewDir, lightDir, roughness);
	vec3 fresnel = fresnelSchlick(max(dot(halfway, viewDir), 0.0f), f0);
	
	vec3 diffuse = (vec3(1.0f) - fresnel) * (1.0f - metalness) * albedo / PI;
	vec3 specular = (distribution * geometry * fresnel) / (4.0f * max(dot(normal, viewDir), 0.0f) * max(dot(normal, lightDir), 0.0f) + EPSILON);
	vec3 ambient = vec3(0.0f);
	if (iblEnabled) {
		vec3 worldView = fTBN * viewDir;
		vec3 worldNormal = fTBN * normal;
		vec3 F = fresnelSchlickRoughness(max(dot(normal, viewDir), 0.0f), f0, roughness);
		vec3 kD = (1.0f - F) * (1.0f - metalness);
		vec3 prefilteredColor = textureLod(prefilterTexID, reflect(-worldView, worldNormal), roughness * maxLOD).rgb;
		vec2 brdf = texture(brdfLUTTexID, vec2(max(dot(normal, viewDir), 0.0f), roughness)).rg;
		vec3 ambientDiffuse = kD * texture(irradianceTexID, worldNormal).rgb * albedo;
		vec3 ambientSpecular = prefilteredColor * (F * brdf.x + brdf.y);
		ambient = (ambientDiffuse + ambientSpecular) * ao;
	}
	else ambient = vec3(0.05f) * albedo * ao;
	return (diffuse + specular) * lightCol * max(dot(normal, lightDir), 0.0f) * inShadow() + ambient;
}

void main() {
	FragColor = directionalLight(hasDiffuse ? texture(diffuseTexID, fTexCoord).rgb : meshColor);
}