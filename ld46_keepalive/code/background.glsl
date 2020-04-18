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

vec4 map( in vec3 pos, float atime )
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


void main() {	

    // use Z up because of bad reasons


    // ray direction and origin
	vec2 ss = (uv * 2.0) - 1.0;
    vec3 rd = (cameraXform * vec4(normalize( vec3( ss.x * aspect, ss.y, -1.8) ), 0.0)).xyz;    
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
	{
		vec2 uv = -1.5*rd.xy/rd.z;
		vec2 floorStripe =  step( 0.95, fract(uv / 2.0f) );
		vec3 floorCol = mix( vec3( 0.2, 0.6, 0.3 ), vec3( 0.8, 0.95, 0.9), max( floorStripe.x, floorStripe.y));

		col = mix( floorCol, col, exp(-10.0*max( -rd.z,0.0)) );   
	}

	#if 1
    float time = 0.0;
    vec4 res = castRay( ro, rd, time );
    if ( res.y > -0.5) {
        float t = res.x;
        vec3 pos = ro + t*rd;
        float focc = res.w;

        //vec3 nrm = calcNormal( pos, time );
        //vec3 ref = reflect( rd, nrm );
        
        // material
        //if (res.y > 0.0 )
        {
            // ground mtl
            col = vec3( res.w );
       }
    }
	#endif
        
	fragColor = vec4( col, 1 );    
}

@end

@program BackgroundShader backgroundVS backgroundFS

