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

#define HEX_SIZE 10.0

uniform fsParams {
    float aspect;
	mat4 cameraXform;
};
in vec2 uv;
out vec4 fragColor;

float HexDist(vec2 p) {
	p = abs(p);
    
    float c = dot(p, normalize(vec2(1,1.73)));
    c = max(c, p.x);
    
    return c;
}

vec4 HexCoords(vec2 uv) {
	vec2 r = vec2(1, 1.73);
    vec2 h = r*.5;
    
    vec2 a = mod(uv, r)-h;
    vec2 b = mod(uv-h, r)-h;
    
    vec2 gv = dot(a, a) < dot(b,b) ? a : b;
    
    float x = atan(gv.x, gv.y);
    float y = HexDist(gv);
    vec2 id = uv-gv;
    return vec4(x, y, id.x,id.y);
}

// Repeat in two dimensions
vec2 pMod2(inout vec2 p, vec2 size) {
	vec2 c = floor((p + size*0.5)/size);
	p = mod(p + size*0.5,size) - size*0.5;
	return c;
}

float HexPrism(vec3 p, vec2 h)
{
   vec3 q = abs(p);
   return max(q.z-h.y,max(q.x+q.y*0.57735,q.y*1.1547)-h.x);
}

vec3 ReplicateXY(vec3 p, vec3 c)
{
   return vec3(mod(p.x, c.x) - 0.5 * c.x, mod(p.y, c.y) - 0.5 * c.y, p.z);
}

// http://iquilezles.org/www/articles/smin/smin.htm
float smin( float a, float b, float k )
{
    float h = max(k-abs(a-b),0.0);
    return min(a, b) - h*h*0.25/k;
}

// http://iquilezles.org/www/articles/smin/smin.htm
vec2 smin( vec2 a, vec2 b, float k )
{
    float h = clamp( 0.5+0.5*(b.x-a.x)/k, 0.0, 1.0 );
    return mix( b, a, h ) - k*h*(1.0-h);
}

// http://iquilezles.org/www/articles/smin/smin.htm
float smax( float a, float b, float k )
{
    float h = max(k-abs(a-b),0.0);
    return max(a, b) + h*h*0.25/k;
}


float hash12(vec2 p)
{
	vec3 p3  = fract(vec3(p.xyx) * .1031);
    p3 += dot(p3, p3.yzx + 33.33);
    return fract((p3.x + p3.y) * p3.z);
}

vec4 calcHex( in vec3 pos, in vec3 hexp, in vec2 hexId )
{
	float hh = 5.0 + sin(hash12(hexId+vec2(300.0)) ) * 0.2;

	float hd = HexPrism( hexp, vec2( 1.1, hh) ) * 0.8;
	
	// edge bevel
	vec2 hxp = abs(hexp.xy);
	float c = dot( hxp.yx, normalize(vec2(1,1.73)));
    c = max(c, hxp.y);
	float cbv = smoothstep( 0.8, 1.0, c );

	return vec4( hd + cbv*0.1, 1.0, 0.0, 1.0 );
}

vec4 map( in vec3 pos0, float atime )
{
	vec3 pos = pos0 * 0.5;

	vec3 op1 = pos;
	vec3 op2 = (pos + vec3(3.5/2.0,1., 0));
	vec3 p1 = op1;
	vec3 p2 = op2;
	pMod2( p1.xy, vec2(3.5, 2) );
	pMod2( p2.xy, vec2(3.5, 2) );

   

	// Hex blobs
	vec4 hres1 = calcHex( pos, p1, (p1 - op1).xy );
	vec4 hres2 = calcHex( pos, p2, (p2 - op2).xy );

	vec4 res = (hres1.x < hres2.x)?hres1:hres2;

	// bowl
    //float d1 = pos.z - (3.0 + clamp( length(pos.xy)*0.25, 0.0, 10.0) );
	//res = vec4( max(res.x, -d1), 1.0, 0.0, 1.0 );

	//blobify
	//float b = 1.0*(sin(pos.x)+sin(pos.y)+pos.z) * (abs(pos.z) - 4.0); 
	vec2 bp = vec2(pos.xy)*0.3 + pos.zz*0.2;
	float b = (sin(bp.x)+sin(bp.y)) ;

	bp *= mat2(0.8,0.6,-0.6,0.8)*2.1;
	b += 0.5*(sin(bp.x)+sin(bp.y));

	b = b + (abs(pos.z) - 6.0); 

	res = vec4( smax(res.x, -b, 0.4), 1.0, 0.0, 1.0 );
	//res = vec4(b, 1.0, 0.0, 1.0 );

    // x: SDF dist, y: mtlId, occl?, unused? 
    return res;    
}

