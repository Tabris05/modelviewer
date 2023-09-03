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

	return (albedo * (diffuse + ambient) + specular) * lightCol;
}

void main() {
	FragColor = fCol * (hasDiffuse ? texture(diffuseTexID, fTexCoord).rgb : vec3(1.0f));
	FragColor = directionalLight(FragColor);
}