attribute vec3 vertexPosition;
attribute vec2 vertexTCoord;

uniform mat4 mvp;

varying vec2 tcoord;


void main(void) 
{    
    gl_Position = mvp * vec4(vertexPosition, 1.0);
    tcoord = vertexTCoord;
}
