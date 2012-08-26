#include "pally.h"
#include "useful.h"

#define min3(a, b, c) ((a)<(b) ? ((a)<(c) ? (a) : (c)) : ((b)<(c) ? (b) : (c)))
#define max3(a, b, c) ((a)>(b) ? ((a)>(c) ? (a) : (c)) : ((b)>(c) ? (b) : (c)))

#define lerp(a, b, t) ((t)*(a) + (1.0-t)*(b))

//vec3f m_colorSky;

	//vec3f m_colorOrganic1;
	//vec3f m_colorOrganic2;

	//vec3f m_colorMineral1;
	//vec3f m_colorMineral2;

//vec3f m_colorAccent;

// from the internet... Author: Bert Bos <bert@w3.org>
vec3f rgb2hsv( const vec3f &rgb )
{
  float max, min, del;
  vec3f ret;

  max = max3(rgb.x, rgb.y, rgb.z);
  min = min3(rgb.x, rgb.y, rgb.z);

  del = max - min;
  ret.z = max;
  ret.y = max == 0.0 ? 0.0 : del / max;

  ret.x = -1;					/* No hue */
  if (ret.y != 0.0) {
    

    if (rgb.x == max) ret.x = ((rgb.y-rgb.z)/del) * 60.0f;
    else if (rgb.y == max) ret.x = ((rgb.z-rgb.x)/del) * 60.0f + 120.0f;
    else ret.x = ((rgb.x-rgb.y)/del) * 60.0f + 240.0f;
    
    if (ret.x < 0) ret.x += 360;
	if (ret.x > 360) ret.x -= 360;
  }  

  return ret;
}

// from the internet... Author: Bert Bos <bert@w3.org>
vec3f hsv2rgb( const vec3f &hsv )
{
	int j;  
	float f, p, q, t, h, s, v;
	vec3f rgb;
	h = hsv.x;
	s = hsv.y;
	v = hsv.z;

  /* convert HSV back to RGB */
  if (h < 0 || s == 0.0) {			/* Gray */
    rgb.x = v;
	rgb.y = v;
    rgb.z = v;
  } else {					/* Not gray */
    if (h == 360.0) h = 0.0;
    h = h / 60.0;
    j = h;					/* = floor(h) */
    f = h - j;
    p = v * (1 - s);
    q = v * (1 - (s * f));
    t = v * (1 - (s * (1 - f)));

    switch (j) {
      case 0:  rgb.x = v;  rgb.y = t;  rgb.z = p;  break; /* Between red and yellow */
      case 1:  rgb.x = q;  rgb.y = v;  rgb.z = p;  break; /* Between yellow and green */
      case 2:  rgb.x = p;  rgb.y = v;  rgb.z = t;  break; /* Between green and cyan */
      case 3:  rgb.x = p;  rgb.y = q;  rgb.z = v;  break; /* Between cyan and blue */
      case 4:  rgb.x = t;  rgb.y = p;  rgb.z = v;  break; /* Between blue and magenta */
      case 5:  rgb.x = v;  rgb.y = p;  rgb.z = q;  break; /* Between magenta and red */
      default: assert(! "Cannot happen"); rgb.x = rgb.y = rgb.z = 0;
    }
  } 

  return rgb;
}




// color scheme generators
enum 
{
	MONOCHROMATIC,
	ANALAGOUS,
	SPLIT_COMPLEMENT,
	TRIAD,

	NUM_GENERATORS
};

const char *genNames[] = 
{
	"Monochromatic", "Analagous", "Split Complement", "Triad"
};
Pally::Pally()
{
}

