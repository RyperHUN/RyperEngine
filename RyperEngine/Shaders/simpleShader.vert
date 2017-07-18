#version 430

layout (location = 0) in vec3 vs_in_pos;
layout (location = 1) in vec3 vs_in_normal;
layout (location = 2) in vec2 vs_in_tex;
layout (location = 3) in vec4 vs_in_weights;
layout (location = 4) in vec4 vs_in_boneId;

uniform mat4 PVM;
uniform mat4 M;
uniform mat4 Minv;
uniform mat4 LightSpaceMtx;
uniform bool isAnimated;

#define MAX_BONES 41
uniform mat4 boneTransformations[MAX_BONES];

//Interface Block
out VS_OUT 
{
	vec3 wFragPos;
	vec3 normal;
	vec2 texCoord;
	vec4 fragPosLightSpace4;
	vec4 testColor;
} VS;

mat4 getBoneTransform (vec4 weights, vec4 boneId)
{
	ivec4 BoneIDs	   = ivec4(boneId.x, boneId.y, boneId.z, boneId.w);
	mat4 boneTransform = boneTransformations[BoneIDs[0]] * weights[0];
	for(int i = 1; i < 4; i++)
		boneTransform     += boneTransformations[BoneIDs[i]] * weights[i];
	return boneTransform;
}

void main()
{
	mat4 boneTransform = mat4(1.0);
	if (isAnimated)
		boneTransform = getBoneTransform(vs_in_weights, vs_in_boneId);
	
	vec4 objSpacePos    = boneTransform * vec4(vs_in_pos, 1 );
	vec4 objSpaceNormal = boneTransform * vec4(vs_in_normal, 0);

	gl_Position = PVM * objSpacePos;

	VS.wFragPos = (M * objSpacePos).xyz;
	VS.normal   = (objSpaceNormal * Minv).xyz;
	VS.texCoord = vs_in_tex;
	VS.fragPosLightSpace4 = (LightSpaceMtx * vec4(VS.wFragPos, 1.0));
	VS.testColor = vs_in_weights;
}