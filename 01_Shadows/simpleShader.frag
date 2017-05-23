#version 430

in vec3 wFragPos;

out vec4 fs_out_col;

struct SpotLight {
    vec3  position;
    vec3  direction;
    float cutOff;
};   

//uniform samplerCube cubeTexture; TODO

uniform SpotLight spotlight;

void main()
{
	vec3 lightDir = normalize(spotlight.position - wFragPos);
	float theta = dot(lightDir, normalize(-spotlight.direction));
    
	if(theta > spotlight.cutOff) 
	{       
	   fs_out_col = vec4(1,0,0,1);// Do lighting calculations
	}
	else  // else, use ambient light so scene isn't completely dark outside the spotlight.
	   fs_out_col = vec4(0.2,0,0,1);

	//fs_out_col = vec4(spotlight.direction, 1.0);
}