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

uniform vec3 eyeDir;
uniform vec3 lightDir0;

varying vec2 st;
void main()
{

	vec3 nrm1 = texture2D( sampler_nrm0, st );
	vec3 nrm = normalize( (nrm1 * 2.0) - 1.0 );		
	
	vec3 dif0 = texture2D( sampler_dif0, st ).rgb * 2.0; // doublebright	
	vec3 halfAng = normalize( lightDir0 + eyeDir );
	
	float diff = 0.35 + max( 0.0, dot( nrm, lightDir0 ));
	float spec = clamp( dot( nrm, halfAng ), 0.0, 1.0);
	spec = pow( spec, 3 );
	
	
	vec3 lightColor = vec3( 1.0, 1.0, 1.0 );
	
	gl_FragColor = vec4( dif0*diff*lightColor + spec, 1 );
	
	// DBG
	//gl_FragColor = vec4( diff, diff, diff, 1 );	
	
}