#version 430

in vec3 wFragPos;
in vec3 frag_normal;
in vec2 frag_tex;

out vec4 fs_out_col;

struct SpotLight {
    vec3  position;
    vec3  direction;
    float cutOff;
	float outerCutOff;
};   

//uniform samplerCube cubeTexture; TODO

uniform SpotLight spotlight;

void main()
{
	vec3 normal = normalize (frag_normal);
	vec3 lightDir   = normalize(spotlight.position - wFragPos);
	float theta     = dot(lightDir, normalize(-spotlight.direction));
	
	vec3 ka = mix(vec3(0.2,0,0), normal, 0.1);
	vec3 kd = mix(vec3(1.0,0,0), normal, 0.1);
	if(theta > spotlight.cutOff * 0.95) 
	{
		float interp = smoothstep(spotlight.cutOff * 0.95,spotlight.cutOff,theta);
	   fs_out_col = vec4(mix(ka,kd,interp),1);// Do lighting calculations
	}
	else
	   fs_out_col = vec4(ka,1);

	//fs_out_col = vec4(normal, 1.0);
}