#version 330 core

out vec3 FragColor;

in vec2 fTexCoord;
in vec3 fLightDir;
in vec3 fCamPos;
in vec3 fFragPos;
in vec3 fCol;
in vec4 fFragPosLight;

uniform sampler2D diffuseTexID;
uniform sampler2D normalTexID;
uniform sampler2D aoTexID;
uniform sampler2D metalnessTexID;
uniform sampler2D roughnessTexID;
uniform sampler2D shadowMapTexID;
uniform sampler3D shadowNoiseTexID;
uniform vec3 lightCol;
uniform float texelSize;
uniform float sampleRadius;
uniform int shadowNoiseWindowSize;
uniform int shadowNoiseFilterSize;

uniform bool hasDiffuse;
uniform bool hasNormal;
uniform bool hasAO;
uniform bool hasMetalness;
uniform bool hasRoughness;

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
		if(i == 0 && (cur == 0.0f || cur == 1.0f)) {
			result += cur * (shadowNoiseFilterSize - i - 1);
			break;
		}
		else {
			result += cur;
		}
	}
	return 1.0f - (result / (shadowNoiseFilterSize * shadowNoiseFilterSize));
}

vec3 directionalLight(vec3 albedo) {
	
	vec3 normal = hasNormal ? normalize(texture(normalTexID, fTexCoord).rgb * 2.0f - 1.0f) : vec3(0.0f, 0.0f, 1.0f);
	float specAmount = 1.0f - (hasRoughness ? texture(roughnessTexID, fTexCoord).g : 1.0f);
	float ao = hasAO ? texture(aoTexID, fTexCoord).r : 1.0f;
	vec3 direction = -normalize(fLightDir);

	// ambient light
	float ambient = 0.05f * ao;

	// diffuse light
	float diffuse = max(dot(normal, direction), 0.0f);

	// specular light
	vec3 viewDir = normalize(fCamPos - fFragPos);
	vec3 halfway = normalize(viewDir + direction);
	float specular = diffuse * pow(max(dot(halfway, normal), 0.0f), 32.0f) * specAmount * 0.1f;

	return (albedo * diffuse + specular) * lightCol * inShadow() + albedo * ambient;
}

void main() {
	FragColor = fCol * (hasDiffuse ? texture(diffuseTexID, fTexCoord).rgb : vec3(1.0f));
	FragColor = directionalLight(FragColor);
}