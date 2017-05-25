#version 430

in vec3 vs_in_pos;
in vec3 vs_in_normal;
in vec2 vs_in_tex;

uniform mat4 MVP;
uniform mat4 M;
uniform mat4 Minv;
uniform mat4 LightSpaceMtx;

out vec3 wFragPos;
out vec3 frag_normal;
out vec2 frag_tex;

out vec4 fragPosLightSpace4;

void main()
{
	gl_Position = MVP*vec4( vs_in_pos, 1 );

	wFragPos = (M * vec4(vs_in_pos, 1)).xyz;
	frag_normal = (vec4(vs_in_normal, 0)* Minv).xyz;
	frag_tex = vs_in_tex;
	fragPosLightSpace4 = (LightSpaceMtx * vec4(wFragPos, 1.0));
}