#ifndef __SHADER_H
#define __SHADER_H

#include <GL/glew.h>

GLuint loadAndCompileShaderFromMemory(GLenum shaderType, GLsizei lines, const GLchar **source);
GLuint loadAndCompileShaderFromFile(GLenum shaderType, const char *name);
GLuint installShaders(const char *vertex, const char *fragment);
void runProgram(GLuint program);

#endif