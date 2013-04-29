
//
//  Blocks.glsl
//
//  Created by joeld on 8/13/2012.
//  Copyright Tapnik 2012. All rights reserved.
//

-- Plastic.Vertex ------------------------------------------

// uniform params
uniform mat4 matrixPMV;
uniform mat4 matrixModelview;
uniform mat3 normalMatrix;

uniform vec3 lightDir0;
uniform vec3 lightColor0;

uniform vec3 Kd; // Diffuse color
uniform float roughness;

// per-vertex data
attribute vec4 position;
attribute vec4 color;
attribute vec4 texcoord;
attribute vec3 normal;

// outputs to fragment shader
varying vec3 diffuseColor;
varying vec3 P;
varying vec3 N;
varying vec2 st;
varying vec3 L0;

void main()
{
    // world space normal
	N = normalMatrix * normal;

	// directional light
	float df0 = max( 0.3, dot(N, lightDir0));

	// fresnel fake backlight
	//float sf = max( 0.0, dot(E, N2 ));
	//sf = (1.0 - pow( sf, 0.01 )) * 20.0;
		
	st = texcoord.st;
	
	vec4 posw = vec4( position.x, position.y, position.z, 1.0 );
	P = (matrixModelview * posw).xyz;
	gl_Position = matrixPMV * posw;
    diffuseColor = Kd * df0;
}


-- Plastic.Fragment ------------------------------------------

uniform vec4 dbgColor;
uniform vec3 lightPos0;

uniform float mixVal;

varying vec3 diffuseColor;
varying vec3 N;

varying vec3 P;
varying vec2 st;

uniform sampler2D sampler_dif0;

void main()
{
	vec4 dif0 = texture2D( sampler_dif0, st );
	gl_FragColor.a = dif0.a;

    vec3 lightPos0 = vec3( 0.0, 10.0, 0.0 );
    vec3 NN = normalize( N );
    vec3 L = normalize( lightPos0 - P);
    vec3 E = vec3( 0, 0, 1);
    vec3 H = normalize( L + E);

    float sf = max( 0.0, dot(NN,H) );
    sf = pow( sf, 30.0 );

    //gl_FragColor.rgb = vec3( 1,1,1 ) * dif0.a;
    gl_FragColor.a = 1.0;
    gl_FragColor.rgb = mix( diffuseColor, dif0.rgb, dif0.a*mixVal );

//    gl_FragColor.rgb = diffuseColor + vec3(1.0,1.0,1.0)*sf;

//    gl_FragColor.rgb = NN;

//    gl_FragColor = texture2D( sampler_dif0, st );
    
    //gl_FragColor.rgb = diffuseColor;

}
