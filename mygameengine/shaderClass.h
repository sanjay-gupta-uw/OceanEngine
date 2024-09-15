#pragma once

#ifndef SHADECLASS_H
#define SHADECLASS_H

#include <glad/glad.h>
#include <iostream>
#include "utility.h"

class Shader {
public:
	GLuint ID;
	Shader(const char* vertexPath, const char* fragmentPath);

	void Bind();
	void Unbind();
	void Delete();

private:
	void compileErrors(unsigned int shader, const char* type);
};


#endif