void Pally::generate( int seed )
{
	// these are mostly used as HSV 
	vec3f sky(0,0,0), organic(0,0,0), mineral(0,0,0), accent(0,0,0);


	int gen = (int)(randUniform()*NUM_GENERATORS);
	printf("Pally::generate, scheme %s\n", genNames[gen] );

	switch ( gen )
	{
		case MONOCHROMATIC:
			{
				// Monochromatic
				accent.x = randUniform(0.0f, 360.0f); // hue
				accent.y = randUniform(0.3f, 1.0f); // sat				

				mineral = accent;				
				organic = mineral;				
				sky = organic;
				if (randUniform(0.0, 1.0) > 0.5) {
					// decrease value
					accent.z = 1.0f; 
					organic.z = 0.3f; 
					mineral.z = 0.6f; 
					sky.z = 0.0f; 
				} else {
					// increase value
					accent.z = 0.3f; 
					organic.z = 0.5f; 
					mineral.z = 0.7f; 
					sky.z = 1.0f; 
				}

			}
			break;

		case ANALAGOUS:		
			{
				float amt = randUniform( 10.0f, 50.0f );				
				
				// Analagous
				accent.x = randUniform(0.0f, 360.0f); // hue
				accent.y = randUniform(0.2f, 1.0f); // sat
				accent.z = 1.0f; // value;
		
				organic = accent;
				organic.x += amt;
				if (organic.x > 360.0f) organic.x -= 360.0f;

				mineral = organic;
				mineral.x += amt;
				if (mineral.x > 360.0f) mineral.x -= 360.0f;

				sky = mineral;
				sky.x += amt;
				if (sky.x > 360.0f) sky.x -= 360.0f;

				// BUG??
				//accent = hsv2rgb( accent );
				//organic = hsv2rgb( organic );
				//mineral = hsv2rgb( mineral );

				// desaturate the sky
				sky.y = randUniform( 0.1, 0.4 );
				sky = hsv2rgb( sky );
			}
			break;

		case SPLIT_COMPLEMENT:		
			{
				// Split complementary
				float amt = randUniform( 10.0f, 50.0f );				
							
				accent.x = randUniform(0.0f, 360.0f); // hue
				accent.y = randUniform(0.3f, 1.0f); // sat
				accent.z = randUniform(0.5f, 1.0f );
		
				organic = accent;
				organic.x += 180.0f + amt;
				if (organic.x > 360.0f) organic.x -= 360.0f;
				if (organic.x < 0.0f) organic.x += 360.0f;

				mineral = accent;
				mineral.x += 180.0f - amt;
				if (mineral.x > 360.0f) mineral.x -= 360.0f;
				if (mineral.x < 0.0f) mineral.x += 360.0f;

				sky = mineral;
				sky.x += 180.0f;
				if (sky.x > 360.0f) sky.x -= 360.0f;
				sky.y = randUniform( 0.1, 0.4 ); // desaturate				
			}
			break;

		case TRIAD:
			{
				// Triad (+ dark accent );			
				sky.x = randUniform(0.0f, 360.0f); // hue
				sky.y = randUniform(0.1f, 0.6f); // keep sat low
				sky.z  = randUniform(0.5f, 1.0f );
		
				organic = sky;
				organic.y = randUniform(0.1f, 0.6f); // keep sat low
				organic.x += 120.0f;
				if (organic.x > 360.0f) organic.x -= 360.0f;				

				mineral = organic;
				mineral.y = randUniform(0.1f, 0.6f); // keep sat low
				mineral.x += 120.0f;
				if (mineral.x > 360.0f) mineral.x -= 360.0f;				

				accent.x = randUniform(0.0f, 360.0f); // hue
				accent.y = randUniform(0.3f, 1.0f); // sat
				accent.z  = randUniform(0.0f, 0.1f );				
			}
	}

	// convert to rgb
	accent = hsv2rgb( accent );
	organic = hsv2rgb( organic );
	mineral = hsv2rgb( mineral );				
	sky = hsv2rgb( sky );

	// set palette colors
	m_colorSky = sky;
	m_colorOrganic1 = organic;
	m_colorMineral1 = mineral;
	m_colorAccent = accent;

	if (randUniform() < 0.5 )
	{
		m_colorMineral2 = sky * 0.1;
		m_colorOrganic2 = vec3f( 1.0, 1.0, 1.0 );
	}
	else
	{
		m_colorMineral2 = vec3f( 1.0, 1.0, 1.0 );
		m_colorOrganic2 = sky * 0.1;
	}

	printf("organic1 %f %f %f\n", m_colorOrganic1.x, m_colorOrganic1.y, m_colorOrganic1.z );
	printf("organic2 %f %f %f\n", m_colorOrganic2.x, m_colorOrganic2.y, m_colorOrganic2.z );
}

	

