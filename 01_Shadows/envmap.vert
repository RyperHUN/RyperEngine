#version 130

// VBO-ból érkezõ változók
in vec3 vs_in_pos;

out vec4 viewDir;

uniform mat4 rayDirMatrix;

void main()
{
	vec4 pos = vec4( vs_in_pos.xy,0.9999, 1 );
	gl_Position = pos;
	viewDir = pos * rayDirMatrix;
}