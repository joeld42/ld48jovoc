
uniform sampler2D tex0;
uniform sampler2D tex1;

varying vec2 tcoord;
varying vec4 color;
varying vec4 posLight;

void main() {
    vec4 texcolor = texture2D(tex0, tcoord);

    /*
    float shadow = texture2D(tex1, posLight.xy).x;
    
    // shadow boundry
    float edge = clamp(
                    smoothstep( 0.9, 1.0, posLight.x ) +
                    smoothstep( -0.1, 0.0, -posLight.x ) + 
                    smoothstep( 0.9, 1.0, posLight.y ) + 
                    smoothstep( -0.1, 0.0, -posLight.y ), 0.0, 1.0 );

    vec4 shadColor = vec4( 0.4, 0.4, 0.5, 1.0)*texcolor;
    float shadVal = step( posLight.z, shadow+0.001);
    float shadMix = max( edge, shadVal);

    gl_FragColor = mix( shadColor, texcolor*color, shadMix );
    */

    gl_FragColor = color*texcolor;
}