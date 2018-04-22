//------------------------------------------------------------------------------
//  Vertex and fragment shaders for rendering into the frame buffer.
//
@vs displayVS
uniform vsParams {
    mat4 mvp;
    vec4 tintColor;
};

in vec4 position;
in vec4 normal;
in vec2 texcoord0;
out vec2 uv;
out vec4 nrm;
out vec4 color;
out vec4 world_nrm;

void main() {
    gl_Position = mvp * position;
    uv = texcoord0;
    world_nrm = normal;
    nrm = normalize(mvp * normal);
    color = tintColor;
}
@end

@fs displayFS
uniform sampler2D tex;
in vec2 uv;
in vec4 nrm;
in vec4 world_nrm;
in vec4 ldir;
in vec4 color;
out vec4 fragColor;

float gamma = 2.2;

vec3 simpleReinhardToneMapping(vec3 color)
{
    float exposure = 2.5;
    color *= exposure/(1. + color / exposure);
    color = pow(color, vec3(1. / gamma));
    return color;
}

void main() {
    vec4 tex = texture(tex, uv);
    vec4 c = tex * color;
    //vec4 c = vec4(1.0) - vec4( uv.x, uv.y, 0.0, 1.0 );
    vec3 ldir = vec3( -1.0, 0.0, 0.3 );
    vec3 ldir2 = vec3( 1.0, 0.5, 0.4 );
    float lite1 = clamp(pow( dot(nrm.xyz, normalize(ldir.xyz)), 5.0 ), 0.0, 1.0) * 5.0;
    float lite2 = clamp(dot(nrm.xyz, normalize(ldir2.xyz)), 0.0, 1.0) * 1.0;
    
    vec3 liteColor = ( vec3(0.23,0.81,0.98)*lite1 +
                       vec3(0.65,0.23,0.84)*lite2);
    
    //vec3 col = simpleReinhardToneMapping(c.xyz * liteColor);
    
    
    //fragColor = vec4( abs(world_nrm.xyz), 1.0 );
    //fragColor = vec4(col, 1.0);
    //fragColor = vec4(lite, lite, lite, 1.0 );
    fragColor = c;
}
@end

@program WorldShader displayVS displayFS

//------------------------------------------------------------------------------

@vs lakeVS
uniform vsParams {
    mat4 mvp;
    vec4 tintColor;
};

in vec4 position;
in vec4 normal;
in vec2 texcoord0;
out vec2 uv;
out vec4 nrm;
out vec4 color;
out vec4 world_nrm;

void main() {
    gl_Position = mvp * position;
    uv = texcoord0;
    world_nrm = normal;
    nrm = normalize(mvp * normal);
    color = tintColor;
}
@end

@fs lakeFS
uniform fsParams {
    float highlight;
    float tval;
};

uniform sampler2D tex;
in vec2 uv;
in vec4 nrm;
in vec4 world_nrm;
in vec4 color;
out vec4 fragColor;

float gamma = 2.2;

vec3 simpleReinhardToneMapping(vec3 color)
{
    float exposure = 2.5;
    color *= exposure/(1. + color / exposure);
    color = pow(color, vec3(1. / gamma));
    return color;
}

void main() {
    vec4 tex1 = texture(tex, uv + vec2( 0.0f, tval )) +
               texture(tex, uv + vec2( 0.5f, tval*1.3 ));
    vec4 tex2 = texture(tex, uv + vec2( sin( tval ), sin( (uv.y * 2) + tval ) ) );
    vec3 c = (tex1 * color).rgb;
    
    //vec3 col = simpleReinhardToneMapping(c.xyz * liteColor);
    vec3 col = c * mix( vec3( 0.6,0.6,0.6), vec3( 1,1,1 ), highlight );
    col = clamp( pow( col, vec3(4.0) ), vec3( 0.0f ), vec3(1.0f));
    //float aval =clamp( pow( (tex2.r - 0.5), 1.0f ), 0.0f, 1.0f);
    float ripple = tex2.r * tex1.r;
    //fragColor = vec4( aval, aval,aval,1.0 );
    fragColor = vec4( col, 0.5 ) + vec4( ripple );
}
@end

@program LakeShader lakeVS lakeFS
