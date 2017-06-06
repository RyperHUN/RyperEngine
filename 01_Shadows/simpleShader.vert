#version 430

in vec3 vs_in_pos;
in vec3 vs_in_normal;
in vec2 vs_in_tex;

uniform mat4 PVM;
uniform mat4 M;
uniform mat4 Minv;
uniform mat4 LightSpaceMtx;

//Interface Block
out VS_OUT 
{
	out vec3 wFragPos;
	out vec3 normal;
	out vec2 texCoord;
	out vec4 fragPosLightSpace4;
} VS;



void main()
{
	gl_Position = PVM*vec4( vs_in_pos, 1 );

	VS.wFragPos = (M * vec4(vs_in_pos, 1)).xyz;
	VS.normal = (vec4(vs_in_normal, 0)* Minv).xyz;
	VS.texCoord = vs_in_tex;
	VS.fragPosLightSpace4 = (LightSpaceMtx * vec4(VS.wFragPos, 1.0));
}