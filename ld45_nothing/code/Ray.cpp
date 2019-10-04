
#include <math.h>
#include "Ray.h"

using namespace Tapnik;

bool Tapnik::RayIsectAABB( Ray ray, glm::vec3 bboxMin, glm::vec3 bboxMax )
{
    bool result = false;
    
    float tx0 = (bboxMin.x - ray.pos.x)/ray.dir.x;
    float tx1 = (bboxMax.x - ray.pos.x)/ray.dir.x;
    
    float ty0 = (bboxMin.y - ray.pos.y)/ray.dir.y;
    float ty1 = (bboxMax.y - ray.pos.y)/ray.dir.y;
    
    float tz0 = (bboxMin.z - ray.pos.z)/ray.dir.z;
    float tz1 = (bboxMax.z - ray.pos.z)/ray.dir.z;
    
    float tA = (float)fmax(fmax(fmin(tx0, tx1), fmin(ty0, ty1)), fmin(tz0, tz1));
    float tB = (float)fmin(fmin(fmax(tx0, tx1), fmax(ty0, ty1)), fmax(tz0, tz1));
    
    result = !(tB < 0 || tA> tB);

    
    return result;
}
