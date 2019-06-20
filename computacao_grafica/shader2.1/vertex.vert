#version 120

attribute vec3 position;
uniform mat4   transformation;

void main()
{
	gl_Position = transformation * vec4(position, 1.0);
}