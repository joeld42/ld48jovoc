
uniform sampler2D tex0;
uniform sampler2D tex1;

varying vec2 tcoord;
varying vec4 color;
varying vec4 posLight;

void main() {
    vec4 texcolor = texture2D(tex0, tcoord);
    float shadow = texture2D(tex1, posLight.xy).x;


    // float luminosity = (texcolor.r + texcolor.g + texcolor.b) / 3.0;
    // vec4 gray = vec4(luminosity,luminosity,luminosity,1);
    // vec4 ocolor = vec4(texcolor.r, texcolor.g, texcolor.b, 1);
    // vec4 uvcolor = vec4(tcoord.x, tcoord.y, (tcoord.x+tcoord.y)*2.0, 1);
    // vec4 white = vec4(1, 1, 1, 0.6);
    // gl_FragColor = gray * ((tcoord.x+tcoord.y)/2.0);

    // vec4 shadColor = mix( vec4(1,0,0,1), vec4(0,0,1,1), shadow );
     float edge = clamp(
                    smoothstep( 0.9, 1.0, posLight.x ) +
                    smoothstep( -0.1, 0.0, -posLight.x ) + 
                    smoothstep( 0.9, 1.0, posLight.y ) + 
                    smoothstep( -0.1, 0.0, -posLight.y ), 0.0, 1.0 );

    vec4 shadColor = vec4( 0.4, 0.4, 0.5, 1.0)*texcolor;
    float shadVal = step( posLight.z, shadow+0.001);
    float shadMix = max( edge, shadVal);

    gl_FragColor = mix( shadColor, texcolor*color, shadMix );
    // gl_FragColor = color;
    
    //gl_FragColor = mix( shadColor, vec4(0,1,0,1), edge );
    // gl_FragColor.rg = posLight.xy;
    // gl_FragColor.b = shadow;
    // gl_FragColor.a = 1.0; 
    // gl_FragColor = (color * texcolor); //vec4(0,0.9,0.6,1); ;
    
    // gl_FragColor = vec4(gl_PointCoord.x, gl_PointCoord.y, 0, 1);
    // vec4 cc = uvcolor * gray;
    // gl_FragColor = texcolor;
}