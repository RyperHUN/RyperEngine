#version 130

// pipeline-b�l bej�v� per-fragment attrib�tumok
in vec3 vs_out_pos;
in vec3 vs_out_normal;
in vec2 vs_out_tex0;
in vec4 vs_out_lightspace_pos;

// kimen� �rt�k - a fragment sz�ne
out vec4 fs_out_col;

//
// uniform v�ltoz�k
//
uniform vec3 light_dir = normalize( vec3(0, -1, -1) );

// sz�nt�r tulajdons�gok
uniform vec4 hemi_high	= vec4(0.2, 0.2, 0.2, 1);
uniform vec4 hemi_low	= vec4(0.2, 0.2, 0.2, 1);
uniform vec3 eye_pos;

// f�nytulajdons�gok
uniform vec4 Ld = vec4(0.4,0.4,0.4,1);
uniform vec4 Ls = vec4(1, 1, 1, 1);
uniform float	inCos	= 0.99f;
uniform float	outCos	= 0.86;

// anyagtulajdons�gok
uniform vec4 ka = vec4(1, 1, 1, 1);
uniform vec4 kd = vec4(1, 1, 1, 1);
uniform vec4 ks = vec4(1, 1, 1, 1);
uniform float specular_power = 16;
uniform sampler2D textureShadow;

uniform samplerCube texCube;

uniform float step_u = 1/1024.0f;
uniform float step_v = 1/1024.0f;
uniform int ksr = 1;

uniform float fresnel_term = 0.9;

void main()
{
	// normaliz�lni kell a bej�v� norm�list ui. az baricentrikus kombin�ci�b�l sz�rmazik!
	vec3 normal = normalize( vs_out_normal );

	// f�nyforr�s fel� mutat� vektor
	vec3 to_light = -light_dir;
	//
	// ambiens sz�n sz�m�t�sa
	//
	float nn = (normal.y + 1)/2;
	vec4 ambient = mix( hemi_low, hemi_high, nn );

	//
	// diff�z sz�n sz�m�t�sa
	//

	/* seg�ts�g:
		- normaliz�l�s: http://www.opengl.org/sdk/docs/manglsl/xhtml/normalize.xml
	    - skal�ris szorzat: http://www.opengl.org/sdk/docs/manglsl/xhtml/dot.xml
	    - clamp: http://www.opengl.org/sdk/docs/manglsl/xhtml/clamp.xml
	*/

	float diffuse_intensity = clamp( dot(normal, to_light) , 0, 1);
	vec4 diffuse = diffuse_intensity * Ld * kd;
	//
	// f�nyfoltk�pz� sz�n
	//

	/* seg�ts�g:
		- reflect: http://www.opengl.org/sdk/docs/manglsl/xhtml/reflect.xml
		- power: http://www.opengl.org/sdk/docs/manglsl/xhtml/pow.xml
	*/
	vec3 h = normalize(normal + to_light);
	vec3 to_eye = normalize( eye_pos - vs_out_pos );
	vec4 specular = vec4(0);
	if ( diffuse_intensity > 0 ) 
		specular = Ls*ks*pow( clamp( dot(h, to_eye), 0, 1), specular_power );
	
	// spot atten
	/*vec3 spot_dir = normalize( vs_out_pos - eye_pos );
	vec3 spot_pos = eye_pos;
	float cosDir = dot( to_light, -spot_dir );
	float atten  = clamp(smoothstep(outCos, inCos, cosDir), 0, 1);*/

	vec3 lightcoords = vs_out_lightspace_pos.xyz/vs_out_lightspace_pos.w;
	vec2 lightuv = lightcoords.xy;

	float cosTheta = clamp( dot( normal,to_light ), 0,1 );
	float bias = 0.01; 
	
	if ( lightuv == clamp(lightuv,0,1)  )
	{
		float visibility = 0;
		for (int i=-ksr; i<=ksr; ++i)
			for (int j=-ksr; j<=ksr; ++j)
			{
				float nearestToLight = 
					texture(textureShadow, 
							lightuv + vec2(step_u*i, step_v*j)).x;

				if ( nearestToLight + bias >= lightcoords.z )
					visibility += 1;
			}

		visibility /= (2*ksr+1)*(2*ksr+1);

		vec3 refl_dir = reflect(-to_eye, normal);
		vec3 refr_dir = refract(-to_eye, normal, 1.02);

		vec4 refl_col = texture(texCube, refl_dir);
		vec4 refr_col = texture(texCube, refr_dir);

		fs_out_col =	ambient + 
						visibility*(diffuse + specular)*( (1-fresnel_term)*refr_col + fresnel_term*refl_col );
	}
	else
		fs_out_col = vec4(1,0,0,0);
}