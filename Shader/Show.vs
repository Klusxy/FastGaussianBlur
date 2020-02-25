#version 330 core
layout(location = 0) in vec3 iPos;
layout(location = 1) in vec2 iTex;
out vec2 uv;

void main()
{
	gl_Position = vec4(iPos.x, iPos.y, iPos.z, 1.0f);
	uv = iTex;
}