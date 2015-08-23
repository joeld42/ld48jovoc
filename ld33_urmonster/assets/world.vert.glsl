attribute vec3 vertexPosition;
attribute vec2 vertexTCoord;
attribute vec4 vertexColor;
attribute vec3 vertexNormal;

varying vec2 tcoord;
varying vec4 color;
varying vec4 posLight;

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
    color.xyz = ((lightColor * clamp(dot(N.rgb, lightDir), 0.0, 1.0) + ambientColor) * 1.8) * tintColor;

    color.a = 1.0;

    vec3 n = vertexNormal;
    gl_PointSize = 1.0;
}
