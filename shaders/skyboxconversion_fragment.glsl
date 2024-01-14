#version 330 core

out vec3 FragColor;

in vec3 fPos;

uniform sampler2D equirectangular;

const vec2 invAtan = vec2(0.1591, 0.3183);

void main() {
    vec3 pos = normalize(fPos);
    vec2 uv = vec2(atan(pos.z, pos.x), asin(pos.y)) * invAtan + 0.5;
    // hdr texture artists can NOT be trusted to keep their light levels in an acceptable range
    FragColor = clamp(texture(equirectangular, uv).rgb, 0.0f, 100.0f);
}