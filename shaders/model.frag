#version 460 core
#extension GL_ARB_bindless_texture : require

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

layout(std430, binding = 0) readonly buffer MaterialBuffer {
	Material materials[];
};

layout(std430, binding = 1) readonly buffer MaterialIndexBuffer {
	uint materialIndices[];
};

out vec3 fCol;

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

void main() {
	uint i = materialIndices[fDrawID];
	vec3 viewDir = normalize(camPos - fPos);
	vec3 normal = normalize(fNorm);
	vec4 materialColor = texture(materials[i].albedoMap, fUV) * materials[i].baseColor;
	normal = normalize(makeTBN(normal, -viewDir, fUV) * (texture(materials[i].normalMap, fUV).rgb * 2.0f - 1.0f));
	fCol = materialColor.rgb * dot(normalize(normal), vec3(0.0f, 1.0f, -1.0f));
}
