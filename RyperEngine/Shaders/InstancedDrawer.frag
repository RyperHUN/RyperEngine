#version 430

in VS_OUT 
{
	vec3 wFragPos;
	vec3 normal;
	vec2 texCoord;
	vec4 fragPosLightSpace4;
	flat int instanceId;
} FS;

uniform sampler2D diffuseTex;
uniform sampler2DArray tex1;

#define MAX_INSTANCED 125
uniform int uLayer[MAX_INSTANCED];

//TODO Texture for cube
out vec4 fs_out_col;

void main()
{
	vec3 normal = normalize (FS.normal);
	vec3 color = mix(normal, vec3(FS.texCoord, 0), 0.5);

	fs_out_col = vec4(color, 1);
	
	//fs_out_col = texture(tex1, vec3(FS.texCoord, uLayer[1]));

	fs_out_col = vec4(FS.instanceId * 0.01, 0, 0, 1);
	//fs_out_col = texture(diffuseTex, FS.texCoord);
}