#version 460 core

uniform mat4 camMatrix;

out vec3 fPos;

void main() {
    // black magic to generate unit cube vertices I found online
	int tri = gl_VertexID / 3;
    int idx = gl_VertexID % 3;
    int face = tri / 2;
    int top = tri % 2;

    int dir = face % 3;
    int pos = face / 3;

    int nz = dir >> 1;
    int ny = dir & 1;
    int nx = 1 ^ (ny | nz);

    vec3 d = vec3(nx, ny, nz);
    float flip = 1 - 2 * pos;

    vec3 n = flip * d;
    vec3 u = -d.yzx;
    vec3 v = flip * d.zxy;

    float mirror = -1 + 2 * top;
    vec3 xyz = -(n + mirror * ( 1 - 2 * (idx & 1)) * u + mirror * (1 - 2 * (idx >> 1)) * v);

    fPos = xyz;
	gl_Position = (camMatrix * vec4(xyz, 1.0f)).xyww;
}