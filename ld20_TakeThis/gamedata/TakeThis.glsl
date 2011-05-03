//
//  TakeThis.glsl
//  ld48jovoc
//
//  Created by Joel Davis on 5/2/11.
//  Copyright 2011 Joel Davis. All rights reserved.
//
---- Vertex ----
void main() 
{
    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
}

---- Fragment ----
void main()
{
    gl_FragColor = vec4(1.0, 0.0, 1.0, 1.0);
}