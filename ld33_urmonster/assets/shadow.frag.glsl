
uniform sampler2D tex0;

varying vec2 tcoord;

void main() {
    vec4 texcolor = texture2D(tex0, tcoord);
    gl_FragColor = texcolor;
}