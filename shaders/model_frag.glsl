#version 460 core

in vec3 fPos;
in vec3 fNorm;
in vec2 fUV;

flat in int fDrawID;

uniform vec3 camPos;

layout(std430, binding = 0) readonly buffer MaterialBuffer {
	vec4 materialColors[];
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
	//vec3 viewDir = normalize(camPos - fPos);
	//vec3 normal = normalize(fNorm);
	//normal = normalize(makeTBN(normal, -viewDir, fUV) * vec3(0.0f, 0.0f, 1.0f));
	//fCol = vec3(1.0f) * dot(normalize(normal), vec3(0.0f, 1.0f, -1.0f));
	fCol = materialColors[fDrawID].rgb;
}
