#version 430

in vec2 fragTex;

uniform sampler2D loadedTex;


out vec4 fs_out_col;

void main()
{             
	fs_out_col = texture(loadedTex, fragTex);
    // gl_FragDepth = gl_FragCoord.z;
}  