@block util
vec4 gamma(vec4 c) {
    float p = 1.0/2.2;
    return vec4(pow(c.xyz, vec3(p, p, p)), c.w);
}
@end

@vs postprocVS
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

@fs postprocFS
@include util
#define FLT_MAX 3.402823466e+38
#define M_PI 3.1415926535897932384626433832795
uniform sampler2D tex;
in vec2 uv;
out vec4 fragColor;
void main() {
    vec4 color = texture(tex, uv);
    fragColor = color;
}
@end

@program PostProcShader postprocVS postprocFS

