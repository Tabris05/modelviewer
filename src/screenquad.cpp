#include "screenquad.h"

ScreenQuad::ScreenQuad(int width, int height) :
	m_vBuf{ std::vector<float>{
	1.0f, -1.0f,  1.0f, 0.0f,
	-1.0f, -1.0f,  0.0f, 0.0f,
	-1.0f,  1.0f,  0.0f, 1.0f,
	 1.0f,  1.0f,  1.0f, 1.0f,
	 1.0f, -1.0f,  1.0f, 0.0f,
	-1.0f,  1.0f,  0.0f, 1.0f } } {
	m_vArr.linkAttribute(m_vBuf, 0, 2, GL_FLOAT, 4 * sizeof(float), 0);
	m_vArr.linkAttribute(m_vBuf, 1, 2, GL_FLOAT, 4 * sizeof(float), 2 * sizeof(float));
	m_vArr.unbind();
	m_vBuf.unbind();
}

void ScreenQuad::draw(ShaderProgram& shader) {
	glDisable(GL_DEPTH_TEST);
	shader.activate();
	m_vArr.bind();
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glEnable(GL_DEPTH_TEST);
}