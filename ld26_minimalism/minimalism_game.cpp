//
//  minimalism_game.cpp
//  ld48jovoc
//
//  Created by Joel Davis on 8/22/12.
//  Copyright (c) 2012 Joel Davis. All rights reserved.
//

#include <prmath/prmath.hpp>

#include <font.h>
#include <gamedata.h>
#include <useful.h>
#include <shader.h>
#include <load_obj.h>

// offsetof() gives a warning about non-POD types with xcode, so use these old
// school macros. This is OK because while VertType is non-POD, it doesn't have
// a vtable so these still work.
#define offset_d(i,f)    (long(&(i)->f) - long(i))
#define offset_s(t,f)    offset_d((t*)1000, f)

#include "minimalism_game.h"

extern Uint32 g_fps;

matrix4x4f LookAt2(const vec3f& target, const vec3f& view, const vec3f& up)
{
    matrix4x4f m44;

    vec3f z = Normalize(target - view);
    vec3f x = Normalize(CrossProduct(z,up));
    vec3f y = CrossProduct(x, z);

    m44[0][0] = x.x;
    m44[1][0] = x.y;
    m44[2][0] = x.z;
    m44[3][0] = DotProduct(x,view);
    m44[0][1] = y.x;
    m44[1][1] = y.y;
    m44[2][1] = y.z;
    m44[3][1] = DotProduct(y,view);
    m44[0][2] = -z.x;
    m44[1][2] = -z.y;
    m44[2][2] = -z.z;
    m44[3][2] = DotProduct(z,view);
    m44[0][3] = 0;
    m44[1][3] = 0;
    m44[2][3] = 0;
    m44[3][3] = 1;

    return m44;
}

BlocksGame::BlocksGame()
{
    
}

void BlocksGame::init()
{
    // Load texture
    m_simpleTex = LoadImagePNG( gameDataFile("", "simpletex.png" ).c_str(), true, false );

    // Load font
    m_fontImg = LoadImagePNG( gameDataFile("", "nesfont.png" ).c_str(), true, false );
    m_nesFont = makeFont_nesfont_8( m_fontImg.textureId );    
    
    m_cube = make_cube();
//    setColorConstant( m_cube, vec4f( 1.0, 1.0, 1.0 ) );
    
//    m_groundTile = load_obj( gameDataFile("", "letter_f.obj").c_str() );
    m_testPost = load_obj( gameDataFile("", "test_post.obj").c_str() );
    
    
    QuadBuff<DrawVert> *person = load_obj( gameDataFile("", "person.obj").c_str() );
    SceneObj *playerObj = new SceneObj( person );
    playerObj->m_tintColor = vec3f( 1.0, 1.0, 0.0 );

    m_scene.push_back( playerObj );
    
    m_player = new Actor( playerObj);
    m_player->setPos( 1, 1 );
    
    PNGImage playerTex = LoadImagePNG( gameDataFile("", "skn_hero.png" ).c_str(), true, false );
    playerObj->m_texId = playerTex.textureId;
    
    m_lightObj = new SceneObj( "litebulb.obj" );
    m_scene.push_back( m_lightObj );
    
    m_world = new World();
    m_world->init();
    //m_world->createMap( m_scene );
    m_world->load( "ld48", m_scene );
    movePlayer( m_world->m_startPosX, m_world->m_startPosY );
    
    m_basicShader = loadShader( "minimalism.Plastic" );
    
    m_rotate = 0.0;
    m_testval = 0.0;
    m_useLookat = true;
    m_paused = true;


    m_camPos = vec3f( 20, 20, 40 );
    
    m_lightPos = vec3f( -1, 11, 18 );
    m_lightObj->m_xform.Translate( m_lightPos );

    glEnable( GL_DEPTH_TEST );
    
}

void BlocksGame::updateSim( float dtFixed )
{
    if (!m_paused)
    {
        // Update stuff with a fixed timestep
        m_rotate += (M_PI/180.0) * (30.0) * dtFixed;

        m_testval = sin( m_rotate * (M_PI/180.0) * 50.0 );

        // rotate camera around scene
        matrix4x4f matRotCam;
        matRotCam.RotateY( (M_PI/180.0) * (30.0) * dtFixed );

        m_camPos = m_camPos * matRotCam;
        m_camPos.y = fabs(m_testval) * 10.0;
    }
}

