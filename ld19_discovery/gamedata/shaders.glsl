-- Treeland.Vertex -------------
uniform mat4 matrixPMV;

attribute vec4 position;
attribute vec4 texcoord;

varying vec2 st;

void main()
{
	st = texcoord.st;
	gl_Position = matrixPMV * vec4( position.x, position.z, position.y, 1.0 );
}

-- Treeland.Fragment ---------------
uniform sampler2D sampler_dif0;
uniform sampler2D sampler_nrm0;

uniform vec3 lightPos0;

varying vec2 st;
void main()
{
	gl_FragColor = texture2D( sampler_dif0, st );
	//gl_FragColor = vec4( st.x, 0.0, st.y, 1.0 );
}