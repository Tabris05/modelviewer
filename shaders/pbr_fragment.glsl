#version 330 core

out vec3 FragColor;

in vec2 fTexCoord;
in vec3 fFragPos;
in vec4 fFragPosLight;
in mat3 fTBN;

uniform samplerCube irradianceTexID;
uniform samplerCube prefilterTexID;
uniform sampler2D brdfLUTTexID;
uniform sampler2D diffuseTexID;
uniform sampler2D emissiveTexID;
uniform sampler2D normalTexID;
uniform sampler2D aoTexID;
uniform sampler2D metalnessTexID;
uniform sampler2D roughnessTexID;
uniform sampler2D shadowMapTexID;
uniform sampler3D shadowNoiseTexID;

uniform vec3 lightCol;
uniform vec3 meshColor;
uniform vec3 meshEmission;
uniform vec3 lightDir;
uniform vec3 camPos;
uniform float texelSize;
uniform float sampleRadius;
uniform int shadowNoiseWindowSize;
uniform int shadowNoiseFilterSize;
uniform float meshAO;
uniform float meshMetalness;
uniform float meshRoughness;
uniform float emissiveIntensity;
uniform float maxLOD;

uniform bool hasDiffuse;
uniform bool hasEmissive;
uniform bool hasNormal;
uniform bool hasAO;
uniform bool hasMetalness;
uniform bool hasRoughness;
uniform bool specAA;

#define PI 3.141593f
#define EPSILON 0.000001f

float inShadow() {
	float result = 0.0f;
	float bias = mix(0.005f, 0.0f, dot(fTBN[2], -lightDir));
	vec3 projectedCoords = fFragPosLight.xyz / fFragPosLight.w * 0.5f + 0.5f;
	ivec3 offset = ivec3(0, ivec2(mod(gl_FragCoord.xy, vec2(shadowNoiseWindowSize))));
	for (int i = 0; i < clamp(shadowNoiseFilterSize / gl_FragCoord.z, 1, shadowNoiseFilterSize); i++) {
		float cur = 0.0f;
		for (int j = i * shadowNoiseFilterSize; j < (i + 1) * shadowNoiseFilterSize; j++) {
			offset.x = j;
			vec2 samplePoint = projectedCoords.xy + texelFetch(shadowNoiseTexID, offset, 0).rg * sampleRadius * texelSize;
			cur += float(projectedCoords.z - bias > texture(shadowMapTexID, samplePoint).r);
		}
		if (cur == 0.0f || cur == 1.0f) {
			result += cur * (shadowNoiseFilterSize - i - 1);
			break;
		}
		else {
			result += cur;
		}
	}
	return 1.0f - (result / (shadowNoiseFilterSize * shadowNoiseFilterSize));
}

float distributionGGX(vec3 normal, vec3 halfway, float alpha) {
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

vec3 fsnormal(vec3 N, vec3 p, vec2 uv) {
	vec3 dp1 = dFdx(p); vec3 dp2 = dFdy(p); vec2 duv1 = dFdx(uv); vec2 duv2 = dFdy(uv);
	vec3 dp2perp = cross( dp2, N );
	vec3 dp1perp = cross( N, dp1 );
	vec3 T = dp2perp * duv1.x + dp1perp * duv2.x;
	vec3 B = dp2perp * duv1.y + dp1perp * duv2.y;
	float invmax = inversesqrt( max( dot(T,T), dot(B,B) ) );
	return normalize(mat3(T * invmax, B * invmax, N) * (texture(normalTexID, fTexCoord).rgb * 2.0f - 1.0f));
}

float specantialias(vec3 normal, float r2) {
	float S = 0.15915494f;
	float K = 0.18f;
	vec3 ddu = dFdx(normal);
	vec3 ddv = dFdy(normal);
	float kr2 = min(2.0f * S * (dot(ddu, ddu) + dot(ddv, ddv)), K);
	return clamp(r2 + kr2, 0.0f, 1.0f);
}

vec3 directionalLight(vec3 albedo) {
	vec3 normal = hasNormal ? (fTBN * normalize(texture(normalTexID, fTexCoord).rgb * 2.0f - 1.0f)) : fTBN[2];
	float ao = hasAO ? texture(aoTexID, fTexCoord).r : meshAO;
	float roughness = hasRoughness ? texture(roughnessTexID, fTexCoord).g : meshRoughness;
	float metalness = hasMetalness ? texture(metalnessTexID, fTexCoord).b : meshMetalness;

	vec3 viewDir = normalize(camPos - fFragPos);
	vec3 lightDir = -normalize(lightDir);
	vec3 halfway = normalize(viewDir + lightDir);
	vec3 f0 = mix(vec3(0.04f), albedo, metalness);

	if (specAA && hasNormal) normal = fsnormal(fTBN[2], -viewDir, fTexCoord);

	// directional light
	if (specAA && hasNormal) roughness = sqrt(specantialias(normal, roughness * roughness));
	float distribution = distributionGGX(normal, halfway, roughness * roughness); // contributes to specular aliasing
	float geometry = geometrySmith(normal, viewDir, lightDir, roughness);
	vec3 fresnel = fresnelSchlick(max(dot(halfway, viewDir), 0.0f), f0);

	vec3 diffuse = (vec3(1.0f) - fresnel) * (1.0f - metalness) * albedo / PI;
	vec3 specular = (distribution * geometry * fresnel) / (4.0f * max(dot(normal, viewDir), 0.0f) * max(dot(normal, lightDir), 0.0f) + EPSILON);

	// ambient light
	vec3 F = fresnelSchlickRoughness(max(dot(normal, viewDir), 0.0f), f0, roughness);
	vec3 kD = (1.0f - F) * (1.0f - metalness);
	vec3 prefilteredColor = textureLod(prefilterTexID, reflect(-viewDir, normal), roughness * maxLOD).rgb;
	vec2 brdf = texture(brdfLUTTexID, vec2(max(dot(normal, viewDir), 0.0f), roughness)).rg;
	vec3 ambientDiffuse = kD * texture(irradianceTexID, normal).rgb * albedo;
	vec3 ambientSpecular = prefilteredColor * (F * brdf.x + brdf.y); // contributes to specular aliasing
	vec3 ambient = (ambientDiffuse + ambientSpecular) * ao;
	return (diffuse + specular) * lightCol * max(dot(normal, lightDir), 0.0f) * inShadow() + ambient + (hasEmissive ? texture(emissiveTexID, fTexCoord).rgb : meshEmission) * emissiveIntensity;
}

void main() {
	FragColor = directionalLight(hasDiffuse ? texture(diffuseTexID, fTexCoord).rgb : meshColor);
}