void BlocksGame::updateFree( float dtRaw )
{
    // Update stuff with free timestep
}

void BlocksGame::redraw()
{
//    glDisable( GL_DEPTH_TEST );

//    glClearColor( 78.0/255.0, 114.0/255.0, 136.0/255.0, 1.0 );
    glClearColor( m_world->m_bgColor.x, m_world->m_bgColor.y, m_world->m_bgColor.z, 1.0 );
            
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    
    // 3d stuff    
    glhPerspectivef2( m_proj, 20.0, 800.0/600.0, 1.0, 500.0 );

//    printf( "testval is %f\n", m_testval);
//    m_view.LookAt(vec3f(0.0, 0.0, 0.0), vec3f(0.0, 0.1, m_testval), vec3f( 0.0, 1.0, 0.0));
//    m_view.Inverse(true); // doing something weird here so i have to invert the lookat??
    m_view = LookAt2(vec3f(0.0,0.0,0.0), m_camPos, vec3f( 0.0, 1.0, 0.0));


#if 0
    printf(" Lookat %3.2f %3.2f %3.2f %3.2f\n"
            "        %3.2f %3.2f %3.2f %3.2f\n"
            "        %3.2f %3.2f %3.2f %3.2f\n"
            "        %3.2f %3.2f %3.2f %3.2f\n",
            m_view.m16[0],m_view.m16[1],m_view.m16[2],m_view.m16[3],
            m_view.m16[4],m_view.m16[5],m_view.m16[6],m_view.m16[7],
            m_view.m16[8],m_view.m16[9],m_view.m16[10],m_view.m16[11],
            m_view.m16[12],m_view.m16[13],m_view.m16[14],m_view.m16[15] );
#endif



#if 0
    printf(" view2 %3.2f %3.2f %3.2f %3.2f\n"
           "        %3.2f %3.2f %3.2f %3.2f\n"
           "        %3.2f %3.2f %3.2f %3.2f\n"
           "        %3.2f %3.2f %3.2f %3.2f\n",
            m_view2.m16[0],m_view2.m16[1],m_view2.m16[2],m_view2.m16[3],
            m_view2.m16[4],m_view2.m16[5],m_view2.m16[6],m_view2.m16[7],
            m_view2.m16[8],m_view2.m16[9],m_view2.m16[10],m_view2.m16[11],
            m_view2.m16[12],m_view2.m16[13],m_view2.m16[14],m_view2.m16[15] );

    printf ("=====================\n");
#endif


    m_model.Identity();
    //m_model.RotateY( m_rotate );

    // Draw 3D scene
    _draw3d();
    
    // set up 2D draw     
    glDisableClientState( GL_VERTEX_ARRAY );
    glDisableClientState( GL_COLOR_ARRAY );
        
    glEnable( GL_BLEND );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

    // 2d stuff
    // TODO: use shaders 
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    
    glOrtho( 0, 800, 0, 600, -1, 1 );
    
    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();

    _draw2d();

}

void BlocksGame::_prepViewMat()
{
    // combine model, view and proj
    m_modelViewProj = m_model * m_view * m_proj;
    m_modelView = m_model * m_view;

    // prep for lighting
    m_normalMatrix = m_modelView;
//    m_normalMatrix.Inverse( true);
//    m_normalMatrix.Transpose();

    GLfloat mnrm[9];
    mnrm[0] = m_normalMatrix.m16[0]; mnrm[1] = m_normalMatrix.m16[1]; mnrm[2] = m_normalMatrix.m16[2];
    mnrm[3] = m_normalMatrix.m16[4]; mnrm[4] = m_normalMatrix.m16[5]; mnrm[5] = m_normalMatrix.m16[6];
    mnrm[6] = m_normalMatrix.m16[8]; mnrm[7] = m_normalMatrix.m16[9]; mnrm[8] = m_normalMatrix.m16[10];


    // set this to the shader
    GLint mvp = glGetUniformLocation( m_basicShader, "matrixPMV");
    glUniformMatrix4fv( mvp, 1, 0, (GLfloat*)(&m_modelViewProj)  );

    GLint mv = glGetUniformLocation( m_basicShader, "matrixModelview");
    glUniformMatrix4fv( mv, 1, 0, (GLfloat*)(&m_modelView)  );

    GLint nrmMat = glGetUniformLocation( m_basicShader, "normalMatrix");
    glUniformMatrix3fv( nrmMat, 1, 0, mnrm  );

}


