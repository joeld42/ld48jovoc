#pragma once
//------------------------------------------------------------------------------
/**
    @class Camera
    @brief camera attributes, generated view/proj matrices, clipping
*/
#include "Core/Types.h"
#include "glm/vec2.hpp"
#include "glm/vec3.hpp"
#include "glm/mat4x4.hpp"

#include "Ray.h"

namespace Tapnik
{
        
class Camera {
public:
    /// initialize camera attributes
    void Setup(const glm::vec3 pos, float fov, int dispWidth, int dispHeight, float near, float far);


	void SetupShadow(const glm::vec3 pos, float hite, int dispWidth, int dispHite, float shadNear, float shadFar );

    /// update projection matrix (call when display size changes)
    void UpdateProj(float fov, int dispWidth, int dispHeight, float near, float far);
    /// directly set the model matrix
    void UpdateModel(const glm::mat4& model);
    /// move and rotate relative to current view
    void MoveRotate(const glm::vec3& move, const glm::vec2& rot);
    // move only, not relative to view
    void MoveCrappy( const glm::vec3& move );
    /// return true if box is visible
    bool BoxVisible(int x0, int x1, int y0, int y1, int z0, int z1) const;
    
    Ray getCameraRay( int px, int py );
    
    glm::vec4 worldPointToNDC( glm::vec3 worldPnt);
    glm::vec2 worldPointToScreen( glm::vec3 worldPnt );    
    
    /// the camera's world-space matrix
    glm::mat4 Model;
    /// the view matrix
    glm::mat4 View;
    /// the projection matrix
    glm::mat4 Proj;
    /// the ViewProj matrix
    glm::mat4 ViewProj;
    /// view frustum
    static const int NumFrustumPlanes = 6;
    glm::vec4 Frustum[NumFrustumPlanes];
    /// current camera position
    glm::vec3 Pos;
    /// current camera rotation
    glm::vec2 Rot;
    
    // Should we save these?
    int displayWidth, displayHeight;
    glm::mat4x4 mvpInverse;

    /// update the viewProj matrix and Frustum (called when view or proj changes)
    void updateViewProjFrustum();
    /// test if box is behind plane
    static bool testPlane(const glm::vec4& plane, float x0, float x1, float y0, float y1, float z0, float z1);
};
    
}
