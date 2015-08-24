
uniform sampler2D tex0;
uniform sampler2D tex1;
uniform sampler2D tex2;

uniform float shiny;

varying vec2 tcoord;
varying vec4 color;
varying vec4 posLight;
varying vec2 vN;

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

    // matcap
    vec3 base = texture2D( tex2, vN ).rgb * shiny;
    
    vec3 litColor = color.rgb * texcolor.rgb;

    gl_FragColor = vec4(base + litColor, 1.);
    //gl_FragColor.a = 1.5;
}