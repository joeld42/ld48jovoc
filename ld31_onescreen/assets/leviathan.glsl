-- base.vertex

attribute vec3 vertexPosition;
attribute vec2 vertexTCoord;
attribute vec4 vertexColor;
attribute vec3 vertexNormal;

varying vec2 tcoord;
varying vec4 color;
varying vec3 nrm;
varying vec4 pos;

uniform mat4 projectionMatrix;
uniform mat4 modelViewMatrix;


void main(void) {

	pos = vec4(vertexPosition, 1.0);
	nrm = vertexNormal;

    gl_Position = projectionMatrix * modelViewMatrix * pos;

    // simple lighting
    //vec3 liteDir = normalize(vec3( -0.5, 1.0, 0.0) );
    vec3 liteDir = vec3( -0.4472135955, 0.894427191, 0 );
    vec3 keyLite = clamp(dot( nrm, liteDir ), 0.1, 1.0) * 
    			   vec3(0.13828125, 0.25078125, 0.271875); // key color

    // vec3 liteDirFill = normalize(vec3( 1.0, 0.2, 0.5) );
    vec3 liteDirFill = vec3(  0.8804509063, 0.1760901813, 0.4402254532 );
    vec3 fillLite = clamp(dot( nrm, liteDir ), 0.1, 1.0) * 
    			    vec3(0.2, 0.1046875, 0.04609375); // fill color

    vec3 lite = fillLite + keyLite;

    tcoord = vertexTCoord;
    color = vec4( lite.x, lite.y, lite.z, 1.0);    
}

-- base.fragment
uniform sampler2D tex0;

varying vec2 tcoord;
varying vec4 color;
varying vec3 nrm;
varying vec4 pos;

void main() {

	vec4 col = texture2D(tex0, tcoord );

	gl_FragColor = (col*0.5) + color;	
	// gl_FragColor = vec4( nrm.x, nrm.y, nrm.z, 1.0 );

}

-- ghost.fragment
uniform float tval;

uniform sampler2D tex0;

varying vec2 tcoord;
varying vec4 color;
varying vec3 nrm;
varying vec4 pos;

void main() {

	vec4 col = texture2D(tex0, tcoord );

	 vec3 gridv = vec3( mod((pos.x*-6.0) + tval*0.25, 1.0),
	  			      mod((pos.y*-6.0) + tval*0.25, 1.0),
	  			      mod((pos.z*-6.0) + tval*0.25, 1.0) );
	 float grid = max( max( gridv.z, gridv.y ), gridv.x );

	//float grid = mod( (pos.y * 4.0) + tval, 1.0);

	vec3 glowc = vec3( 0.07421875, 0.3515625, 0.6328125) * (grid + col.r * 0.2);
	vec3 wirec = vec3(0.54296875, 0.7578125, 0.9375) * smoothstep( 0.94, 0.95, grid );
	glowc = glowc + wirec;

	gl_FragColor = vec4( glowc.r, glowc.g, glowc.b, 1.0 );
	//gl_FragColor = vec4( tval, col.g, grid, 1.0 );

}