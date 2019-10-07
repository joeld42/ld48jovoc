//------------------------------------------------------------------------------
//  Camera.cc
//------------------------------------------------------------------------------
#include <stdio.h>

#include "Pre.h"
#include "Camera.h"

#define GLM_SWIZZLE
#include "glm/glm.hpp"
#include "glm/trigonometric.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/matrix_access.hpp"

void dbgPrintMatrix( const char *label, glm::mat4 m );

using namespace Oryol;
using namespace Tapnik;

//------------------------------------------------------------------------------
void
Camera::Setup(const glm::vec3 pos, float fov, int dispWidth, int dispHeight, float near, float far) {
    this->Pos = pos;
    this->Model = glm::translate(glm::mat4(), pos);
	this->UpdateProj(fov, dispWidth, dispHeight, near, far);
}

void Camera::SetupShadow(const glm::vec3 pos, float hite, int dispWidth, int dispHeight, float shadNear, float shadFar) {
	this->Pos = pos;
	this->Model = glm::translate(glm::mat4(), pos);

	//this->UpdateProj(20.0f, dispWidth, dispHeight, near, far);

	// Doesn't work ??
	//this->Proj = glm::perspectiveFov( 20.0f, float(dispWidth), float(dispHeight), near, far);
	this->Proj = glm::ortho(-hite, hite, -hite, hite, shadNear, shadFar);
	

}


//------------------------------------------------------------------------------
void
Camera::UpdateProj(float fov, int dispWidth, int dispHeight, float near, float far) {
    this->Proj = glm::perspectiveFov(fov, float(dispWidth), float(dispHeight), near, far);    
    
    this->updateViewProjFrustum();
    
    displayWidth = dispWidth;
    displayHeight = dispHeight;
}

//------------------------------------------------------------------------------
void
Camera::UpdateModel(const glm::mat4& model) {
    this->Model = model;
    this->updateViewProjFrustum();
}

//------------------------------------------------------------------------------
void
Camera::MoveRotate(const glm::vec3& move, const glm::vec2& rot) {
    const glm::vec3 up(0.0f, 0.0f, 1.0f);
    const glm::vec3 hori(1.0f, 0.0f, 0.0f);
    this->Rot += rot;
    glm::mat4 m = glm::translate(glm::mat4(), this->Pos);
    m = glm::rotate(m, this->Rot.x, up);
    m = glm::rotate(m, this->Rot.y, hori);
    m = glm::translate(m, move);
    this->Model = m;
    this->Pos = glm::vec3(this->Model[3].x, this->Model[3].y, this->Model[3].z);
    this->updateViewProjFrustum();
}

void
Camera::MoveCrappy( const glm::vec3& move ) {
    glm::mat4 m = glm::translate(glm::mat4(), this->Pos);
    m = glm::translate(m, move);
    this->Model = m;
    this->Pos = glm::vec3(this->Model[3].x, this->Model[3].y, this->Model[3].z);
    this->updateViewProjFrustum();

}

Ray
Camera::getCameraRay( int px, int py )
{
    glm::vec4 ndcA = glm::vec4( 2.0f * ((float(px) / float(displayWidth)) - 0.5f),
                                -2.0f * ((float(py) / float(displayHeight)) - 0.5f),
                                -1.0, 1.0f );
    glm::vec4 ndcB = ndcA;
    ndcB.z = 0.0f;
    
    glm::vec4 a = mvpInverse * ndcA;
    a.w = 1.0 / a.w;
    a.x = a.x * a.w;
    a.y = a.y * a.w;
    a.z = a.z * a.w;
    
    glm::vec4 b = mvpInverse * ndcB;
    b.w = 1.0 / b.w;
    b.x = b.x * b.w;
    b.y = b.y * b.w;
    b.z = b.z * b.w;
    
    
    //printf("A is %3.2f %3.2f %3.2f %3.2f\n", a.x, a.y, a.z, a.w );
    
    Ray ray;
    ray.pos = glm::vec3(a);
    ray.dir = glm::normalize( glm::vec3(b - a) );
    
    return ray;
}

glm::vec4 Camera::worldPointToNDC( glm::vec3 worldPnt)
{
    glm::vec4 worldPointW = glm::vec4( worldPnt, 1.0 );
    glm::vec4 ndcPoint = ViewProj * worldPointW;
    
    if (fabs(ndcPoint.w) > 0.001) {
        ndcPoint = ndcPoint * (1.0f/ndcPoint.w);
    }
    
    return ndcPoint;
}

glm::vec2 Camera::worldPointToScreen( glm::vec3 worldPnt )
{
    glm::vec4 ndcPoint = worldPointToNDC( worldPnt );
    
    // perspective divide, NDC to screen...
    glm::vec2 ndcScreen( ndcPoint );
    ndcScreen = (ndcScreen + glm::vec2( 1.0 )) * 0.5f;
    return glm::vec2( ndcScreen.x * displayWidth, (1.0-ndcScreen.y) * displayHeight );

}


//------------------------------------------------------------------------------
bool
Camera::testPlane(const glm::vec4& p, float x0, float x1, float y0, float y1, float z0, float z1) {
    // see: https://github.com/nothings/stb/blob/master/tests/caveview/cave_render.c
    float d=0.0f;
    d += p.x > 0.0f ? x1 * p.x : x0 * p.x;
    d += p.y > 0.0f ? y1 * p.y : y0 * p.y;
    d += p.z > 0.0f ? z1 * p.z : z0 * p.z;
    d += p.w;
    return d >= 0.0f;
}

//------------------------------------------------------------------------------
bool
Camera::BoxVisible(int x0, int x1, int y0, int y1, int z0, int z1) const {
    // see: https://github.com/nothings/stb/blob/master/tests/caveview/cave_render.c
    for (int i = 0; i < 6; i++) {
        if (!testPlane(this->Frustum[i], x0, x1, y0, y1, z0, z1)) {
            return false;
        }
    }
    return true;
}

//------------------------------------------------------------------------------
void
Camera::updateViewProjFrustum() {
    this->View = glm::inverse(this->Model);
    this->ViewProj = this->Proj * this->View;
    
    // TODO: dirty flag, mark as needed?
    mvpInverse = glm::inverse(this->ViewProj);

    // extract frustum
    // https://fgiesen.wordpress.com/2012/08/31/frustum-planes-from-the-projection-matrix/
    // http://gamedevs.org/uploads/fast-extraction-viewing-frustum-planes-from-world-view-projection-matrix.pdf
    glm::vec4 rx = glm::row(this->ViewProj, 0);
    glm::vec4 ry = glm::row(this->ViewProj, 1);
    glm::vec4 rz = glm::row(this->ViewProj, 2);
    glm::vec4 rw = glm::row(this->ViewProj, 3);

    this->Frustum[0] = rw + rx;
    this->Frustum[1] = rw - rx;
    this->Frustum[2] = rw + ry;
    this->Frustum[3] = rw - ry;
    this->Frustum[4] = rw + rz;
    this->Frustum[5] = rw - rz;
}

