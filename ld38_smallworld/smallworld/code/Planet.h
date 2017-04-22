#pragma once

#include "Pre.h"
#include "Core/Main.h"
#include "Gfx/Gfx.h"
#include "Input/Input.h"

#include "IO/IO.h"
#include "LocalFS/LocalFileSystem.h"

#include "Assets/Gfx/ShapeBuilder.h"
#include "Assets/Gfx/MeshBuilder.h"
#include "Assets/Gfx/MeshLoader.h"
#include "Assets/Gfx/TextureLoader.h"

#include "glm/gtc/random.hpp"
#include "glm/gtc/constants.hpp"
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/quaternion.hpp"

#include "shaders.h"
#include "IsosurfaceBuilder.h"
#include "SceneObject.h"
#include "Camera.h"

class Planet {
public:
    Planet();
    
    IsosurfaceBuilder surfBuilder;
    
    Oryol::VertexLayout surfLayout;
    
    Oryol::DrawState planetDrawState;
    Oryol::Id meshIsosurf;
    Oryol::Id planetShader;
    Oryol::Id planetTexture;
    
    Oryol::PlanetShader::VSParams planetVSParams;
    Oryol::PlanetShader::FSParams planetFSParams;
    
    void Setup( Oryol::GfxSetup *gfxSetup );
    void Rebuild( Scene *scene );    
    void Draw();
    
    void UpdateCamera( Camera *camera );
    
    bool _built;
};
