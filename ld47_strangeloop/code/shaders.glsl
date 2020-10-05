//------------------------------------------------------------------------------
//  float/rgba8 encoding/decoding so that we can use an RGBA8
//  shadow map instead of floating point render targets which might
//  not be supported everywhere
//
//  http://aras-p.info/blog/2009/07/30/encoding-floats-to-rgba-the-final/
//
@block util
vec4 encodeDepth(float v) {
    vec4 enc = vec4(1.0, 255.0, 65025.0, 160581375.0) * v;
    enc = fract(enc);
    enc -= enc.yzww * vec4(1.0/255.0,1.0/255.0,1.0/255.0,0.0);
    return enc;
}

float decodeDepth(vec4 rgba) {
    return dot(rgba, vec4(1.0, 1.0/255.0, 1.0/65025.0, 1.0/160581375.0));
}

//------------------------------------------------------------------------------
//  perform simple shadow map lookup returns 0.0 (unlit) or 1.0 (lit)
//
float sampleShadow(sampler2D shadowMap, vec2 uv, float compare) {
    #if !ORYOL_GLSL
    uv.y = 1.0-uv.y;
    #endif
    
    float depth = decodeDepth(texture(shadowMap, vec2(uv.x, uv.y)));
    depth += 0.001;
    return step(compare, depth);
}

//------------------------------------------------------------------------------
//  perform percentage-closer shadow map lookup
//
float sampleShadowPCF(sampler2D shadowMap, vec2 uv, vec2 smSize, float compare) {
    float result = 0.0;
    for (int x=-2; x<=2; x++) {
        for (int y=-2; y<=2; y++) {
            vec2 off = vec2(x,y)/smSize;
            result += sampleShadow(shadowMap, uv+off, compare);
        }
    }
    return result / 25.0;
}

//------------------------------------------------------------------------------
//  world lighting
//
vec4 applyShadow( sampler2D shadowMap, vec4 baseColor, vec4 lightProjPos ) {
    
    vec3 lightPos = lightProjPos.xyz / lightProjPos.w;
    vec2 smUV = (lightPos.xy+1.0)*0.5;
    float depth = clamp( lightPos.z, 0.0, 1.0 );
    
    float bounds = step( 0.0, smUV.x ) *
                    step( 0.0, smUV.y ) *
                    step( smUV.x, 1.0 ) *
                    step( smUV.y, 1.0 );
    
    float s = sampleShadowPCF(shadowMap, smUV, vec2(2048, 2048), depth);
    //    float s = sampleShadow( shadowMap, smUV, depth);
    
    vec3 shadowedColor = baseColor.xyz * vec3(0.5, 0.55, 0.6);
    vec3 litColor = mix( shadowedColor, baseColor.xyz, s);
    
    return vec4( mix( baseColor.xyz, litColor, bounds ), baseColor.a );
}


//------------------------------------------------------------------------------
//  perform gamma correction
//
vec4 gamma(vec4 c) {
    float p = 1.0/2.2;
    return vec4(pow(c.xyz, vec3(p, p, p)), c.w);
}
@end

//------------------------------------------------------------------------------
//  World Shader
//------------------------------------------------------------------------------
@vs worldVS
uniform vsParams {
    mat4 modelview;
    mat4 mvp;
    mat4 lightMVP;
    vec4 decalTint;
    vec4 tintColor;
    vec4 checkColor;
};

in vec4 position;
in vec4 normal;
in vec2 texcoord0;
in vec2 texcoord1;
in vec4 color0;

out vec2 uv;
out vec4 color;
out vec4 nrm_ws;
out vec4 lightProjPos;
out vec4 fragP;
out vec4 fragN;
out vec4 edgeCode;

//out vec4 nrm_ws;
//out vec4 lightProjPos;
//out vec4 lightDir;
//out vec4 world_nrm;

