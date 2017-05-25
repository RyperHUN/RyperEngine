#version 430

in vec2 fragTex;

uniform sampler2D loadedTex;

out vec4 fs_out_col;

void main()
{             
	fs_out_col = texture(loadedTex, fragTex);
	//fs_out_col = vec4(fragTex.xy, 0,1);
	//fs_out_col = vec4(1,0,0,1);
    // gl_FragDepth = gl_FragCoord.z;
}  