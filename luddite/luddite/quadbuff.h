#ifndef QUADBUFF_H
#define QUADBUFF_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <crtdbg.h>
#endif


#import <OpenGLES/ES1/gl.h>
#import <OpenGLES/ES1/glext.h>

//#include "debug.h"

// QuadBuff is a growable buffer of quads, usually screen-aligned
// and good for using for stuff like text rendering or sprites.
//
// FIXME: the 'grow' stuff is a bit misleading and still needs work
template <typename VertexT>
class QuadBuff
{
public:
	// Usage is usage from glBufferData, should be GL_STATIC_DRAW or
	// GL_DYNAMIC_DRAW.
    QuadBuff( size_t initial_size=0, bool grow=true, GLuint usage=GL_DYNAMIC_DRAW );
    ~QuadBuff();
    

    // Simplist way to add a quad.. fill in the data
    // yourself. For example:
    //   MyVert *quad = buff->addQuad();
    //   quad[0].p = VEC3( 0.0, 0.0, 0.0 );
    //   quad[1].p = VEC3( 1.0, 0.0, 0.0 );
    //   quad[2].p = VEC3( 1.0, 1.0, 0.0 );
    //   quad[3].p = VEC3( 0.0, 1.0, 0.0 );    
    VertexT *addQuad();

    // returns the raw buffer
    VertexT *data();

    // size of the buffer
    size_t size();    
    size_t capacity();
	
	// Clear the data, without shrinking capacity
	void clear();

    // Gets a VBO for this data. Note that you can't 
    // grow the buffer any more after you call this.
    GLuint vbo();
	
protected:
	// For subclasses to call after the data has changed
	void updateBuffer();
	
//private:
    VertexT *m_bufferData;
    bool m_growable;
	GLuint m_usage;

    size_t m_buffSize; // number of verts in use
    size_t m_buffCapacity; // number of quads possible

    // Support for GL vertex buffers
    bool m_vboInitted;    
	size_t m_vboSize;
    GLuint m_vbo;    
    
};

// =========================================
// Implementation
// =========================================
template <typename VertexT>
QuadBuff<VertexT>::QuadBuff( size_t initial_size, bool grow, GLuint usage ) : 
                 m_bufferData( NULL ),
                 m_growable( false),
				 m_usage( usage ),
                 m_buffSize( 0 ),
                 m_buffCapacity( 0 ),
                 m_vboInitted( false ),
                 m_vbo( 0 ),
				 m_vboSize( 0 )
{
	// initialize if requested
	if (initial_size)
	{
		int buffTargetCapacity = initial_size * 6;
		m_bufferData = (VertexT*)calloc( buffTargetCapacity, 
								   sizeof( VertexT ) );
		m_buffCapacity = buffTargetCapacity;
	}
}

template <typename VertexT>
QuadBuff<VertexT>::~QuadBuff()
{
    if (m_vboInitted)
    {
        glDeleteBuffers( 1, &m_vbo );        
    }

    free( m_bufferData );
}


template <typename VertexT>
size_t QuadBuff<VertexT>::size()
{
	return m_buffSize;
}

template <typename VertexT>
size_t QuadBuff<VertexT>::capacity()
{
	return m_buffCapacity;
}

template <typename VertexT>
void QuadBuff<VertexT>::clear()
{
	m_buffSize = 0;
}


template <typename VertexT>
VertexT *QuadBuff<VertexT>::addQuad()
{
    VertexT *newQuad;

	// Do we need to make more space for this quad??
	// NOTE: potentially confusing: size/capacity is in # of verts,
	// not num quads
	if (m_buffSize + 6 >= m_buffCapacity)
	{

        // Can't grow if we've used this as a VBO ... todo add a
        // lock/release or something
        //Assert( m_vboInitted == false );        

		// increase capacity
		int buffTargetCapacity;
		if (m_buffCapacity==0)
		{
			// start with 8192 because it's a nice number
			buffTargetCapacity = 8192;
		}
		else
		{
            // Double our size
			buffTargetCapacity = m_buffCapacity * 2;
		}

		//DBG::info("Grow quadbuff to new capacity %d (size %d)\n", m_buffTargetCapacity,
        //          sizeof(MapVert) * m_buffTargetCapacity );

		VertexT *newBuff = (VertexT*)calloc( buffTargetCapacity, 
											  sizeof( VertexT ) );

		// copy over existing if needed
		if (m_bufferData)
		{
            // FIXME: use a common set of memory macros
			memcpy( newBuff, m_bufferData, sizeof(VertexT) * m_buffSize );
			free( m_bufferData );
		}
		m_bufferData = newBuff;
		m_buffCapacity = buffTargetCapacity;
	}

	// Ok, now we know there's space for the new quad
	newQuad = &m_bufferData[m_buffSize];
	m_buffSize += 6;

	return newQuad;
}

template <typename VertexT>
GLuint QuadBuff<VertexT>::vbo()
{
    // Get a vbo if we haven't yet
    if (!m_vboInitted)
    {
        glGenBuffers( 1, &m_vbo );
        glBindBuffer( GL_ARRAY_BUFFER, m_vbo );
        size_t uiSize = sizeof( VertexT ) * m_buffCapacity;

		// Create the buffer
        glBufferData( GL_ARRAY_BUFFER, uiSize, m_bufferData, m_usage );

		m_vboSize = uiSize;
        m_vboInitted = true;
    }

    return m_vbo;
}


template <typename VertexT>
void QuadBuff<VertexT>::updateBuffer()
{
	// Update our data
	size_t uiSize = sizeof( VertexT ) * m_buffSize;
	
	// Did the buffer grow?
	if (uiSize < m_vboSize )
	{
		// Nope, just SubData part of it
		glBufferSubData( GL_ARRAY_BUFFER, 0, uiSize, m_bufferData );
	}
	else
	{
		// yes, need to realloc
		glBufferData( GL_ARRAY_BUFFER, uiSize, m_bufferData, m_usage );
		m_vboSize = uiSize;
	}
}
#endif


