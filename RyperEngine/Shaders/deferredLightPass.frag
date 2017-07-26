#version 430

in vec2 fragTex;

out vec4 fs_out_col;

///////////////////////////////////
////////--------STRUCTS-----------
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

///////////////////////////////////
////////--------UNIFORMS-----------

uniform sampler2D tex_pos;
uniform sampler2D tex_normal;
uniform sampler2D tex_color;
uniform vec3 uwEye;

//TODO define only for max light, and upload used point light number
#define POINT_LIGHT_NUM 3

uniform SpotLight uSpotlight;
uniform DirLight uDirlight;
uniform PointLight uPointlights[POINT_LIGHT_NUM];

///////////////////////////////////
////////--------FUNCTIONS-----------

vec3 calcDirLight(DirLight light, vec3 normal, vec3 viewDir, vec2 texCoord, vec3 objColor)
{
	vec3 toLight = normalize(-light.direction);

	float diff = max(dot(normal, toLight), 0.0);

	vec3 reflectDir = reflect(-toLight, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 20);

	vec3 diffuse  = diff * light.color * objColor;
	vec3 specular = spec * light.color * objColor;

	return diffuse + specular;
}

vec3 calcPointLight (PointLight light, vec3 normal, vec3 viewDir, vec3 wFragPos, vec2 texCoord, vec3 objColor)
{
	vec3 toLight = normalize(light.position - wFragPos);
	float diff = max(dot(normal, toLight), 0.0);
	// Specular shading
    vec3 reflectDir = reflect(-toLight, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 20);

	float dist = length(light.position - wFragPos);
	//Lecsenges
	float attenuation = 1.0f / 
	(light.constant + light.linear * dist +  light.quadratic * (dist * dist));   

	vec3 diffuse  = light.color * diff * objColor;
	diffuse  *= attenuation;

	return diffuse;
}

void main()
{
	vec2 texCoord = vec2(fragTex.x, 1 - fragTex.y);
	vec3 wFragPos = texture(tex_pos, texCoord).xyz;
	vec3 objColor = texture(tex_color, texCoord).xyz;

	vec3 normal  = texture(tex_normal, texCoord).xyz;
	vec3 viewDir = normalize (uwEye - wFragPos);
	
	
	vec3 color = objColor * 0.1;
	
	for(int i = 0; i < POINT_LIGHT_NUM; i++)
		color += calcPointLight(uPointlights[i],normal,viewDir, wFragPos, texCoord, objColor);

	//color += calcDirLight (uDirlight, normal, viewDir, texCoord, objColor);
	
	fs_out_col   = vec4(color, 1.0);
}