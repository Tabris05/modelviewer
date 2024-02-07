#version 460 core

out vec3 FragColor;

flat in int drawid;

void main() {
	//FragColor = vec3(1.0f/float(drawid));
	FragColor = vec3(1.0f);
}
