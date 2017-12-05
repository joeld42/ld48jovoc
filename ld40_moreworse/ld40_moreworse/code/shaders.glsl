//------------------------------------------------------------------------------
//  Basic Shader for Basic Stuff
//------------------------------------------------------------------------------
@vs displayVS
uniform vsParams {
    mat4 mvp;
    vec4 tintColor;
};

in vec4 position;
in vec4 normal;
in vec2 texcoord0;
in vec2 texcoord1;
out vec2 uv;
out vec2 uvDecal;
out vec4 nrm;
out vec4 color;
out vec4 world_nrm;

void main() {
    gl_Position = mvp * position;
    uv = texcoord0;
    uvDecal = texcoord1;
    world_nrm = normal;
    nrm = normalize(mvp * normal);
    
    color = tintColor;
}
@end

@fs displayFS
uniform sampler2D tex;
in vec2 uv;
in vec2 uvDecal;
in vec4 nrm;
in vec4 world_nrm;
in vec4 ldir;
in vec4 color;
out vec4 fragColor;
void main() {
    vec4 c = texture(tex, uv );
    //vec4 c = vec4(1.0) - vec4( uv.x, uv.y, 0.0, 1.0 );
    vec3 ldir = vec3( -1.0, 0.0, 0.3 );
    vec3 ldir2 = vec3( 1.0, 0.5, 0.4 );
    float lite1 = clamp(dot(nrm.xyz, normalize(ldir.xyz)), 0.0, 1.0) * 2.0;
    float lite2 = clamp(dot(nrm.xyz, normalize(ldir2.xyz)), 0.0, 1.0) * 2.0;
    
    vec3 liteColor = ( vec3(0.23,0.81,0.98)*lite1 +
                       vec3(0.65,0.23,0.84)*lite2);
    
    //fragColor = vec4( abs(world_nrm.xyz), 1.0 );
    //fragColor = vec4(c.xyz * liteColor, 1.0);
    fragColor = vec4(c.xyz, 1.0) * color;
    fragColor = vec4( 1.0,0.0,1.0, 1.0) * color;
    //fragColor = vec4(lite, lite, lite, 1.0 );
}
@end

@program TestShader displayVS displayFS


//------------------------------------------------------------------------------
//  Crate Shader for crates with decals and stuff
//------------------------------------------------------------------------------
@vs crateVS
uniform vsParams {
    mat4 mvp;
    vec4 decalTint;
    vec4 tintColor;
};

in vec4 position;
in vec4 normal;
in vec2 texcoord0;
in vec2 texcoord1;
out vec2 uv;
out vec2 uvDecal;
out vec4 nrm;
out vec4 color;
out vec4 decalTintColor;
out vec4 world_nrm;

void main() {
    gl_Position = mvp * position;
    uv = texcoord0;
    uvDecal = texcoord1;
    world_nrm = normal;
    nrm = normalize(mvp * vec4(normal.xyz,0) );
    
    color = tintColor;
    decalTintColor = decalTint;
}
@end

@fs crateFS
uniform fsParams {
    vec2 decalUVOffs;
};
uniform sampler2D tex;
uniform sampler2D texDecal;
in vec2 uv;
in vec2 uvDecal;
in vec4 nrm;
in vec4 world_nrm;
in vec4 ldir;
in vec4 color;
in vec4 decalTintColor;
out vec4 fragColor;
void main() {
    vec4 c = texture(tex, uv );
    vec2 uvDecal2 = (uvDecal * 0.25) + decalUVOffs;
    vec4 cDecal = texture(texDecal, uvDecal2 );
    

    vec3 cBaseColor = c.xyz * color.xyz;
    vec3 cDecalColor = cDecal.xyz * decalTintColor.xyz;    
    vec3 cResult = mix( cBaseColor, cDecalColor, cDecal.a * decalTintColor.a );

    vec3 ldir = vec3( -1.0, 0.3, 0.0 );
    float lite1 = clamp(dot(nrm.xyz, normalize(ldir.xyz)) , 0.6, 1.0) * 1.5;

    fragColor = vec4( cResult * lite1, 1.0);
}
@end

@program CrateShader crateVS crateFS

