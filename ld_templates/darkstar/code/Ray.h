#ifndef TK_RAY_H
#define TK_RAY_H

#include "Core/Types.h"

#include "glm/vec2.hpp"
#include "glm/vec3.hpp"
#include "glm/mat4x4.hpp"

namespace Tapnik
{
    struct Ray
    {
        glm::vec3 pos;
        glm::vec3 dir;
    };
    
    struct RayResult
    {
        bool hit;
        float hitDist;
        glm::vec3 hitPoint;
    };
    
    bool RayIsectAABB( Ray ray, glm::vec3 bboxMin, glm::vec3 bboxMax );
    
} // namespace Tapnik
#endif
