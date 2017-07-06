#version 430

in VS_OUT 
{
	in vec3 wFragPos;
	in vec3 normal;
	in vec2 texCoord;
	in vec4 fragPosLightSpace4;
	in vec4 testColor;
} FS;

uniform sampler2D diffuseTex;
uniform sampler2DArray tex1;
uniform int layer = 1;

//TODO Texture for cube
out vec4 fs_out_col;

void main()
{
	vec3 normal = normalize (FS.normal);
	vec3 color = mix(normal, vec3(FS.texCoord, 0), 0.5);

	fs_out_col = vec4(color, 1);
	fs_out_col = texture(tex1, vec3(FS.texCoord, layer));
	//fs_out_col = texture(diffuseTex, FS.texCoord);
}