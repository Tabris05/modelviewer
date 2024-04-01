#version 460 core
#extension GL_ARB_bindless_texture : require

#define INVPI 0.318309f

in vec3 fPos;

layout(bindless_sampler) uniform sampler2D equirectangularMap;

out vec3 fCol;

void main() {
	vec3 pos = normalize(fPos);
	vec2 uv = vec2(atan(pos.z, pos.x) * 0.5, asin(pos.y)) * INVPI + 0.5f;
	uv.y = 1.0f - uv.y; // OpenGL has a flipped y axis compared to stbi
	// hdr texture artists can NOT be trusted to keep their light levels in an acceptable range
	fCol = clamp(texture(equirectangularMap, uv).rgb, 0.0f, 100.0f); 
}