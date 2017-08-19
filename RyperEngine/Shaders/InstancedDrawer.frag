#version 430

in VS_OUT 
{
	vec3 wFragPos;
	vec3 normal;
	vec2 texCoord;
	vec4 fragPosLightSpace4;
	flat int instanceId;
	flat ivec4 texId;
} FS;

uniform sampler2D diffuseTex;
uniform sampler2DArray tex1;

//TODO Texture for cube
out vec4 fs_out_col;

void main()
{
	vec3 normal = normalize (FS.normal);
	vec3 color = mix(normal, vec3(FS.texCoord, 0), 0.5);

	fs_out_col = vec4(color, 1);
	
	fs_out_col = texture(tex1, vec3(FS.texCoord, FS.texId.x));

	//fs_out_col = vec4(FS.instanceId * 0.001, 0, 0, 1);
	//fs_out_col = vec4(FS.texId.x* 0.0001,0,0,1);
}