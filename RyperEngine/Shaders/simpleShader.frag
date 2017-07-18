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

struct Material {
	vec3 ka;
	vec3 kd;
	vec3 ks;
	float shininess;
};
vec3 wEye;	

uniform Material uMaterial;

//TODO define only for max light, and upload used point light number
#define POINT_LIGHT_NUM 3

uniform SpotLight uSpotlight;
uniform DirLight uDirlight;
uniform PointLight uPointlights[POINT_LIGHT_NUM];

//TODO Specular
vec3 calcSpotLight (SpotLight light, vec3 wFragPos, vec2 texCoord, Material mat)
{
	vec3 lightDir = normalize(light.position - wFragPos);
	float theta   = dot(lightDir, normalize(-light.direction));
	
	vec3 color = vec3(0,0,0);
	vec3 texturedColor = light.color * mat.kd * texture(texture_diffuse1, texCoord).xyz ;
	if(theta > light.cutOff * 0.96) 
	{
		float interp = smoothstep(light.cutOff * 0.96,light.cutOff,theta);
	    color = mix(vec3(0,0,0),texturedColor,interp);// Do lighting calculations
	}
	
	float dist = distance(wFragPos, light.position);
	if (dist > 10.0)
	{
		color *= 10.0f / dist; //attenuation with distance
	}

	return color;
}

vec3 calcDirLight(DirLight light, vec3 normal, vec3 viewDir, vec2 texCoord, Material mat)
{
	vec3 toLight = normalize(-light.direction);

	float diff = max(dot(normal, toLight), 0.0);

	vec3 reflectDir = reflect(-toLight, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), mat.shininess);

	vec3 diffuse  = diff * light.color * mat.kd *texture(texture_diffuse1, texCoord).xyz;
	vec3 specular = spec * light.color * mat.ks *texture(texture_specular1, texCoord).xyz;

	return diffuse + specular;
}

vec3 calcPointLight (PointLight light, vec3 normal, vec3 viewDir, vec3 wFragPos, vec2 texCoord, Material mat)
{
	vec3 toLight = normalize(light.position - wFragPos);
	float diff = max(dot(normal, toLight), 0.0);
	// Specular shading
    vec3 reflectDir = reflect(-toLight, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), mat.shininess);

	float dist = length(light.position - wFragPos);
	//Lecsenges
	float attenuation = 1.0f / 
	(light.constant + light.linear * dist +  light.quadratic * (dist * dist));   

	vec3 diffuse  = light.color * diff * mat.kd * texture(texture_diffuse1, texCoord).xyz;
	vec3 specular = light.color * spec * mat.ks * texture(texture_specular1, texCoord).xyz;
	diffuse  *= attenuation; 
	specular *= attenuation;

	return diffuse + specular;
}

float ShadowCalcWithPcf(vec4 fragPosLightSpace)
{
	// perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // Transform to [0,1] range for texture coord
    projCoords = projCoords * 0.5 + 0.5;
	//Percentage Closer Filtering
	float shadowMapSizePixel = 4096.0;
	float texelSize = 1.0 / shadowMapSizePixel;
	int pcfSize = 3; //TODO Uniform

	float bias = 0.005;
	int sumShadowPixels = (pcfSize * 2 + 1) * (pcfSize * 2 + 1);
	float notInShadowTexel = 0;
	float currentDepth = projCoords.z;
	for(int i = -pcfSize; i <= pcfSize; i++)
	{
		for(int j = -pcfSize; j <= pcfSize; j++)
		{
			vec2 offset = vec2(i, j) * texelSize;
			// Get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
			float shadowDepth = texture(shadowMap, projCoords.xy + offset).r; 
			if(currentDepth - bias < shadowDepth)
				notInShadowTexel+= 1;
		}
	}
	return float(notInShadowTexel) / float(sumShadowPixels);
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
	
	vec3 color = uMaterial.ka * texture(texture_diffuse1, FS.texCoord).xyz;
	
	for(int i = 0; i < POINT_LIGHT_NUM; i++)
		color += calcPointLight(uPointlights[i],normal,viewDir, FS.wFragPos, FS.texCoord, uMaterial);
	color += calcSpotLight (uSpotlight, FS.wFragPos, FS.texCoord, uMaterial);

	//float lightValue = ShadowCalculation(FS.fragPosLightSpace4);
	float lightValue = ShadowCalcWithPcf (FS.fragPosLightSpace4);
	color += calcDirLight (uDirlight, normal, viewDir, FS.texCoord, uMaterial) * lightValue;
	
	vec4 colorWLight    = vec4(color, 1.0);
	//fs_out_col   = colorWLight;
/////////////////////////////////////////////
//Reflection
	vec4 reflectedColor = vec4(texture(skyBox, reflectedDir).xyz, 1.0);
	vec4 refractedColor = vec4(texture(skyBox, refractedDir).xyz, 1.0);
	
	vec4 reflectedWithTex = vec4(reflectedColor * texture(texture_reflect1, FS.texCoord));
	
	//fs_out_col = vec4(mix(colorWLight.xyz, reflectedWithTex.xyz, 0.5) ,1.0);
	fs_out_col = colorWLight + reflectedWithTex;
	//fs_out_col = vec4(lightValue, 0,0,1);

	//fs_out_col = texture(texture_reflect1, FS.texCoord);
	//fs_out_col = vec4(abs(normal), 1.0);
	//fs_out_col = vec4(FS.texCoord.xy, 0, 1);
	//fs_out_col = vec4(FS.testColor,1);
}