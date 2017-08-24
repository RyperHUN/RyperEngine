#version 430

// VBO-ból érkezõ változók
layout (location = 0) in vec3 vs_in_pos;
layout (location = 2) in vec2 vs_in_tex;
layout (location = 3) in float in_alpha;
layout (location = 4) in mat4 MVP;
//TODO Texture index

out vec2 fragTex;
out float alpha;

void main()
{
	vec4 in_pos = vec4(vs_in_pos.xy, 0, 1);

	vec4 pos = MVP * in_pos;
	gl_Position = pos;
	fragTex = vs_in_tex;
	alpha = in_alpha;
}