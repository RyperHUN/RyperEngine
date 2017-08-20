#version 430

in VS_OUT 
{
	vec3 wFragPos;
	vec3 normal;
	vec2 texCoord;
	vec4 fragPosLightSpace4;
	flat int instanceId;
	flat ivec4 texId;
	flat int cubeSide;
} FS;

uniform sampler2DArray tex1;

out vec4 fs_out_col;

void main()
{
	vec3 normal = normalize (FS.normal);
	vec3 color = mix(normal, vec3(FS.texCoord, 0), 0.5);

	fs_out_col = vec4(color, 1);
	
	fs_out_col = texture(tex1, vec3(FS.texCoord, FS.texId.x));

	//fs_out_col = vec4(FS.instanceId * 0.001, 0, 0, 1);
	//fs_out_col = vec4(FS.texId.x* 0.0001,0,0,1);
	fs_out_col = vec4(0, FS.cubeSide * 0.3, 0, 1);
}

//texId
// Top texId.z
// ------
// |	|   Side: texId.y;
// |	|
// |	|
// ------
// bottom: texId.x
// texId.w = extra, for example block destruction