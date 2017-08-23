#version 430

// VBO-ból érkezõ változók
layout (location = 0) in vec3 vs_in_pos;
layout (location = 1) in vec3 vs_in_normal;
layout (location = 2) in vec2 vs_in_tex;

uniform mat4 M;
uniform bool isBackPos;

out vec2 fragTex;

void main()
{
	vec4 in_pos = vec4(vs_in_pos.xy, 0, 1);
	if (isBackPos)
		in_pos.z = 0.999;

	vec4 pos = M * in_pos;
	gl_Position = pos;
	fragTex = vs_in_tex;
}