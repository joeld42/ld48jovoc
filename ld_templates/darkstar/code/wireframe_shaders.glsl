@vs wireframeVS
uniform vsParams {
    mat4 viewProj;
};
in vec4 position;
in vec4 color0;
out vec4 color;
void main() {
    gl_Position = viewProj * position;
    gl_PointSize = 5.0;
    color = color0;
}
@end

@fs wireframeFS
in vec4 color;
out vec4 fragColor;

void main() {
    fragColor = color;    
}
@end

@program WireframeShader wireframeVS wireframeFS

// =========================================================================

@vs pointVS
uniform vsParams {
    mat4 viewProj;
};
in vec4 position;
in vec4 color0;
out vec4 color;
void main() {
    gl_Position = viewProj * position;
    gl_PointSize = color0.a;
    color = color0;
}
@end

@fs pointFS
in vec4 color;
out vec4 fragColor;

void main() {
    vec2 coord = gl_PointCoord - vec2(0.5);  //from [0,1] to [-0.5,0.5]
    float d = length(coord);
    
    if(length(coord) > 0.55)  {
            discard;
    }
    
    float a = 1.0 - smoothstep( 0.45, 0.55, d );
    fragColor = vec4( color.rgb * a, a  );
}
@end

@program PointShader pointVS pointFS

// =========================================================================

@vs glyphVS
uniform vsParams {
    mat4 viewProj;
};
in vec2 position;
in vec2 texcoord0;
in vec3 color0;
out vec2 texcoord;
out vec4 color;
void main() {
    gl_Position = viewProj * vec4(position, 0, 1);
    texcoord = texcoord0;
    color = vec4(color0, 1.0);
}
@end

@fs glyphFS
uniform sampler2D tex;

in vec4 color;
in vec2 texcoord;

out vec4 fragColor;

void main() {
    fragColor = vec4( color.xyz, texture(tex, texcoord).x );
}
@end

@program GlyphShader glyphVS glyphFS
