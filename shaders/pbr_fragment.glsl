#version 330 core

out vec3 FragColor;

in vec2 fTexCoord;
in vec3 fLightDir;
in vec3 fCamPos;
in vec3 fFragPos;
in vec3 fCol;

uniform sampler2D diffuseTexID;
uniform sampler2D normalTexID;
uniform sampler2D aoTexID;
uniform sampler2D metalnessTexID;
uniform sampler2D roughnessTexID;
uniform vec3 lightCol;

uniform bool hasDiffuse;
uniform bool hasNormal;
uniform bool hasAO;
uniform bool hasMetalness;
uniform bool hasRoughness;

uniform bool normalsActive;

#define PI 3.141593f

float distributionGGX(vec3 normal, vec3 halfway, float alpha) {
	float alphaSquared = alpha * alpha;
	float nDotH = max(dot(normal, halfway), 0.0f);
	float nDotHSquared = nDotH * nDotH;
	float bottomTerm = nDotHSquared * (alphaSquared - 1.0f) + 1.0f;
	return alphaSquared / (PI * bottomTerm * bottomTerm);
}

float geometrySmith(vec3 normal, vec3 viewDir, vec3 lightDir, float roughness) {
	float nDotV = max(dot(normal, viewDir), 0.0f);
	float nDotL = max(dot(normal, lightDir), 0.0f);
	float ggx1 = nDotV / (nDotV * (1.0f - roughness) + roughness);
	float ggx2 = nDotL / (nDotL * (1.0f - roughness) + roughness);
	return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 f0) {
	return f0 + (1.0f - f0) * pow(clamp(1.0f - cosTheta, 0.0f, 1.0f), 5.0f);
}

vec3 directionalLight(vec3 albedo) {
	vec3 normal = hasNormal ? normalize(texture(normalTexID, fTexCoord).rgb * 2.0f - 1.0f) : vec3(0.0f, 0.0f, 1.0f);
	float ao = hasAO ? texture(aoTexID, fTexCoord).r : 1.0f;
	float roughness = hasRoughness ? texture(roughnessTexID, fTexCoord).g : 1.0f;
	float metalness = hasMetalness ? texture(metalnessTexID, fTexCoord).b : 0.0f;
	
	vec3 viewDir = normalize(fCamPos - fFragPos);
	vec3 lightDir = -normalize(fLightDir);
	vec3 halfway =  normalize(viewDir + lightDir);

	float distribution = distributionGGX(normal, halfway, roughness);
	float geometry = geometrySmith(normal, viewDir, lightDir, roughness);
	vec3 fresnel = fresnelSchlick(max(dot(halfway, viewDir), 0.0f), mix(vec3(0.04), albedo, metalness));
	
	vec3 diffuse = (vec3(1.0f) - fresnel) * (1.0f - metalness);
	vec3 specular = (distribution * geometry * fresnel) / (4.0f * max(dot(normal, viewDir), 0.0f) * max(dot(normal, lightDir), 0.0f) + 0.000001f);
	vec3 ambient = vec3(0.05f) * albedo * ao;

	return (diffuse * albedo / PI + specular) * lightCol * max(dot(normal, lightDir), 0.0f) + ambient;
}

void main() {
	FragColor = fCol * (hasDiffuse ? texture(diffuseTexID, fTexCoord).rgb : vec3(1.0f));
	FragColor = directionalLight(FragColor.rgb);
}