void main() {
    fragP = gl_Position = mvp * position;
    lightProjPos = lightMVP * position;
    //lightDir = vec4( normalize(vec3( 0.4, 0.1, 1.0)), 0.0);

    uv = texcoord0;
    //uvDecal = texcoord1;
    //world_nrm = normal;
    vec4 nrm4 = vec4(normal.xyz,0);
    vec3 nrm = normalize(mvp * nrm4).xyz;
    nrm_ws = normalize( modelview * nrm4 );
	fragN = mvp * vec4( normal.xyz,0);
    
    float fresnel = pow( 1.0 - clamp( dot (nrm, vec3( 0.0, 0.0, -1.0)), 0.0, 1.0), 10.0 );
    vec4 tintColor2 = tintColor + vec4( vec3( 0.2, 1, 0.2) * fresnel * 10, 0.0);
    color = mix( tintColor2, checkColor, texcoord1.x );
    //color = vec4( fract(texcoord1.x), texcoord1.y, 1.0, 1.0 );
    //decalTintColor = decalTint;
    
    //lightDir = (vec4( normalize(vec3( 0.4, 0.1, 1.0)), 0.0) * transpose(lightMVP) ).xyz;
    
    //edgeCode = texcoord1.x;
    edgeCode = color0;
}
@end


@fs worldFS
@include util
uniform fsParams {
    vec2 decalUVOffs;
    vec3 lightDir;
};
uniform sampler2D tex;
uniform sampler2D edgeTex;
uniform sampler2D shadowMap;
//uniform sampler2D texDecal;

in vec2 uv;
in vec4 fragP;
in vec4 fragN;
in vec4 color;
in vec4 nrm_ws;
in vec4 lightProjPos;
in vec4 edgeCode;

//in vec4 lightDir;
//in vec4 world_nrm;

//out vec4 fragColor; 
layout(location=0) out vec4 fragColor0;
layout(location=1) out vec4 gbuffColor;

void main() {
    vec4 c = texture(tex, uv );
    
//    if (c.a < 0.2f) {
//        discard;
//    }

    //vec4 cDecal = texture(texDecal, uvDecal );

    vec3 cBaseColor = c.xyz*color.xyz;
    
    //vec3 cDecalColor = cDecal.xyz * decalTintColor.xyz;
    //vec3 cResult = mix( cBaseColor, cDecalColor, cDecal.a * decalTintColor.a );
	
    
	float nDotL = clamp( dot( lightDir, nrm_ws.xyz ), 0, 1);
    vec3 lightCol = mix( vec3(0.1,0.12,0.25) * 4, vec3( 0.35, 0.1, 0.24) * 3, nDotL ) + vec3( 0.2, 0.5, 0.3);
    //fragColor0 = applyShadow( shadowMap, vec4(cBaseColor * lightCol, 1.0), lightProjPos );
	
    float sz = 0.085;
    vec2 uv2 = mod( uv * 8.0, 1 );
    vec2 edgeUv =  uv2 * vec2( 0.25, 1.0 ); // squash to 0.25
    edgeUv = clamp( edgeUv, vec2( 0.001f, 0.0f), vec2( 0.249f, 1.0f) );
        
    if (uv2.y < sz) {
        edgeUv += vec2( edgeCode.x * 0.25, 0.0 );
    } else if (uv2.y > (1.0-sz) ) {
        edgeUv += vec2( edgeCode.z * 0.25, 0.0 );
    }else if (uv2.x < sz) {
        edgeUv += vec2( edgeCode.y * 0.25, 0.0 );
    } else if (uv2.x > (1.0-sz) ) {
        edgeUv += vec2( edgeCode.w * 0.25, 0.0 );
    }
//    fragColor0 = vec4( mix(cBaseColor, edgeCol, clamp(edgeAmt, 0.0, 1.0) ), 1.0 );
  

  vec4 edgeCol = texture( edgeTex, edgeUv );
  
  vec3 surfColor = mix(cBaseColor, edgeCol.rgb, edgeCol.a );
  
  //fragColor0 = applyShadow( shadowMap, vec4(surfColor * lightCol, 1.0), lightProjPos );
  fragColor0 = vec4(surfColor * lightCol, 1.0) * 1.2;
    
	
	//gbuffColor = vec4( vec3( abs(sin(fragP.z)) ), 1.0 ) ;
	//gbuffColor = fragP;
    //fragColor0 = vec4(lightDir,1);

    //fragColor = applyShadow( shadowMap, vec4( lightCol, 1.0), lightProjPos );
    
    //fragColor = vec4( (lightDir.xyz * 0.5) + vec3(0.5), 1.0 );
    //fragColor = vec4( cBaseColor * lightCol, 1.0 );
}
@end

@program WorldShader worldVS worldFS

