#version 430

layout (location = 0) in vec3 vs_in_pos;
layout (location = 1) in vec3 vs_in_normal;
layout (location = 2) in vec2 vs_in_tex;
layout (location = 10) in vec3 positions;

uniform mat4 PV;
uniform mat4 LightSpaceMtx;
uniform float uScale;

//Interface Block
out VS_OUT 
{
	vec3 wFragPos;
	vec3 normal;
	vec2 texCoord;
	vec4 fragPosLightSpace4;
	flat int instanceId;
} VS;

mat4 Translate (vec3 pos)
{
	mat4 matrix = mat4(1.0); //Identity
	matrix[3].xyz = pos;
	return matrix;
}

mat4 GetModel(vec3 pos, float scale)
{
	mat4 matrix = mat4(scale);
	matrix[3][3] = 1;
	return Translate(pos) * matrix;
}

uniform vec4 uPlane;

void main()
{
	vec3 wPosition = positions;
	mat4 M = GetModel(wPosition, uScale);
	mat4 Minv = GetModel (-wPosition, 1 / uScale);
	mat4 PVM = PV * M;

	vec4 objSpacePos    = vec4(vs_in_pos, 1 );
	vec4 objSpaceNormal = vec4(vs_in_normal, 0);

	gl_Position = PVM * objSpacePos;

	VS.wFragPos = (M * objSpacePos).xyz;
	gl_ClipDistance[0] = dot(M * objSpacePos,uPlane);  //Clipping for WaterRendering

	VS.normal = (objSpaceNormal * Minv).xyz;
	VS.texCoord = vs_in_tex;
	VS.fragPosLightSpace4 = (LightSpaceMtx * vec4(VS.wFragPos, 1.0));
	VS.instanceId = gl_InstanceID;
}