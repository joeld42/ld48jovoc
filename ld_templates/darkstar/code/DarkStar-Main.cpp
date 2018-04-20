//------------------------------------------------------------------------------
//  TestApp.cc
//------------------------------------------------------------------------------
#include <unistd.h>

#include "Pre.h"
#include "Core/Main.h"
#include "Gfx/Gfx.h"
#include "Assets/Gfx/ShapeBuilder.h"
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "shaders.h"

#include "IO/IO.h"
#include "LocalFS/LocalFileSystem.h"

#include "Camera.h"
#include "SceneObject.h"
#include "DarkStarApp.h"

#define STB_IMAGE_IMPLEMENTATION
#define STBI_NO_STDIO
#define STBI_ONLY_PNG
#define STBI_NO_SIMD
#include "stb_image.h"

#define PAR_EASINGS_IMPLEMENTATION
#include "par_easings.h"

using namespace Oryol;

OryolMain(DarkStarApp);


