#version 460 core

uniform mat4 camMatrix;

out vec3 fPos;

vec3 vertices[36] = {
    { -1,  1, -1 },
    { -1,  1,  1 },
    { -1, -1, -1 },
    { -1, -1,  1 },
    { -1, -1, -1 },
    { -1,  1,  1 },
    { -1, -1,  1 },
    {  1, -1,  1 },
    { -1, -1, -1 },
    {  1, -1, -1 },
    { -1, -1, -1 },
    {  1, -1,  1 },
    {  1, -1, -1 },
    {  1,  1, -1 },
    { -1, -1, -1 },
    { -1,  1, -1 },
    { -1, -1, -1 },
    {  1,  1, -1 },
    {  1, -1, -1 },
    {  1, -1,  1 },
    {  1,  1, -1 },
    {  1,  1,  1 },
    {  1,  1, -1 },
    {  1, -1,  1 },
    { -1,  1, -1 },
    {  1,  1, -1 },
    { -1,  1,  1 },
    {  1,  1,  1 },
    { -1,  1,  1 },
    {  1,  1, -1 },
    { -1, -1,  1 },
    { -1,  1,  1 },
    {  1, -1,  1 },
    {  1,  1,  1 },
    {  1, -1,  1 },
    { -1,  1,  1 }
};

void main() {
    vec3 xyz = vertices[gl_VertexID];
    fPos = xyz;
    vec4 transformedPos = camMatrix * vec4(xyz, 1.0f);
    transformedPos.z = 0.0f; // reverse z
	gl_Position = transformedPos;
}