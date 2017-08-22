#version 430

// VBO-ból érkezõ változók
layout (location = 0) in vec3 vs_in_pos;
layout (location = 8) in vec3 vs_in_color;

uniform mat4 M;

out vec3 color;

void main()
{
	vec4 pos = M * vec4(vs_in_pos.xyz, 1);
	gl_Position = pos;
	color = vs_in_color;
}