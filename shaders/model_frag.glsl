#version 460 core

in vec3 fPos;
in vec3 fNorm;
in vec2 fUV;

flat in int fDrawID;

uniform vec3 camPos;

out vec3 fCol;

mat3 makeTBN(vec3 N, vec3 pos, vec2 uv) {
	vec3 dpdx = dFdx(pos);
	vec3 dpdy = dFdy(pos);
	vec2 duvdx = dFdx(uv);
	vec2 duvdy = dFdy(uv);
	vec3 dpdxPerp = cross(N, dpdx);
	vec3 dpdyPerp = cross(dpdy, N);
	vec3 T = dpdyPerp * duvdx.x + dpdxPerp * duvdy.x;
	vec3 B = dpdyPerp * duvdx.y + dpdxPerp * duvdy.y;
	float invmax = inversesqrt(max(dot(T, T), dot(B, B)));
	return mat3(T * invmax, B * invmax, N);
}

void main() {
	fCol = vec3(1.0f) * dot(normalize(fNorm), vec3(0.0f, 1.0f, -1.0f));
}
