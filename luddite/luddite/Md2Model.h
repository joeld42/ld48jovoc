/* -*- c++ -*- */
/////////////////////////////////////////////////////////////////////////////
//
// Md2Model.h -- Copyright (c) 2005-2006 David Henry
// last modification: feb. 25, 2006
//
// This code is licenced under the MIT license.
//
// This software is provided "as is" without express or implied
// warranties. You may freely copy and compile this source into
// applications you distribute provided that the copyright text
// below is included in the resulting source code.
//
// Definition of MD2 Model Classes.
//
/////////////////////////////////////////////////////////////////////////////

#ifndef __MD2MODEL_H__
#define __MD2MODEL_H__

#include <vector>
#include <string>
#include <map>

using std::map;
using std::vector;
using std::string;


// OpenGL vector types
typedef GLfloat vec2_t[2];
typedef GLfloat vec3_t[3];


// Md2 header
struct Md2Header_t
{
  int ident;          // Magic number, "IDP2"
  int version;        // Md2 format version, should be 8

  int skinwidth;      // Texture width
  int skinheight;     // Texture height

  int framesize;      // Size of a frame, in bytes

  int num_skins;      // Number of skins
  int num_vertices;   // Number of vertices per frame
  int num_st;         // Number of texture coords
  int num_tris;       // Number of triangles
  int num_glcmds;     // Number of OpenGL commands
  int num_frames;     // Number of frames

  int offset_skins;   // offset to skin data
  int offset_st;      // offset to texture coords
  int offset_tris;    // offset to triangle data
  int offset_frames;  // offset to frame data
  int offset_glcmds;  // offset to OpenGL commands
  int offset_end;     // offset to the end of the file
};


// Skin data
struct Md2Skin_t
{
  char name[64];  // Texture's filename
};


// Texture coords.
struct Md2TexCoord_t
{
  short s;
  short t;
};


// Triangle data
struct Md2Triangle_t
{
  unsigned short vertex[3];  // Triangle's vertex indices
  unsigned short st[3];      // Texture coords. indices
};


// Vertex data
struct Md2Vertex_t
{
  unsigned char v[3];         // Compressed vertex position
  unsigned char normalIndex;  // Normal vector index
};


// Frame data
struct Md2Frame_t
{
  // Destructor
  ~Md2Frame_t () {
    delete [] verts;
  }

  vec3_t scale;        // Scale factors
  vec3_t translate;    // Translation vector
  char name[16];       // Frame name
  Md2Vertex_t *verts;  // Frames's vertex list
};


// OpenGL command packet
struct Md2Glcmd_t
{
  float s;    // S texture coord.
  float t;    // T texture coord.
  int index;  // Vertex index
};


// Animation infos
struct Md2Anim_t
{
  int start;  // first frame index
  int end;    // last frame index
};


/////////////////////////////////////////////////////////////////////////////
//
// class Md2Model -- MD2 Model Data Class.
//
/////////////////////////////////////////////////////////////////////////////

class Md2Model
{
public:
  // Constructors/destructor
  Md2Model (const string &filename);
  ~Md2Model ();

public:
  // Internal types
  typedef map<string, GLuint> SkinMap;
  typedef map<string, Md2Anim_t> AnimMap;

public:
  // Public interface
  bool loadTexture (const string &filename);
  void setTexture (const string &filename);

  void renderFrameImmediate (int frame);
  void drawModelItpImmediate (int frameA, int frameB, float interp);

  void renderFrameWithGLcmds (int frame);
  void drawModelItpWithGLcmds (int frameA, int frameB, float interp);

  void setScale (GLfloat scale) { _scale = scale; }

  // Accessors
  const SkinMap &skins () const { return _skinIds; }
  const AnimMap &anims () const { return _anims; }

private:
  // Internal functions
  void setupAnimations ();

private:
  // Member variables

  // Constants
  static vec3_t _kAnorms[162];
  static int _kMd2Ident;
  static int _kMd2Version;

  // Model data
  Md2Header_t _header;
  Md2Skin_t *_skins;
  Md2TexCoord_t *_texCoords;
  Md2Triangle_t *_triangles;
  Md2Frame_t *_frames;
  int *_glcmds;

  GLfloat _scale;
  GLuint _tex;

  SkinMap _skinIds;
  AnimMap _anims;
};


/////////////////////////////////////////////////////////////////////////////
//
// class Md2Object -- MD2 Object Class.
//
/////////////////////////////////////////////////////////////////////////////

class Md2Object
{
public:
  // Public internal types/enums
  enum Md2RenderMode
    {
      kDrawImmediate = 0,
      kDrawGLcmds,
    };

public:
  // Constructor/destructor
  Md2Object ();
  Md2Object (Md2Model *model);
  ~Md2Object ();

public:
  // Public interface
  void drawObjectItp (bool animated, Md2RenderMode renderMode);
  void drawObjectFrame (int frame, Md2RenderMode renderMode);
  void animate (int startFrame, int endFrame, float percent);
  void animate (float percent);

  void setModel (Md2Model *model);
  void setScale (float scale) { _scale = scale; }
  void setAnim (const string &name);

  // Accessors
  const Md2Model *model () const { return _model; }
  float scale () const { return _scale; }
  const string &currentAnim () const { return _currentAnim; }

private:
  // Member variables
  Md2Model *_model;

  int _currFrame;
  int _nextFrame;
  float _interp;

  float _percent;
  float _scale;

  // Animation data
  const Md2Anim_t *_animInfo;
  string _currentAnim;
};

#endif // __MD2_H__
