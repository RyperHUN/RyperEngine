#version 130

// VBO-ból érkezõ változók
in vec3 vs_in_pos;
in vec3 vs_in_normal;
in vec2 vs_in_tex0;

// a pipeline-ban tovább adandó értékek
out vec3 vs_out_pos;
out vec3 vs_out_normal;
out vec2 vs_out_tex0;
out vec4 vs_out_lightspace_pos;

// shader külsõ paraméterei - most a három transzformációs mátrixot külön-külön vesszük át
uniform mat4 world;
uniform mat4 worldIT;
uniform mat4 MVP;
uniform mat4 shadowVP;

void main()
{
	gl_Position = MVP * vec4( vs_in_pos, 1 );

	vs_out_tex0 = vs_in_tex0;
	vs_out_normal	= (worldIT*vec4(vs_in_normal, 0)).xyz;
	vs_out_pos		= (world*vec4(vs_in_pos, 1)).xyz;
	vs_out_lightspace_pos = shadowVP*vec4(vs_out_pos, 1);
	vs_out_lightspace_pos /= vs_out_lightspace_pos.w;
	vs_out_lightspace_pos.xyz = (vec3(1,1,1) + vs_out_lightspace_pos.xyz)/2;
}