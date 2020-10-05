//------------------------------------------------------------------------------
//  Background quad shader
//
@vs bgquadVS
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

@fs bgquadFS
in vec2 uv;
out vec4 fragColor;
uniform sampler2D tex;
void main() {
    vec4 bgcolor = texture(tex, uv);
    fragColor = bgcolor;
}

@end

@program BGQuadShader bgquadVS bgquadFS



//------------------------------------------------------------------------------
//  Background Scene shader
//
@vs backgroundVS
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

@fs backgroundFS

uniform fsParams {
    float aspect;
	mat4 cameraXform;
	float aTime;
};
in vec2 uv;
out vec4 fragColor;

// Repeat in two dimensions
vec2 pMod2(inout vec2 p, vec2 size) {
	vec2 c = floor((p + size*0.5)/size);
	p = mod(p + size*0.5,size) - size*0.5;
	return c;
}

float hash(vec3 p)  // replace this by something better
{
    p  = fract( p*0.3183099+.1 );
    p *= 17.0;
    return fract( p.x*p.y*p.z*(p.x+p.y+p.z) );
}

float noise( in vec3 x )
{
    vec3 i = floor(x);
    vec3 f = fract(x);
    f = f*f*(3.0-2.0*f);
    
    return mix(mix(mix( hash(i+vec3(0,0,0)),
                        hash(i+vec3(1,0,0)),f.x),
                   mix( hash(i+vec3(0,1,0)),
                        hash(i+vec3(1,1,0)),f.x),f.y),
               mix(mix( hash(i+vec3(0,0,1)),
                        hash(i+vec3(1,0,1)),f.x),
                   mix( hash(i+vec3(0,1,1)),
                        hash(i+vec3(1,1,1)),f.x),f.y),f.z);
}

vec4 map( in vec3 pos, float atime )
{
    //pMod2( pos.xy, vec2(3.0) );

    // ground
    //float d1 = pos.z - 0.2;
    //float d2 = length(pos) - 1.0;

    //vec4 res = vec4(max (d1,d2), 1.0, 0.0, 1.0 );
    
    // exclusion sphere
    float dSph = 3.0 - length( pos );
    float dPatt = pow( noise( (pos + vec3( atime * 3.3, atime * 1.7, 0.0 )) * 0.8 ), 4.0 );
    vec4 res = vec4(max (dSph,dPatt), 1.0, 0.0, 1.0 );
    
    // x: SDF dist, y: mtlId, occl?, unused?
    return res;
}


vec4 mapOLD( in vec3 pos, float atime )
{
	pMod2( pos.xy, vec2(3.0) );

    // ground
    float d1 = pos.z - 0.2;
	float d2 = length(pos) - 1.0;

    vec4 res = vec4(max (d1,d2), 1.0, 0.0, 1.0 );
	
    // x: SDF dist, y: mtlId, occl?, unused? 
    return res;    
}

vec4 castRay( in vec3 ro, in vec3 rd, float time )
{
    vec4 res = vec4( -1.0, -1.0, 0.0, 1.0 );
    float tmin = 0.5;
    float tmax = 200.0;

    float t = tmin;
	int i;
    for (i=0; i < 256 && t < tmax; i++)
    {
        vec4 h = map( ro+rd*t, time );
        if ( abs(h.x) < 0.0005*t) {
            res = vec4( t, h.yzw );
            break;
        }
        t += h.x;
    }	
	float st = i;
	res.w = st / (st + 30.0);
    return res;
}

#define ZERO (min(time,0))
// http://iquilezles.org/www/articles/normalsSDF/normalsSDF.htm
vec3 calcNormal( in vec3 pos, float time )
{
    
#if 1
    vec2 e = vec2(1.0,-1.0)*0.5773*0.001;
    return normalize( e.xyy*map( pos + e.xyy, time ).x +
                      e.yyx*map( pos + e.yyx, time ).x +
                      e.yxy*map( pos + e.yxy, time ).x +
                      e.xxx*map( pos + e.xxx, time ).x );
#else
    // inspired by tdhooper and klems - a way to prevent the compiler from inlining map() 4 times
    vec3 n = vec3(0.0);
    for( int i=ZERO; i<4; i++ )
    {
        vec3 e = 0.5773*(2.0*vec3((((i+3)>>1)&1),((i>>1)&1),(i&1))-1.0);
        n += e*map(pos+0.001*e,time).x;
    }
    return normalize(n);
#endif
}

vec3 calcLight( vec3 pos, vec3 nrm, float fOcc )
{
    vec3 baseMtlColor = vec3( 0.5, 0.4, 0.4 );
    vec3 lightCol = vec3( 1, 1.3, 2.9 ) * 0.3;
    vec3 lightDir = normalize( vec3( 1, 0.5, 0.5 ) );
    //vec3 lightDir = normalize( vec3( 1, 0.5, 0.5 ) );
    
    float nDotL = abs( dot( nrm, lightDir ) );
    vec3 litCol = (baseMtlColor * fOcc) + lightCol * nDotL;
    
    return litCol * 1.6;
}

void main() {	

    // use Z up because of bad reasons


    // ray direction and origin
	vec2 ss = (uv * 2.0) - 1.0;
    vec3 rd = (cameraXform * vec4(normalize( vec3( ss.x * aspect, ss.y, -1.8) ), 0.0)).xyz;    
    vec3 ro = (cameraXform * vec4( 0,0,0,1 )).xyz;    
    

    // sky color (borrowed from iq happy jumping shader)
    //vec3 col = vec3(0.5, 0.8, 0.9) - max(rd.y,0.0)*0.5;
    vec3 col = vec3(2.9, 2.0, 1.5) - max(rd.y,0.0)*0.5;

    // sky clouds
	{
		vec2 uv = 1.5*rd.xy/rd.z;
		float cl  = 1.0*(sin(uv.x + aTime)+sin(uv.y)); uv *= mat2(0.8,0.6,-0.6,0.8)*2.1;
        cl += 0.5*(sin(uv.x+ aTime * 1.3 )+sin(uv.y));
		col += 0.1*(-1.0+2.0*smoothstep(-0.1,0.1,cl-0.4));

		// sky horizon
        col = mix( col, vec3(2.9, 2.0, 1.5)*0.2, exp(-10.0*max(rd.z,0.0)) );
	}

	// floor
	{
		vec2 uv = -1.5*rd.xy/rd.z;
		vec2 floorStripe =  step( 0.95, fract(uv / 2.0f) );
        vec3 floorCol = mix( vec3(2.9, 2.0, 1.5), vec3( 1.2 ), max( floorStripe.x, floorStripe.y));

		col = mix( floorCol, col, exp(-10.0*max( -rd.z,0.0)) );   
	}

	#if 1
    float time = aTime;
    vec4 res = castRay( ro, rd, time );
    if ( res.y > -0.5) {
        float t = res.x;
        vec3 pos = ro + t*rd;
        float focc = res.w;

        vec3 nrm = calcNormal( pos, time );
        //vec3 ref = reflect( rd, nrm );
        
        // material
        //if (res.y > 0.0 )
        {
            // ground mtl
            //col = vec3( res.w );
            col = calcLight( pos, nrm, res.w );
       }
    }
	#endif
        
	fragColor = vec4( col, 1 );
    //fragColor = vec4( col.r, col.g, time, 1 );
}

@end

@program BackgroundShader backgroundVS backgroundFS

