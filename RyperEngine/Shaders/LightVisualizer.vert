#version 430
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 vsNormal;
layout (location = 2) in vec2 tex;

uniform mat4 PVM;

out vec2 texCoord;
out vec3 normal;

void main()
{
    gl_Position = PVM * vec4(position, 1.0f);
	texCoord = tex;
	normal = vsNormal;
}  