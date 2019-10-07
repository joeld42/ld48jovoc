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

vec3 whitePreservingLumaBasedReinhardToneMapping(vec3 color)
{
	float white = 2.;
	float luma = dot(color, vec3(0.2126, 0.7152, 0.0722));
	float toneMappedLuma = luma * (1. + luma / (white*white)) / (1. + luma);
	color *= toneMappedLuma / luma;
	color = pow(color, vec3(1. / 2.2));
	return color;
}

void main() {
    vec4 color = texture(tex, uv);
	vec3 c2 = whitePreservingLumaBasedReinhardToneMapping( color.xyz );
    
	fragColor = vec4( pow( c2+vec3(0.1, 0.1,0.2), vec3(2.5f)), 1.0 );
}
@end

@program PostProcShader postprocVS postprocFS

