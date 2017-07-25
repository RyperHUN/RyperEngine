#version 430

layout (location = 0) out vec3 out_pos;
layout (location = 1) out vec3 out_normal;
layout (location = 2) out vec3 out_color;

in VS_OUT 
{
	in vec3 wFragPos;
	in vec3 normal;
	in vec2 texCoord;
	in vec4 fragPosLightSpace4;
	in vec4 testColor;
} FS;

struct Material {
	vec3 ka;
	vec3 kd;
	vec3 ks;
	float shininess;
};

uniform sampler2D shadowMap;
uniform sampler2D texture_diffuse;
uniform sampler2D texture_specular;
uniform sampler2D texture_reflect;
uniform samplerCube skyBox;

uniform Material uMaterial;

void main()
{
	out_color.xyz = uMaterial.kd;

	out_pos    = FS.wFragPos;
	out_normal = FS.normal;
}