#ifndef QUADBUFF_H
#define QUADBUFF_H

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <crtdbg.h>
#endif

#include "debug.h"

// QuadBuff is a growable buffer of quads, usually screen-aligned
// and good for using for stuff like text rendering or sprites.
template <typename VertexT>
class QuadBuff
{
    QuadBuff( size_t initial_size=0, bool grow=true );
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

    // Gets a VBO for this data. Note that you can't 
    // grow the buffer any more after you call this.
    GLuint vbo();

private:
    VertexT *m_bufferData;
    bool m_growable;

    size_t m_buffSize; // number of verts in use
    size_t m_buffCapacity; // number of quads possible

    // Support for GL vertex buffers
    bool m_vboInitted;    
    GLuint m_vbo;    
    
};

// =========================================
// Implementation
// =========================================
template <typename VertexT>
QuadBuff<VertexT>::QuadBuff( size_t initial_size=0, bool grow=true ) : 
                 m_bufferData( NULL ),
                 m_growable( false),
                 m_buffSize( 0 ),
                 m_buffCapacity( 0 ),
                 m_vboInitted( false ),
                 m_vbo( 0 )
{
}

template <typename VertType>
QuatBuff<VertexT>::~QuadBuff()
{
    if (m_vboInitted)
    {
        glDeleteBuffers( 1, &m_vbo );        
    }

    free( m_bufferData );
}

template <typename VertType>
VertexT *QuadBuff<VertexT>::addQuad()
{
    VertexT *newQuad;

	// Do we need to make more space for this quad??
	// NOTE: potentially confusing: size/capacity is in # of verts,
	// not num quads
	if (m_buffSize + 4 >= m_buffCapacity)
	{

        // Can't grow if we've used this as a VBO ... todo add a
        // lock/release or something
        Assert( m_vboInitted == false );        

		// increase capacity
		int m_buffTargetCapacity;
		if (m_buffCapacity==0)
		{
			// start with 8192 because it's a nice number
			m_buffTargetCapacity = 8192;
		}
		else
		{
            // Double our size
			m_buffTargetCapacity = m_buffCapacity * 2;
		}

		//DBG::info("Grow quadbuff to new capacity %d (size %d)\n", m_buffTargetCapacity,
        //          sizeof(MapVert) * m_buffTargetCapacity );

		MapVert *newBuff = (MapVert*)calloc( m_buffTargetCapacity, 
											  sizeof( MapVert ) );

		// copy over existing if needed
		if (m_bufferData)
		{
            // FIXME: use a common set of memory macros
			memcpy( newBuff, m_bufferData, sizeof(VertexT) * m_buffSize );
			free( m_bufferData );
		}
		m_bufferData = newBuff;
		m_buffCapacity = m_buffTargetCapacity;
	}

	// Ok, now we know there's space for the new quad
	newQuad = &m_islandDrawBuf[m_buffSize];
	m_buffSize += 4;

	return newQuad;
}

template <typename VertType>
GLuint QuadBuff<VertexT>::vbo()
{
    // Get a vbo if we haven't yet
    if (!m_vboInitted)
    {
        glGenBuffers( 1, &m_bufferData );
        glBindBuffer( GL_ARRAY_BUFFER, m_vertexData );
        size_t uiSize = sizeof( VertexT ) * m_quadSize;

        // FIXME: Sometimes we don't want static draw...
        glBufferData( GL_ARRAY_BUFFER, uiSize, m_vertexData, GL_STATIC_DRAW );        

        m_vboInitted = true;
    }

    return m_vbo;
}



#endif