void BlocksGame::shutdown()
{
    // shutdown stuff
}

void BlocksGame::mouseMotion( float x, float y )
{
    
}

// For instantanious button effects 
void BlocksGame::mouseButton( SDL_MouseButtonEvent &btnEvent )
{
    
}

// For "continuous" button events
void BlocksGame::updateButtons( unsigned int btnMask )
{
    
}

// moves player to new tile, only if it is valid
void BlocksGame::movePlayer( int xx, int yy )
{
    printf("Moveplayer %d %d, walkable %s\n", xx, yy, m_world->m_map[xx][yy].isWalkable()?"YES":"NO" );           

    if (m_world->m_map[xx][yy].isWalkable())
    {
        m_player->setPos(xx, yy);
    }
}

void BlocksGame::keypress( SDLKey &key )
{
    switch(key)
    {
        case 'o':
            // orbit
//            doSpin = !doSpin;
//            printf("Orbit %s\n", doSpin?"true":"false" );
            break;

        case 'a':
            m_useLookat = !m_useLookat;
            break;

        case 'p':
            m_paused = !m_paused;
            break;

        case 'r':
            reloadShader();
            break;
            
        case SDLK_1:
            m_lightPos.x -= 1.0;
            break;
        case SDLK_2:
            m_lightPos.x += 1.0;
            break;
        case SDLK_3:
            m_lightPos.y -= 1.0;
            break;
        case SDLK_4:
            m_lightPos.y += 1.0;
            break;
        case SDLK_5:
            m_lightPos.z -= 1.0;
            break;
        case SDLK_6:
            m_lightPos.z += 1.0;
            break;

            
        case SDLK_UP:
            movePlayer(m_player->m_mapX, m_player->m_mapY-1);
            break;

        case SDLK_DOWN:
            movePlayer(m_player->m_mapX, m_player->m_mapY+1);
            break;

        case SDLK_LEFT:
            movePlayer(m_player->m_mapX-1, m_player->m_mapY);
            break;

        case SDLK_RIGHT:
            movePlayer(m_player->m_mapX+1, m_player->m_mapY);
            break;

            
        default:
            break;
    }
    
    printf("lightPos: %f %f %f\n",m_lightPos.x, m_lightPos.y, m_lightPos.z );
    m_lightObj->m_xform.Translate( m_lightPos );
    
}

void BlocksGame::reloadShader()
{
    printf("reload shader...\n");
    glDeleteProgram( m_basicShader );
    m_basicShader = loadShader( "minimalism.Plastic" );
}

    // Draw 3d stuff
void BlocksGame::_draw3d()
{
    glEnable( GL_TEXTURE_2D );

    // Set up basic shader
    glUseProgram( m_basicShader );

    glActiveTexture(GL_TEXTURE0 );
    glBindTexture( GL_TEXTURE_2D, m_simpleTex.textureId );
    
    GLint paramTex = glGetUniformLocation( m_basicShader, "sampler_dif0" );
    glUniform1i( paramTex, 0 );        

    vec3f lightDir = m_lightPos;
    lightDir.Normalize();
    
    GLint lightPos0 = glGetUniformLocation( m_basicShader, "lightPos0");
    glUniform3f( lightPos0, m_lightPos.x, m_lightPos.y, m_lightPos.z );

    GLint lightDir0 = glGetUniformLocation( m_basicShader, "lightDir0");
    glUniform3f( lightDir0, lightDir.x, lightDir.y, lightDir.z );

    
    // Draw something
//    _drawMesh( m_testPost );

    // draw player
//    m_model.Translate( m_playerPos.x - 9.5, m_playerPos.y, m_playerPos.z - 9.5 );
//    _prepViewMat();
//    _drawMesh( m_person );
    
    // draw scene
    for (auto si = m_scene.begin(); si != m_scene.end(); ++si)
    {
        SceneObj *obj = *si;
        m_model = obj->m_xform;
        _prepViewMat();

        // Set tint color
        GLint tint = glGetUniformLocation( m_basicShader, "Kd");
        glUniform3f( tint, obj->m_tintColor.x, obj->m_tintColor.y, obj->m_tintColor.z );
        
        // set texture?
        glBindTexture( GL_TEXTURE_2D, obj->m_texId );
        GLint mixVal = glGetUniformLocation( m_basicShader, "mixVal");
        glUniform1f( mixVal, obj->m_texId?1.0:0.0 );

        _drawMesh( obj->m_mesh );
    }


//    for (int i=0; i < 20; i++)
//    {
//        for (int j=0; j < 20; j++)
//        {
//            _drawGroundTile(i,j,0);
//        }
//    }
}

