#version 430

// VBO-ból érkezõ változók
in vec3 vs_in_pos;
in vec3 vs_in_normal;
in vec2 vs_in_tex;

uniform mat4 M;

out vec2 fragTex;

void main()
{
	vec4 pos = M * vec4(vs_in_pos.xy,0.05, 1);
	gl_Position = pos;
	fragTex = vs_in_tex;
}