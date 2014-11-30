

uniform sampler2D tex0;

varying vec2 tcoord;
varying vec4 color;


void main() {


	vec4 col = texture2D(tex0, tcoord );

	//gl_FragColor = col;
	gl_FragColor = mix( vec4( 1, 0, 1, 1), col, 0.5) ;

}