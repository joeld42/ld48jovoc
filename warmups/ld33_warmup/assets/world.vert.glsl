attribute vec3 vertexPosition;
attribute vec2 vertexTCoord;
attribute vec4 vertexColor;
attribute vec3 vertexNormal;

varying vec2 tcoord;
varying vec4 color;

uniform mat4 projectionMatrix;
uniform mat4 modelViewMatrix;

uniform mat4 mvp;
uniform mat4 normalMatrix;

uniform vec3 tintColor;
uniform vec3 lightDir;

uniform mat4 model; //this is only for custom submissions

void main(void) 
{
    gl_Position = mvp * vec4(vertexPosition, 1.0);
	vec4 N = normalMatrix * vec4(vertexNormal, 0.0);

    tcoord = vertexTCoord;
    //color = vertexColor;
    color.xyz = tintColor * dot(N.rgb, lightDir);
    //color.xyz = N.rgb;
    color.a = 1.0;

    vec3 n = vertexNormal;
    gl_PointSize = 1.0;
}
