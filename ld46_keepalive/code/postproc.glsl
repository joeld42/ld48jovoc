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

uniform fsParams {
    float flameAmt;
};

uniform sampler2D tex;
uniform sampler2D depthTex;
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

// cosine based palette, 4 vec3 params
// from iq the master
vec3 palette( in float t, in vec3 a, in vec3 b, in vec3 c, in vec3 d )
{
    return a + b*cos( 6.28318*(c*t+d) );
}

void main() {

	vec2 uvdir = uv - vec2( 0.5, 0.5 );
	float abbr = 0.015;
	//vec2 uvoffs = pow( uvdir, vec2(2.0, 2.0) ) * abbr;
	vec2 uvoffs = uvdir * abbr;
	
    vec4 colorR = texture(tex, uv - uvoffs*0.0 );
	vec4 colorG = texture(tex, uv - uvoffs*0.5 );
	vec4 colorB = texture(tex, uv - uvoffs*1.0 );
	vec3 cpixel;

	float d = length( vec2( uvdir.x * (1280.0/720.0), uvdir.y + 0.15 ) );
	float flameRad = 0.05 + (flameAmt * 0.9);
	if (d < flameRad) {
	//if (true) {
		cpixel = vec3( colorR.x, colorG.y, colorB.z);
	
	} else {
		
		float pz = texture( depthTex, uv ).z - 0.001;
		float occ = 0.0;
		float sz = 1;
		for (int x=-1; x <= 3; x++) {
			for (int y=-1; y <= 3; y++) {
			float samz = texture( depthTex, vec2( uv.x + (x * sz/1280.0), uv.y + (y * sz/720.0)) ).z;
					
					float range = smoothstep( 0.0, 1.0, 5.0 / abs( samz - pz));
					occ += (samz > pz) ? 0.0 : range;
			}
		}

		float halftone = occ / 8.0;
		vec3 cpixel1 = mix( vec3( 0, 0.1, 0.12 ), vec3 ( 0.8, 0.6, 0.5 ), halftone );
		vec3 cpixel2 = mix( vec3( 0, 0, 0.12 ), vec3 ( 0.4, 0.4, 0.45 ), halftone );
		cpixel = mix( cpixel1, cpixel2, clamp( pz / 100.0f, 0.0, 1.0) );
		//cpixel = vec3( occ / 8.0 );
		
	}
	vec3 c2 = whitePreservingLumaBasedReinhardToneMapping( cpixel );
    
    //if (uv.y < 0.33)
    //{
    //    fragColor = color; // top: no tonemapping
    //} else if (uv.y > 0.66)
    //{
    //    fragColor = vec4( c2, 1.0f ); // bottom, tonemapping, no CC
    //}
    //else {
        //middle: tonemap and cc
        fragColor = vec4( pow( c2+vec3(0.1, 0.1,0.12), vec3(2.2f)), 1.0 );
    //}
	//fragColor = vec4( uvdir.x, uvdir.y, uv.x, 1.0 );

    
}
@end

@program PostProcShader postprocVS postprocFS

