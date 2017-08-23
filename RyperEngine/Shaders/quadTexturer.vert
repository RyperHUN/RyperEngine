#version 430

// VBO-ból érkezõ változók
layout (location = 0) in vec3 vs_in_pos;
layout (location = 1) in vec3 vs_in_normal;
layout (location = 2) in vec2 vs_in_tex;

uniform mat4 M;

out vec2 fragTex;

void main()
{
	vec4 pos = M * vec4(vs_in_pos.xy, 0, 1);
	gl_Position = pos;
	fragTex = vs_in_tex;
}