#version 430

in VS_OUT 
{
	in vec3 wFragPos;
	in vec3 normal;
	in vec2 texCoord;
	in vec4 fragPosLightSpace4;
	in vec4 testColor;
} FS;


uniform sampler2D shadowMap;
uniform sampler2D texture_diffuse1;
uniform sampler2D texture_specular1;
uniform sampler2D texture_reflect1;

uniform samplerCube skyBox;

out vec4 fs_out_col;

///TODO Colors for lights
struct SpotLight {
    vec3  position;
    vec3  direction;
    float cutOff;
	vec3 color;
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

uniform vec3 ka = vec3(0.2,0,0);
uniform vec3 kd = vec3(0.6,0,0);
uniform vec3 ks = vec3(0.8,0.8,0.8);
uniform vec3 wEye;
uniform float shininess = 20.0f;

//TODO define only for max light, and upload used point light number
#define POINT_LIGHT_NUM 3

uniform SpotLight spotlight;
uniform DirLight dirlight;
uniform PointLight pointlight[POINT_LIGHT_NUM];

//TODO Specular
vec3 calcSpotLight (SpotLight light, vec3 wFragPos)
{
	vec3 lightDir = normalize(spotlight.position - wFragPos);
	float theta   = dot(lightDir, normalize(-spotlight.direction));
	
	vec3 color = vec3(0,0,0);
	vec3 texturedColor = light.color * kd * texture(texture_diffuse1, FS.texCoord).xyz ;
	if(theta > spotlight.cutOff * 0.96) 
	{
		float interp = smoothstep(spotlight.cutOff * 0.96,spotlight.cutOff,theta);
	    color = mix(vec3(0,0,0),texturedColor,interp);// Do lighting calculations
	}
	
	float dist = distance(wFragPos, spotlight.position);
	if (dist > 10.0)
	{
		color *= 10.0f / dist; //attenuation with distance
	}

	return color;
}

vec3 calcDirLight(DirLight light, vec3 normal, vec3 viewDir)
{
	vec3 toLight = normalize(-light.direction);

	float diff = max(dot(normal, toLight), 0.0);

	vec3 reflectDir = reflect(-toLight, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);

	vec3 diffuse  = diff * light.color * kd *texture(texture_diffuse1, FS.texCoord).xyz;
	vec3 specular = spec * light.color * ks *texture(texture_specular1, FS.texCoord).xyz;

	return diffuse + specular;
}

vec3 calcPointLight (PointLight light, vec3 normal, vec3 viewDir, vec3 wFragPos)
{
	vec3 toLight = normalize(light.position - wFragPos);
	float diff = max(dot(normal, toLight), 0.0);
	// Specular shading
    vec3 reflectDir = reflect(-toLight, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);

	float dist = length(light.position - wFragPos);
	//Lecsenges
	float attenuation = 1.0f / 
	(light.constant + light.linear * dist +  light.quadratic * (dist * dist));   

	vec3 diffuse  = light.color * diff * kd * texture(texture_diffuse1, FS.texCoord).xyz;
	vec3 specular = light.color * spec * ks * texture(texture_specular1, FS.texCoord).xyz;
	diffuse  *= attenuation; 
	specular *= attenuation;

	return diffuse + specular;
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
	vec3 normal  = normalize (FS.normal);
	vec3 viewDir = normalize (wEye - FS.wFragPos);
	vec3 reflectedDir   = reflect(-viewDir, normal);
	vec3 refractedDir   = refract(-viewDir, normal, 0.7);
	
	vec3 color = ka * texture(texture_diffuse1, FS.texCoord).xyz;
	float isShadow = ShadowCalculation(FS.fragPosLightSpace4);
	
	for(int i = 0; i < POINT_LIGHT_NUM; i++)
		color += calcPointLight(pointlight[i],normal,viewDir, FS.wFragPos);
	color += calcSpotLight (spotlight, FS.wFragPos);
	if(isShadow > 0.5)
	{
		color += calcDirLight (dirlight, normal, viewDir);
	}
	vec4 colorWLight    = vec4(color, 1.0);
	//fs_out_col   = colorWLight;
/////////////////////////////////////////////
//Reflection
	vec4 reflectedColor = vec4(texture(skyBox, reflectedDir).xyz, 1.0);
	vec4 refractedColor = vec4(texture(skyBox, refractedDir).xyz, 1.0);
	
	vec4 reflectedWithTex = vec4(reflectedColor * texture(texture_reflect1, FS.texCoord));
	
	//fs_out_col = vec4(mix(colorWLight.xyz, reflectedWithTex.xyz, 0.5) ,1.0);
	fs_out_col = colorWLight + reflectedWithTex;


	//fs_out_col = texture(texture_reflect1, FS.texCoord);
	//fs_out_col = vec4(abs(normal), 1.0);
	//fs_out_col = vec4(FS.texCoord.xy, 0, 1);
	//fs_out_col = vec4(FS.testColor,1);
}