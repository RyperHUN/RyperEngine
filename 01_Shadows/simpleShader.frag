#version 430

in vec3 wFragPos;
in vec3 frag_normal;
in vec2 frag_tex;

out vec4 fs_out_col;

struct SpotLight {
    vec3  position;
    vec3  direction;
    float cutOff;
};   

//uniform samplerCube cubeTexture; TODO

uniform vec3 ka = vec3(0.2,0,0);
uniform vec3 kd = vec3(0.8,0,0);

uniform SpotLight spotlight;

vec3 calcSpotLight (SpotLight light)
{
	vec3 lightDir   = normalize(spotlight.position - wFragPos);
	float theta     = dot(lightDir, normalize(-spotlight.direction));
	
	vec3 color = vec3(0,0,0);
	if(theta > spotlight.cutOff * 0.95) 
	{
		float interp = smoothstep(spotlight.cutOff * 0.95,spotlight.cutOff,theta);
	    color = mix(vec3(0,0,0),kd,interp);// Do lighting calculations
	}
	return color;
}

void main()
{
	vec3 normal = normalize (frag_normal);
		
	fs_out_col = vec4(calcSpotLight (spotlight), 1);
	
	//fs_out_col = vec4(normal, 1.0);
}