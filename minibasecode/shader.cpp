//
//  shader.cpp
//  ld48jovoc
//
//  Created by Joel Davis on 8/23/12.
//  Copyright (c) 2012 Joel Davis. All rights reserved.
//
#include <stdio.h>
#include <stdint.h>

#include <GLee.h>
#include <glsw.h>
#include <shader.h>
#include <shapes.h> // for DrawVert

int32_t _compileShader( const char *shaderText, uint32_t shaderType );
void _printShaderLog( int32_t program );
void _link( int32_t program );

GLint loadShader( const char *shaderKey )
{
    GLint program;
	printf("loadShader %s ...\n", shaderKey );
	
    // Create shader program
    program = glCreateProgram();
    
    // vertex and fragment shaders
    GLuint vertShader, fragShader;    
    
    // Get the vertex shader part
    char buff[1024];
    sprintf( buff, "%s.Vertex", shaderKey );
    const char *vertShaderText = glswGetShader( buff );
    if (!vertShaderText)
    {
        printf("Couldn't find shader key '%s' : %s\n", buff, glswGetError() );
		return SHADER_FAIL;
    }
    
    // Compile the vertex shader
    vertShader = _compileShader( vertShaderText, GL_VERTEX_SHADER );
    
    // Get fragment shader
    sprintf( buff, "%s.Fragment", shaderKey );
    const char *fragShaderText = glswGetShader( buff);
    if (!fragShaderText)
    {
        printf("Couldn't find shader key: %s\n", buff );
		return SHADER_FAIL;
    }
    
    printf( "compile fragment shader" );
    
    // Compile the fragment shader
    fragShader = _compileShader( fragShaderText, GL_FRAGMENT_SHADER );
    
    // Attach shaders
    glAttachShader( program, vertShader );
    glAttachShader( program, fragShader );
    
    printf("... bind attrs\n" );
	
	// Bind Attrs (todo put in subclass)
	// FIXME: some shaders dont have all these attrs..
	glBindAttribLocation( program, DrawVertAttrib_POSITION, "position" );
	glBindAttribLocation( program, DrawVertAttrib_TEXCOORD, "texcoord" );
	glBindAttribLocation( program, DrawVertAttrib_NORMAL,   "normal" );    
	glBindAttribLocation( program, DrawVertAttrib_COLOR,   "color" );
	
    
    //  Link Shader
    printf("... links shaders\n" );
    _link( program );
    
    // Release vert and frag shaders
    glDeleteShader( vertShader );
    glDeleteShader( fragShader );    
    
	printf(" ----- %s ------\n", shaderKey );
	int activeUniforms;
	glGetProgramiv( program, GL_ACTIVE_UNIFORMS, &activeUniforms );	
	printf(" Active Uniforms: %d\n", activeUniforms );
	
    // done
    return program;    
}

int32_t _compileShader( const char *shaderText, 
                       uint32_t shaderType )
{
    GLint status;    
    GLuint shader;
    shader = glCreateShader( shaderType );
    
    // compile the shader
    glShaderSource( shader, 1, &shaderText, NULL );
    glCompileShader( shader );
    
    // Check for errors
    GLint logLength;
    glGetShaderiv( shader, GL_INFO_LOG_LENGTH, &logLength );
    if (logLength > 0)
    {
        char *log = (char *)malloc(logLength);
        glGetShaderInfoLog( shader, logLength, &logLength, log );
        
        printf("Error compiling shader:\n%s\n", log );
        free(log);        
    }
    
    glGetShaderiv( shader,GL_COMPILE_STATUS, &status );
    if (status==0)
    {
        glDeleteShader( shader );
        
        // TODO: better handle errors
        printf("Compile status is bad\n" );
        
        return 0;        
    }
    
    return shader;    
}

void _printShaderLog( int32_t program )
{
	GLint logLength;
    glGetProgramiv( program, GL_INFO_LOG_LENGTH, &logLength );
    if (logLength > 0 )
    {
        char *log = (char*)malloc(logLength);
        glGetProgramInfoLog( program, logLength, &logLength, log );
        printf ("Link Log:\n%s\n", log );
        free(log);        
    }
	
}

void _link( int32_t program )
{
    GLint status;
    
    glLinkProgram( program );
	
	_printShaderLog( program );
	
    glGetProgramiv( program, GL_LINK_STATUS, &status);
    if (status==0)
    {
        printf("ERROR Linking shader\n");        
    }    
}
