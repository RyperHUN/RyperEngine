#version 430

in vec3 wFragPos;
in vec3 frag_normal;
in vec2 frag_tex;
in vec4 fragPosLightSpace4;
uniform sampler2D shadowMap;

out vec4 fs_out_col;

///TODO Colors for lights
struct SpotLight {
    vec3  position;
    vec3  direction;
    float cutOff;
}; 

struct DirLight {
    vec3 direction;

	vec3 color;
};

struct PointLight {    
    vec3 position;
    
    float constant;
    float linear;
    float quadratic;  

	vec3 color;
};  

//uniform samplerCube cubeTexture; TODO

uniform vec3 ka = vec3(0.2,0,0);
uniform vec3 kd = vec3(0.8,0,0);

#define POINT_LIGHT_NUM 3

uniform SpotLight spotlight;
uniform DirLight dirlight;
uniform PointLight pointlight[POINT_LIGHT_NUM];
uniform vec3 wEye;

vec3 calcSpotLight (SpotLight light)
{
	vec3 lightDir   = normalize(spotlight.position - wFragPos);
	float theta     = dot(lightDir, normalize(-spotlight.direction));
	
	vec3 color = vec3(0,0,0);
	if(theta > spotlight.cutOff * 0.96) 
	{
		float interp = smoothstep(spotlight.cutOff * 0.96,spotlight.cutOff,theta);
	    color = mix(vec3(0,0,0),kd,interp);// Do lighting calculations
	}

	float dist = distance(wFragPos, spotlight.position);

	if (dist > 10.0)
	{
		color *= 10.0f / dist; //attenuation with distance
	}

	return color;
}

vec3 calcDirLight(DirLight light, vec3 normal)
{
	vec3 toLight = normalize(-light.direction);

	float diff = max(dot(normal, toLight), 0.0);

	///TODO Specular shading
	//vec3 reflectDir = reflect(-lightDir, normal);
    //   float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);

	light.color = vec3(1,1,1); //TODO
	vec3 diffuse = diff * light.color * kd;
	
	return diffuse;
}

vec3 calcPointLight (PointLight light, vec3 normal)
{
	vec3 toLight = normalize(light.position - wFragPos);
	float diff = max(dot(normal, toLight), 0.0);

	float dist = length(light.position - wFragPos);
	//Lecsenges
	float attenuation = 1.0f / 
	(light.constant + light.linear * dist +  light.quadratic * (dist * dist));   

	light.color = vec3(1,1,1);
	vec3 diffuse = light.color * diff * kd;
	diffuse *= attenuation; 

	return diffuse;
}

float ShadowCalculation(vec4 fragPosLightSpace)
{
    // perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // Transform to [0,1] range for texture coord
    projCoords = projCoords * 0.5 + 0.5;
    // Get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float shadowDepth = texture(shadowMap, projCoords.xy).r; 
    // Get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;
    // Check whether current frag pos is in shadow
	float bias = 0.005;
    if(currentDepth - bias < shadowDepth)
		return 1.0;
	else
		return 0.0;
}  

void main()
{
	vec3 normal = normalize (frag_normal);
	
	vec3 color = ka;
	float isShadow = ShadowCalculation(fragPosLightSpace4);
	
	for(int i = 0; i < POINT_LIGHT_NUM; i++)
		color += calcPointLight(pointlight[i],normal);
	color += calcSpotLight (spotlight);
	if(isShadow > 0.5)
	{
		color += calcDirLight (dirlight, normal);
	}
	fs_out_col = vec4(color, 1.0);
	//fs_out_col = vec4(normal, 1.0);
	//fs_out_col = vec4(frag_tex.xy, 0, 1);
}