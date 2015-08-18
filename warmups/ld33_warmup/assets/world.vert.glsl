attribute vec3 vertexPosition;
attribute vec2 vertexTCoord;
attribute vec4 vertexColor;
attribute vec3 vertexNormal;

varying vec2 tcoord;
varying vec4 color;

uniform mat4 projectionMatrix;
uniform mat4 modelViewMatrix;

uniform mat4 model; //this is only for custom submissions

void main(void) {

	mat4 mvp = projectionMatrix * modelViewMatrix * model;
	vec4 i0 = model[0];
	vec4 i1 = model[1];
	vec4 i2 = model[2];
	vec4 i3 = model[3];
	mat4 mvpInv = mat4(
		vec4( i0.x, i1.x, i2.x, i3.x ),
		vec4( i0.y, i1.y, i2.y, i3.y ),
		vec4( i0.z, i1.z, i2.z, i3.z ),
		vec4( i0.w, i1.w, i2.w, i3.w )
	);
    gl_Position = mvp * vec4(vertexPosition, 1.0);
	vec4 N = mvpInv * vec4(vertexNormal, 0.0);

    tcoord = vertexTCoord;
    //color = vertexColor;
    color.xyz = N.xyz;
    color.a = 1.0;

    vec3 n = vertexNormal;
    gl_PointSize = 1.0;
}
