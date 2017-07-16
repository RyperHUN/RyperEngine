uniform mat4 rayDirMatrix;

shader SkyboxVS(in vec3 vs_in_pos : 0, out vec4 viewDir)
{
	vec4 pos = vec4( vs_in_pos.xy,0.9999, 1 );
	gl_Position = pos;
	viewDir = rayDirMatrix * pos;
};

uniform samplerCube skyBox;

shader SkyboxFS(in vec4 viewDir, out vec4 fs_out_col:0)
{
	//fs_out_col = vec4(normalize(viewDir.xyz),1);
	fs_out_col = texture(skyBox, normalize(viewDir.xyz));
};

program SkyBox
{
	vs(330)=SkyboxVS();
	fs(330)=SkyboxFS();
};