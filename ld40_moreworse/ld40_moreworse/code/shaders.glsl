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
void main() {
    //vec4 c = texture(tex, uv * vec2(20.0, 10.0));
    vec4 c = vec4(1.0) - vec4( uv.x, uv.y, 0.0, 1.0 );
    vec3 ldir = vec3( -1.0, 0.0, 0.3 );
    vec3 ldir2 = vec3( 1.0, 0.5, 0.4 );
    float lite1 = clamp(dot(nrm.xyz, normalize(ldir.xyz)), 0.0, 1.0) * 2.0;
    float lite2 = clamp(dot(nrm.xyz, normalize(ldir2.xyz)), 0.0, 1.0) * 2.0;
    
    vec3 liteColor = ( vec3(0.23,0.81,0.98)*lite1 +
                       vec3(0.65,0.23,0.84)*lite2);
    
    //fragColor = vec4( abs(world_nrm.xyz), 1.0 );
    fragColor = vec4(c.xyz * liteColor, 1.0);
    //fragColor = vec4(lite, lite, lite, 1.0 );
}
@end

@program TestShader displayVS displayFS
