#version 430

in VS_OUT 
{
	in vec3 wFragPos;
	in vec3 normal;
	in vec2 texCoord;
	in vec4 fragPosLightSpace4;
	in vec4 testColor;
} FS;

//TODO Texture for cube
out vec4 fs_out_col;

void main()
{
	vec3 normal = normalize (FS.normal);

	fs_out_col = vec4(abs(normal), 1);
}