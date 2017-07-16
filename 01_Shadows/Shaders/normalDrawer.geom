#version 330 core

layout (triangles) in;
layout (line_strip, max_vertices = 6) out;

//Interface Block
in VS_OUT 
{
	vec3 wFragPos;
	vec3 normal;
	vec2 texCoord;
	vec4 fragPosLightSpace4;
	vec4 testColor;
} GS[];

vec3 GetNormal()
{
   vec3 a = vec3(gl_in[0].gl_Position) - vec3(gl_in[1].gl_Position);
   vec3 b = vec3(gl_in[2].gl_Position) - vec3(gl_in[1].gl_Position);
   return normalize(cross(a, b));
}  

void DrawNormalLine(int index)
{
	const float MAGNITUDE = 0.6;

	vec4 begin = gl_in[index].gl_Position; //Built in variable ndc pos
	gl_Position = begin;
	EmitVertex();

	//mat3 normalMatrix = mat3(transpose(inverse(view * model)));
	//vs_out.normal = normalize(vec3(projection * vec4(normalMatrix * aNormal, 1.0)));

	vec4 end = begin + vec4(GetNormal(), 0.0) * MAGNITUDE; //NDC Normalnak ez nem jo!!! TODO
	gl_Position = end;
	EmitVertex();

	EndPrimitive();
}

void main()
{
	DrawNormalLine(0);
	//DrawNormalLine(1);
	//DrawNormalLine(2);
}