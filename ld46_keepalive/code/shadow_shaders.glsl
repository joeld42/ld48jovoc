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
//  perform gamma correction
//
vec4 gamma(vec4 c) {
    float p = 1.0/2.2;
    return vec4(pow(c.xyz, vec3(p, p, p)), c.w);
}
@end

//------------------------------------------------------------------------------
//  Shadowmap pass shaders
//
@vs shadowVS
uniform vsParams {
    mat4 mvp;
};

in vec4 position;
out vec2 projZW;

void main() {
    gl_Position = mvp * position;
    projZW = gl_Position.zw;
}
@end

@vs shadowVSInstanced
uniform vsParams {
    mat4 mvp;
};

in vec4 position;
in vec4 instance0;      // instance transform xxxx
in vec4 instance1;      // instance transform yyyy
in vec4 instance2;      // instance transform zzzz
out vec2 projZW;

void main() {
    vec4 pos = position;
    vec4 xxxx = instance0;
    vec4 yyyy = instance1;
    vec4 zzzz = instance2;
    vec4 instPos = vec4(dot(pos, xxxx), dot(pos, yyyy), dot(pos, zzzz), 1.0);
    gl_Position = mvp * instPos;
    projZW = gl_Position.zw;
}
@end

@fs shadowFS
@include util
in vec2 projZW;
out vec4 fragColor;

void main() {
    float depth = projZW.x / projZW.y;
    fragColor = encodeDepth(depth);
    
    // DBG
    //fragColor = vec4( 1.0f, 0.0f, 1.0f, 1.0f );
}
@end

@program ShadowShader shadowVS shadowFS
@program ShadowShaderInstanced shadowVSInstanced shadowFS

//------------------------------------------------------------------------------
//  Color pass shaders
//
@block colorUniforms
uniform vsParams {
    mat4 model;
    mat4 mvp;
    mat4 lightMVP;
    vec3 diffColor;
};
@end

@vs colorVS
@include colorUniforms
in vec4 position;
in vec3 normal;
out vec3 color;
out vec4 lightProjPos;
out vec3 P;
out vec3 N;

void main() {
    gl_Position = mvp * position;
    lightProjPos = lightMVP * position;
    P = (model * position).xyz;
    N = (model * vec4(normal, 0.0)).xyz;
    color = diffColor;
}
@end

@vs colorVSInstanced
@include colorUniforms
in vec4 position;
in vec3 normal;
in vec4 instance0;      // instance transform xxxx
in vec4 instance1;      // instance transform yyyy
in vec4 instance2;      // instance transform zzzz
in vec4 color0;         // instance color
out vec3 color;
out vec4 lightProjPos;
out vec3 P;
out vec3 N;
void main() {
    vec4 pos = position;
    vec4 nrm = vec4(normal, 0.0);
    vec4 xxxx = instance0;
    vec4 yyyy = instance1;
    vec4 zzzz = instance2;

    // instPos and instNrm are in world space, the model part of
    // ModelViewProj is usually identity when rendering instanced
    vec4 instPos = vec4(dot(pos, xxxx), dot(pos, yyyy), dot(pos, zzzz), 1.0);
    vec4 instNrm = vec4(dot(nrm, xxxx), dot(nrm, yyyy), dot(nrm, zzzz), 0.0);
    gl_Position = mvp * instPos;
    lightProjPos = lightMVP * instPos;
    P = instPos.xyz;
    N = instNrm.xyz;
    color = color0.xyz * diffColor;
}
@end

@fs colorFS
@include util

uniform fsParams {
    vec2 shadowMapSize;
    vec3 lightDir;
    vec3 eyePos;
};
uniform sampler2D shadowMap;

in vec3 color;
in vec4 lightProjPos;
in vec3 P;
in vec3 N;
out vec4 fragColor;

void main() {
    float specPower = 16.0;
    float ambientIntensity = 0.25;

    // diffuse lighting
    vec3 l = lightDir;
    vec3 n = normalize(N);
    float n_dot_l = dot(n,l);
    if (n_dot_l > 0.0) {

        vec3 lightPos = lightProjPos.xyz / lightProjPos.w;
        vec2 smUV = (lightPos.xy+1.0)*0.5;
        float depth = lightPos.z;
        float s = sampleShadowPCF(shadowMap, smUV, shadowMapSize, depth);
        float diffIntensity = max(n_dot_l * s, 0.0);

        vec3 v = normalize(eyePos - P);
        vec3 r = reflect(-l, n);
        float r_dot_v = max(dot(r, v), 0.0);
        float specIntensity = pow(r_dot_v, specPower) * n_dot_l * s;

        fragColor = vec4(vec3(specIntensity, specIntensity, specIntensity) + (diffIntensity+ambientIntensity)*color, 1.0);
    }
    else {
        fragColor = vec4(color * ambientIntensity, 1.0);
    }
    fragColor = gamma(fragColor);
}
@end

@program ColorShader colorVS colorFS
@program ColorShaderInstanced colorVSInstanced colorFS

//------------------------------------------------------------------------------
//  Shadowmap Debug shaders
//
@vs debugShadowVS
uniform vsParams {
    vec2 size;
    vec2 offs;
};

in vec4 position;
in vec2 texcoord0;
out vec2 uv;
void main() {
    gl_Position = vec4( position.x * size.x + offs.x,
                        position.y * size.y + offs.y,
                        position.z, 1.0f );
    uv = texcoord0;
}
@end

@fs debugShadowFS
@include util
#define FLT_MAX 3.402823466e+38
#define M_PI 3.1415926535897932384626433832795
uniform sampler2D tex;
in vec2 uv;
out vec4 fragColor;
void main() {
    // scale back to 0..1
    vec4 dEnc = texture(tex, uv);
    float dist = decodeDepth( dEnc );
    
    
    float z = clamp( (dist - 0.96) / 0.04, 0.0, 1.0); // TODO make this dynamic or at lest settable
    
    //http://www.iquilezles.org/www/articles/palettes/palettes.htm
    vec3 a = vec3( 0.5, 0.5, 0.5 );
    vec3 b = vec3( 0.5, 0.5, 0.5 );
    vec3 c = vec3( 1.0, 1.0, 1.0 );
    vec3 d = vec3( 0.00, 0.10, 0.20);
    fragColor = vec4(a + b * cos(2.0*M_PI*(c*z + d) ), 1.0);
}
@end

@program DebugShadowShader debugShadowVS debugShadowFS

