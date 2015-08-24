attribute vec3 vertexPosition;
attribute vec2 vertexTCoord;
attribute vec4 vertexColor;
attribute vec3 vertexNormal;

varying vec2 tcoord;
varying vec4 color;
varying vec4 unlitColor;
varying vec4 posLight;

varying vec2 vN;

uniform mat4 projectionMatrix;
uniform mat4 modelViewMatrix;

uniform mat4 mvp;
uniform mat4 mvpLight;
uniform mat4 normalMatrix;

uniform vec3 tintColor;
uniform vec3 lightDir;

uniform mat4 model; //this is only for custom submissions

void main(void) 
{    
    gl_Position = mvp * vec4(vertexPosition, 1.0);

	vec4 N = normalMatrix * vec4(vertexNormal, 0.0);

	vec4 posLight1 = (mvpLight * vec4(vertexPosition, 1.0 ));	
	posLight = vec4( posLight1.x * 0.5 + 0.5, 
					 posLight1.y * 0.5 + 0.5,
					 posLight1.z * 0.5 + 0.5, 1.0 );

    tcoord = vertexTCoord;
    //color = vertexColor;
    // TODO make param
    vec3 lightColor = vec3( 1.1, 1.05, 1.0 );
    vec3 ambientColor = vec3( 0.1, 0.1, 0.15 );
    float nDotL = clamp(dot(N.rgb, lightDir), 0.0, 1.0);
    color = vec4( ((lightColor * nDotL + ambientColor) * 2.0) * tintColor, 1.0 );

    vec3 ambientColor2 = vec3( 0.4, 0.2, 0.1 );
    unlitColor = vec4( mix(ambientColor, ambientColor2, nDotL) * 1.4 * tintColor, 1.0 );

    // reflect for matcap
    vec4 p = vec4( vertexPosition, 1. );

    vec3 e = normalize( vec3( modelViewMatrix * p ) );
    vec3 r = reflect( e, N.rgb );
    float m = 2. * sqrt( 
        pow( r.x, 2. ) + 
        pow( r.y, 2. ) + 
        pow( r.z + 1., 2. ) 
    );
    vN = r.xy / m + .5;

    vec3 n = vertexNormal;
    gl_PointSize = 1.0;
}
