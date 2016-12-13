#version 330

// Input vertex attributes (from vertex shader)
in vec2 fragTexCoord;
in vec4 fragColor;

// Input uniform values
uniform sampler2D texture0;
uniform vec4 colDiffuse;

// Extra textures for the 16-color effect
uniform sampler3D pally;
uniform sampler2D dither;
uniform float ditherStrength;

// Output fragment color
out vec4 finalColor;

// NOTE: Add here your custom variables

//const vec2 size = vec2(800, 450);   // render size
const vec2 size = vec2(320, 200);   // render size
const float samples = 7.0;          // pixels per axis; higher = bigger glow, worse performance
const float quality = 2.5; 	        // lower = smaller glow, better quality

void main()
{
//    vec4 sum = vec4(0);
//    vec2 sizeFactor = vec2(1)/size*quality;

    // Texel color fetching from texture sampler
    vec4 source = texture(texture0, fragTexCoord);
    
    vec4 ditherColor = texture( dither, fragTexCoord*vec2(80,50) );
    
    float ditherAmt = (ditherColor.r * ditherStrength) - (ditherStrength/2.0);
    vec3 lookupColor = source.rgb + vec3(ditherAmt, ditherAmt, ditherAmt);
    lookupColor = max( lookupColor, vec3(0.0, 0.0, 0.0) );
    lookupColor = min( lookupColor, vec3(0.9, 0.9, 0.9) );
    
//    float brite = source.r + source.g + source.b;
    vec4 pallycolor = texture( pally, lookupColor);
//    vec4 pallycolor = texture( pally, vec3( fragTexCoord.x, fragTexCoord.y, ditherStrength ));
//    if (brite <0.01) {
//        pallycolor = vec4(0.0, 0.0, 0.0, 1.0);
//    } else if (brite > 2.9) {
//           pallycolor = vec4(1.0, 1.0, 1.0, 1.0);
//    }
    //vec4 pallycolor = vec4(lookupColor, 1.0);
    //vec4 pallycolor = vec4(lookupColor, 1.0);
    
    
    
    //vec4 pallycolor = texture( pally, fragTexCoord);
    
    //pallycolor.rgb = vec3(1.0, 0.0, 1.0);
    //finalColor = vec4( fragTexCoord.x, pallycolor.r, fragTexCoord.y, 1.0 );
//    if (fragTexCoord.x > 0.25) {
        finalColor = pallycolor;
//    } else {
//        finalColor = vec4(lookupColor, 1.0);
//    }
    //finalColor = ditherColor;
    
    //finalColor = texture( pally, )
    
    //finalColor = texture( texture0, fragTexCoord) * colDiffuse;
    
//    const int range = 3;            // should be = (samples - 1)/2;
//
//    for (int x = -range; x <= range; x++)
//    {
//        for (int y = -range; y <= range; y++)
//        {
//            sum += texture(texture0, fragTexCoord + vec2(x, y)*sizeFactor);
//        }
//    }
//
//    // Calculate final fragment color
//    finalColor = ((sum/(samples*samples)) + source)*colDiffuse;
}