void BlocksGame::_drawGroundTile( int x, int y, int hite)
{
    vec3f tileLoc( x - 10.0, hite-0.5, y - 10.0 );

    m_model.Translate(tileLoc);
    _prepViewMat();

//    _drawMesh(m_groundTile);

}

// Draw 2D stuff    
void BlocksGame::_draw2d()
{
    // disable shaders (TODO: make a text shader so this ports to ES2 easier)
    glUseProgram(0);
    
    m_nesFont->setColor(1.0, 1.0, 1.0, 1.0);
    m_nesFont->drawStringCentered( 400, 580, m_useLookat?"Lookat":"View2" );

    // Draw fps
    char buff[50];
    sprintf( buff, "fps: %d objs: %d", g_fps, m_scene.size() );
    m_nesFont->drawString( 50, 580, buff );

    /*
    matrix4x4f m = m_view;
    sprintf( buff, "%5.2f %5.2f %5.2f %5.2f", m.m16[0], m.m16[1], m.m16[2], m.m16[3] );
    m_nesFont->drawString( 50, 500, buff );
    sprintf( buff, "%5.2f %5.2f %5.2f %5.2f", m.m16[4], m.m16[5], m.m16[6], m.m16[7] );
    m_nesFont->drawString( 50, 490, buff );
    sprintf( buff, "%5.2f %5.2f %5.2f %5.2f", m.m16[8], m.m16[9], m.m16[10], m.m16[11] );
    m_nesFont->drawString( 50, 480, buff );
    sprintf( buff, "%5.2f %5.2f %5.2f %5.2f", m.m16[12], m.m16[13], m.m16[14], m.m16[15] );
    m_nesFont->drawString( 50, 470, buff );
*/


    m_nesFont->renderAll();
    m_nesFont->clear();

}

void BlocksGame::_drawMesh( QuadBuff<DrawVert> *mesh )
{
    // Buffer is already created, just bind it
    glBindBuffer( GL_ARRAY_BUFFER, mesh->vbo() );

    // Bind buffer data
    glEnableVertexAttribArray( DrawVertAttrib_TEXCOORD );
    glVertexAttribPointer( DrawVertAttrib_TEXCOORD, 4, GL_FLOAT, GL_FALSE, 
                          sizeof(DrawVert), (void*)offset_s( DrawVert, m_st ) );

    glEnableVertexAttribArray( DrawVertAttrib_POSITION );
    glVertexAttribPointer( DrawVertAttrib_POSITION, 3, GL_FLOAT, GL_FALSE, 
                          sizeof(DrawVert), (void*)offset_s( DrawVert, m_pos) );


    glEnableVertexAttribArray( DrawVertAttrib_NORMAL );
    glVertexAttribPointer( DrawVertAttrib_NORMAL, 3, GL_FLOAT, GL_FALSE, 
                          sizeof(DrawVert), (void*)offset_s( DrawVert, m_nrm) );

    glEnableVertexAttribArray( DrawVertAttrib_COLOR );
    glVertexAttribPointer( DrawVertAttrib_COLOR, 4, GL_UNSIGNED_BYTE, GL_TRUE, 
                          sizeof(DrawVert), (void*)offset_s( DrawVert, m_color) );

    // Draw it!
    glDrawArrays(GL_TRIANGLES, 0, mesh->size() );
}
