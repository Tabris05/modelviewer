#version 460 core
#extension GL_ARB_bindless_texture : require

#define PI 3.141593f

in vec3 fPos;

layout(bindless_sampler) uniform samplerCube skyboxTex;

out vec3 fCol;

void main() {
    vec3 irradiance = vec3(0.0f);
    vec3 normal = normalize(fPos);
    vec3 up = vec3(0.0f, 1.0f, 0.0f);
    vec3 right = normalize(cross(up, normal));
    up = normalize(cross(normal, right));

    const float sampleDelta = 0.025f;
    float numSamples = 0.0f;
    for(float phi = 0.0f; phi < PI * 2.0f; phi += sampleDelta) {
        for(float theta = 0.0f; theta < PI * 0.5f; theta += sampleDelta) {
            vec3 tangentSample = vec3(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));
            vec3 sampleDir = tangentSample.x * right + tangentSample.y * up + tangentSample.z * normal; 
            irradiance += texture(skyboxTex, sampleDir).rgb * cos(theta) * sin(theta);
            numSamples++;
        }
    }
    fCol = irradiance * PI / numSamples;
}