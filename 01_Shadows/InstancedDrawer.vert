#version 430

layout (location = 0) in vec3 vs_in_pos;
layout (location = 1) in vec3 vs_in_normal;
layout (location = 2) in vec2 vs_in_tex;
layout (location = 3) in vec4 weights;
layout (location = 4) in vec4 boneId;

uniform mat4 PV;
uniform mat4 LightSpaceMtx;

#define MAX_INSTANCED 125
uniform vec3 positions[MAX_INSTANCED];
uniform float instancedAmount;


//Interface Block
out VS_OUT 
{
	vec3 wFragPos;
	vec3 normal;
	vec2 texCoord;
	vec4 fragPosLightSpace4;
	vec4 testColor;
} VS;

mat4 Translate (vec3 pos)
{
	mat4 matrix = mat4(1.0); //Identity
	matrix[3].xyz = pos;
	return matrix;
}

void main()
{
	vec3 wPosition = positions[gl_InstanceID];
	mat4 M = Translate(wPosition);
	mat4 Minv = Translate(-wPosition);
	mat4 PVM = PV * M;

	vec4 objSpacePos    = vec4(vs_in_pos, 1 );
	vec4 objSpaceNormal = vec4(vs_in_normal, 0);

	gl_Position = PVM * objSpacePos;

	VS.wFragPos = (M * objSpacePos).xyz;
	VS.normal = (objSpaceNormal * Minv).xyz;
	VS.texCoord = vs_in_tex;
	VS.fragPosLightSpace4 = (LightSpaceMtx * vec4(VS.wFragPos, 1.0));
	VS.testColor = weights;
}