#include <allegro.h>
#include <assert.h>

#include "shader.h"
#include "globals.h"

Color::Color()
{
	r = 1.0f;
	g = 0.0f;
	b = 1.0f;
}
Color::Color(float _r, float _g, float _b )
{
	r = _r; g = _g; b = _b;
}

// from the internet... Author: Bert Bos <bert@w3.org>
Color rgb2hsv( const Color &rgb )
{
  float max, min, del;
  Color ret;

  max = max3(rgb.r, rgb.g, rgb.b);
  min = min3(rgb.r, rgb.g, rgb.b);

  del = max - min;
  ret.b = max;
  ret.g = max == 0.0 ? 0.0 : del / max;

  ret.r = -1;					/* No hue */
  if (ret.g != 0.0) {
    

    if (rgb.r == max) ret.r = ((rgb.g-rgb.b)/del) * 60.0f;
    else if (rgb.g == max) ret.r = ((rgb.b-rgb.r)/del) * 60.0f + 120.0f;
    else ret.r = ((rgb.r-rgb.g)/del) * 60.0f + 240.0f;
    
    if (ret.r < 0) ret.r += 360;
	if (ret.r > 360) ret.r -= 360;
  }  

  return ret;
}

// from the internet... Author: Bert Bos <bert@w3.org>
Color hsv2rgb( const Color &hsv )
{
	int j;  
	float f, p, q, t, h, s, v;
	Color rgb;
	h = hsv.r;
	s = hsv.g;
	v = hsv.b;

  /* convert HSV back to RGB */
  if (h < 0 || s == 0.0) {			/* Gray */
    rgb.r = v;
	rgb.g = v;
    rgb.b = v;
  } else {					/* Not gray */
    if (h == 360.0) h = 0.0;
    h = h / 60.0;
    j = h;					/* = floor(h) */
    f = h - j;
    p = v * (1 - s);
    q = v * (1 - (s * f));
    t = v * (1 - (s * (1 - f)));

    switch (j) {
      case 0:  rgb.r = v;  rgb.g = t;  rgb.b = p;  break; /* Between red and yellow */
      case 1:  rgb.r = q;  rgb.g = v;  rgb.b = p;  break; /* Between yellow and green */
      case 2:  rgb.r = p;  rgb.g = v;  rgb.b = t;  break; /* Between green and cyan */
      case 3:  rgb.r = p;  rgb.g = q;  rgb.b = v;  break; /* Between cyan and blue */
      case 4:  rgb.r = t;  rgb.g = p;  rgb.b = v;  break; /* Between blue and magenta */
      case 5:  rgb.r = v;  rgb.g = p;  rgb.b = q;  break; /* Between magenta and red */
      default: assert(! "Cannot happen"); rgb.r = rgb.g = rgb.b = 0;
    }
  } 

  return rgb;
}
