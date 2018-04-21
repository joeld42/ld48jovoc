//------------------------------------------------------------------------------
//  Vertex and fragment shaders for rendering into the frame buffer.
//
@vs displayVS
uniform vsParams {
    mat4 mvp;
};

in vec4 position;
in vec4 normal;
in vec2 texcoord0;
out vec2 uv;
out vec4 nrm;
out vec4 world_nrm;

void main() {
    gl_Position = mvp * position;
    uv = texcoord0;
    world_nrm = normal;
    nrm = normalize(mvp * normal);
}
@end

@fs displayFS
uniform sampler2D tex;
in vec2 uv;
in vec4 nrm;
in vec4 world_nrm;
in vec4 ldir;
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
    vec4 c = texture(tex, uv);
    //vec4 c = vec4(1.0) - vec4( uv.x, uv.y, 0.0, 1.0 );
    vec3 ldir = vec3( -1.0, 0.0, 0.3 );
    vec3 ldir2 = vec3( 1.0, 0.5, 0.4 );
    float lite1 = clamp(pow( dot(nrm.xyz, normalize(ldir.xyz)), 5.0 ), 0.0, 1.0) * 5.0;
    float lite2 = clamp(dot(nrm.xyz, normalize(ldir2.xyz)), 0.0, 1.0) * 1.0;
    
    vec3 liteColor = ( vec3(0.23,0.81,0.98)*lite1 +
                       vec3(0.65,0.23,0.84)*lite2);
    
    vec3 col = simpleReinhardToneMapping(c.xyz * liteColor);
    
    
    //fragColor = vec4( abs(world_nrm.xyz), 1.0 );
    fragColor = vec4(col, 1.0);
    //fragColor = vec4(lite, lite, lite, 1.0 );
}
@end

@program WorldShader displayVS displayFS