// http://iquilezles.org/www/articles/normalsSDF/normalsSDF.htm
vec3 calcNormal( in vec3 pos, float time )
{
    
#if 0
    vec2 e = vec2(1.0,-1.0)*0.5773*0.001;
    return normalize( e.xyy*map( pos + e.xyy, time ).x + 
                      e.yyx*map( pos + e.yyx, time ).x + 
                      e.yxy*map( pos + e.yxy, time ).x + 
                      e.xxx*map( pos + e.xxx, time ).x );
#else
    // inspired by klems - a way to prevent the compiler from inlining map() 4 times
    vec3 n = vec3(0.0);
	// iq defined ZERO here as min(0,iFrame) but idk why
    for( int i=0; i<4; i++ )
    {
        vec3 e = 0.5773*(2.0*vec3((((i+3)>>1)&1),((i>>1)&1),(i&1))-1.0);
        n += e*map(pos+0.001*e,time).x;
    }
    return normalize(n);
#endif    
}

#define MAX_DIST 200.0
vec4 castRay( in vec3 ro, in vec3 rd, float time )
{
    vec4 res = vec4( -1.0, -1.0, 0.0, 1.0 );
    float tmin = 0.5;
    float tmax = MAX_DIST;

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


void main() {	

    // use Z up because of bad reasons


    // ray direction and origin
	vec2 ss = (uv * 2.0) - 1.0;
    vec3 rd = (cameraXform * vec4(normalize( vec3( ss.x * aspect, ss.y, -2) ), 0.0)).xyz;    
    vec3 ro = (cameraXform * vec4( 0,0,0,1 )).xyz;    


    // sky color (borrowed from iq happy jumping shader)
    vec3 col = vec3(0.5, 0.8, 0.9) - max(rd.y,0.0)*0.5;

    // sky clouds
	{
		vec2 uv = 1.5*rd.xy/rd.z;
		float cl  = 1.0*(sin(uv.x)+sin(uv.y)); uv *= mat2(0.8,0.6,-0.6,0.8)*2.1;
			  cl += 0.5*(sin(uv.x)+sin(uv.y));
		col += 0.1*(-1.0+2.0*smoothstep(-0.1,0.1,cl-0.4));

		// sky horizon
		col = mix( col, vec3(0.5, 0.7, .9), exp(-10.0*max(rd.z,0.0)) );   
	}

	// floor
	vec3 floorCol;
	{
		vec2 uv = -1.5*rd.xy/rd.z;
		vec2 floorStripe = smoothstep( vec2(0.95 - (1.0-abs(rd.z))*0.95 ), vec2(0.95), cos( uv ) );
		floorCol = mix( vec3( 0.2, 0.6, 0.3 ), vec3( 0.8, 0.95, 0.9), max( floorStripe.x, floorStripe.y));

		col = mix( floorCol, col, exp(-5.0*max( -rd.z,0.0)) );   
	}

	#if 1
    float time = 0.0;
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

            col = (nrm + vec3(1.0)) * 0.5;
			
			//vec4 hc = HexCoords( pos.xy / HEX_SIZE );
			//vec3 hexFill = vec3( 0.850, 0.670, 0.603 );
			//vec3 hexStroke = vec3( 0.9, 0.9, 0.95 );
			//col = mix( hexFill, hexStroke, smoothstep( 0.48, 0.49, hc.y ) );
			//col = hc.xyz;
       }

	   // Fade in the floor color at MAX_DIST
		col = mix (col, floorCol, smoothstep(MAX_DIST-50, MAX_DIST, res.x));
    }
	#endif

	
        
	fragColor = vec4( col, 1 );    
}

@end

@program BackgroundShader backgroundVS backgroundFS

