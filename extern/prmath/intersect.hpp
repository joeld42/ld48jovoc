/*
	Twilight Prophecy SDK
	A multi-platform development system for virtual reality and multimedia.

	Copyright (C) 1997-2003 Twilight 3D Finland Oy Ltd.
*/
#ifndef PRMATH_INTERSECT_HPP
#define PRMATH_INTERSECT_HPP


#include "minmax.hpp"
#include <cmath>
#include <limits>
#include "vector3.hpp"
#include "cone.hpp"
#include "sphere.hpp"
#include "ray.hpp"
#include "line.hpp"
#include "box.hpp"
#include "plane.hpp"


namespace prmath
{

	template <typename T>
	bool IntersectRayPlane(const Ray<T>& ray, const Plane<T>& plane, T& enter)
	{
		T ndot = plane.dist - DotProduct(ray.origin,plane.normal);

		// backface culling: intersection cannot occur if origin
		// of the ray is behind the plane
		if ( ndot > 0 )
			return false;

		T vdot = DotProduct(ray.direction,plane.normal);

		// is line parallel to the plane? if so, even if the line is
		// at the plane it is not considered as intersection because
		// it would be impossible to determine the point of intersection
		if ( vdot == 0 )
			return false;

		// the resulting intersection is behind the origin of the ray
		// if the result is negative ( enter < 0 )
		enter = ndot / vdot;

		return true;
	}

	template <typename T>
	inline bool IntersectLinePlane(const Line<T>& line, const Plane<T>& plane, T& time)
	{
		const Vector3<T>& origin = line.a;
		Vector3<T> direction = line.b - line.a;

		T ndot = plane.dist - DotProduct(origin,plane.normal);
		T vdot = DotProduct(direction,plane.normal);

		// is line parallel to the plane? if so, even if the line is
		// at the plane it is not considered as intersection because
		// it would be impossible to determine the point of intersection
		if ( vdot == 0 )
			return false;

		// the resulting intersection is in the line segment from a - b
		// if the intersect time is between 0 and 1
		time = ndot / vdot;

		return true;
	}

	template <typename T>
	inline bool IntersectRaySphere(const Ray<T>& ray, const Sphere<T>& sphere, T& enter)
	{
		Vector3<T> dep = sphere.center - ray.origin;
		T dot3 = DotProduct(dep,ray.direction);
		T det = dot3 * dot3 - LengthSquared(dep) + sphere.radius * sphere.radius;

		// no intersection
		if ( det < 0 )
			return false;

		// positive solution, or tangent (det==0)
		det = static_cast<T>(sqrt(det));
		T time = dot3 + det;

		// intersection behind the origin for positive solution?
		if ( time < 0 )
		{
			// try negative solution, since the positive was behind the origin
			time = dot3 - det;
			if ( time < 0 ) 
				return false; // also behind origin
		}
		else
		{
			// negative solution
			if ( det > 0 )
			{
				T to = dot3 - det;
				// use negative solution only if closer to origin, and not behind it
				if ( (to < time) && (to >= 0) )
					time = to;
			}
		}

		// intersection result
		enter = time;

		return true;
	}

	template <typename T>
	inline bool IntersectRaySphere(const Ray<T>& ray, const Sphere<T>& sphere, T& enter, T& leave)
	{
		T b = -2 * (DotProduct(ray.origin,ray.direction) - DotProduct(sphere.center,ray.direction));
		T c = DotProduct(ray.origin,ray.origin) + DotProduct(sphere.center,sphere.center) - 
			2 * DotProduct(ray.origin,sphere.center) - sphere.radius * sphere.radius;
		
		T det = b * b - 4 * c;
		if ( det >= 0 )
		{
			T sd = static_cast<T>(sqrt(det));
			T t0 = (b + sd) * static_cast<T>(0.5);
			T t1 = (b - sd) * static_cast<T>(0.5);
			if ( t0 > t1 )
			{
				enter = t1;
				leave = t0;
			}
			else
			{
				enter = t0;
				leave = t1;
			}
			return true;
		}
		return false;
	}

	template <typename T>
	inline bool IntersectRayTriangle(const Ray<T>& ray, const Vector3<T>& v0, const Vector3<T>& v1, const Vector3<T>& v2, T& enter)
	{
		// intersect the ray with plane
		Plane<T> plane(v0,v1,v2);
		if ( !IntersectRayPlane(ray,plane,enter) )
			return false;

		// intersection point
		Vector3<T> point = ray.origin + ray.direction * enter;

		// test if intersection point is inside the triangle
		const Vector3<T>& origin = ray.origin;
		Vector3<T> normal;
	
		// edge 1
		normal = CrossProduct(origin-v1,origin-v0);
		if ( DotProduct(point,normal) < DotProduct(origin,normal) )
			return false;
		
		// edge 2
		normal = CrossProduct(origin-v2,origin-v1);
		if ( DotProduct(point,normal) < DotProduct(origin,normal) )
			return false;
		
		// edge 3
		normal = CrossProduct(origin-v0,origin-v2);
		if ( DotProduct(point,normal) < DotProduct(origin,normal) )
			return false;
		
		return true;
	}

	template <typename T>
	inline bool IntersectRayTriangleBarycentricCullON(const Ray<T>& ray, const Vector3<T>& v0, const Vector3<T>& v1, const Vector3<T>& v2, T& enter, T& bary0, T& bary1)
	{
		// =====================================
		// Based on article by Tomas Möller
		// Fast, Minimum Storage Ray-Triangle Intersection
		// =====================================

		Vector3<T> edge1 = v1 - v2;
		Vector3<T> edge2 = v0 - v2;

		Vector3<T> pvec = CrossProduct(ray.direction,edge2);
		T det = DotProduct(edge1,pvec);

		const T epsilon = static_cast<T>(0.000001);
		if ( det < epsilon )
			return false;

		Vector3<T> tvec = ray.origin - v2;
		T u = DotProduct(tvec,pvec);
		if ( u < 0 || u > det )
			return false;

		Vector3<T> qvec = CrossProduct(tvec,edge1);
		T v = DotProduct(ray.direction,qvec);
		if ( v < 0 || (u + v) > det )
			return false;

		det = 1 / det;

		// write result
		enter = DotProduct(edge2,qvec) * det;
		bary0 = v * det;
		bary1 = u * det;

		return true;
	}

	template <typename T>
	inline bool IntersectRayTriangleBarycentricCullOFF(const Ray<T>& ray, const Vector3<T>& v0, const Vector3<T>& v1, const Vector3<T>& v2, T& enter, T& bary0, T& bary1)
	{
		// =====================================
		// Based on article by Tomas Möller
		// Fast, Minimum Storage Ray-Triangle Intersection
		// =====================================

		Vector3<T> edge1 = v1 - v2;
		Vector3<T> edge2 = v0 - v2;

		Vector3<T> pvec = CrossProduct(ray.direction,edge2);
		T det = DotProduct(edge1,pvec);

		const T epsilon = static_cast<T>(0.000001);
		if ( det > -epsilon && det < epsilon )
			return false;

		det = 1 / det;

		Vector3<T> tvec = ray.origin - v2;
		T u = DotProduct(tvec,pvec) * det;
		if ( u < 0 || u > 1 )
			return false;

		Vector3<T> qvec = CrossProduct(tvec,edge1);
		T v = DotProduct(ray.direction,qvec) * det;
		if ( v < 0 || (u + v) > 1 )
			return false;

		// write result
		enter = DotProduct(edge2,qvec) * det;
		bary0 = v;
		bary1 = u;

		return true;
	}

	template <typename T>
	inline bool IntersectRayBox(const Ray<T>& ray, const Box<T>& box, T& enter, T& leave)
	{
		// =====================================
		// Kay and Kayjia "slab" method
		// =====================================
	
		T tnear = -std::numeric_limits<T>::max();
		T tfar = std::numeric_limits<T>::max();
	
		for ( int i=0; i<3; ++i )
		{
			T origin = ray.origin[i];
			T direction = ray.direction[i];
			T vmin = box.vmin[i];
			T vmax = box.vmax[i];
		
			if ( direction )
			{
				// calculate intersections to the slab
				direction = 1 / direction;
				T t1 = (vmin - origin) * direction;
				T t2 = (vmax - origin) * direction;
			
				if ( t1 > t2 )
				{
					if ( t2 > tnear ) tnear = t2;
					if ( t1 < tfar ) tfar = t1;
				}
				else
				{
					if ( t1 > tnear ) tnear = t1;
					if ( t2 < tfar ) tfar = t2;
				}
		
				// failed intersecting this axis or intersecting behind the origin
				if ( tnear > tfar || tfar < 0 )
					return false;
			}
			else
			{
				// ray parallel to axis/slab being tested against
				if ( origin < vmin || origin > vmax )
					return false;
			}
		}

		// intersection result
		enter = tnear;
		leave = tfar;

		// ray hits box
		return true;
	}

	template <typename T>
	inline bool PlaneIntersect(const Plane<T>& p0, const Plane<T>& p1, Vector3<T>& point, Vector3<T>& direction)
	{
		direction = CrossProduct(p0.normal,p1.normal);
		T sqrlen = LengthSquared(direction);
	
		// parallel?
		if ( sqrlen < static_cast<T>(0.000001) ) 
			return false;

		// intersection result
		point = (p0.dist * CrossProduct(p1.normal,direction) + p1.dist * CrossProduct(direction,p0.normal)) / sqrlen;
		direction /= static_cast<T>(sqrt(sqrlen)); // normalize direction vector

		return true;
	}

	template <typename T>
	inline bool PlaneIntersect(const Plane<T>& p0, const Plane<T>& p1, const Plane<T>& p2, Vector3<T>& point)
	{
		// determinant
		Vector3<T> cp01 = CrossProduct(p0.normal,p1.normal);
		T det = DotProduct(cp01,p2.normal);
	
		// parallel?
		if ( det < static_cast<T>(0.001) ) 
			return false;

		// intersection
		Vector3<T> cp12 = CrossProduct(p1.normal,p2.normal) * p0.dist;
		Vector3<T> cp20 = CrossProduct(p2.normal,p0.normal) * p1.dist;

		point = (cp12 + cp20 + p2.dist * cp01) / det;
		return true;
	}

	template <typename T>
	inline bool PointInsideTriangle(const Vector3<T>& v0, const Vector3<T>& v1, const Vector3<T>& v2, const Vector3<T>& point)
	{
		Vector3<T> v10 = v1 - v0;
		Vector3<T> v20 = v2 - v0;
		Vector3<T> vp0 = point - v0;
		T b = DotProduct(v10,v20);
		T c = DotProduct(v20,v20);
		T d = DotProduct(vp0,v10);
		T e = DotProduct(vp0,v20);

		T x = d * c - e * b;
		if ( x < 0 )
			return false;

		T a = DotProduct(v10,v10);
		T y = e * a - d * b;
		if ( y < 0 )
			return false;

		return (x + y) <= (a * c - b * b);
	}

	template <typename T>
	inline T GetTriangleArea(const T& edge0, const T& edge1, const T& edge2)
	{
		// heron's formula
		T s = (edge0 + edge1 + edge2) * static_cast<T>(0.5);
		return static_cast<T>(sqrt(s * (s-edge0) * (s-edge1) * (s-edge2)));
	}

	template <typename T>
	inline Vector3<T> ClosestPointOnLine(const Line<T>& line, const Vector3<T>& point)
	{
		Vector3<T> v = line.b - line.a;

		T time = DotProduct(v,point-line.a);
		if ( time <= 0 )
			return line.a;

		T sqrlen = LengthSquared(v);
		if ( time >= sqrlen )
			return line.b;

		return line.a + (v * time / sqrlen);
	}

	template <typename T>
	inline bool IntersectConeSphere(const Cone<T>& cone, const Sphere<T>& sphere)
	{
		// test if cone vertex is in sphere
		Vector3<T> diff = sphere.center - cone.origin;
	
		T r2 = sphere.radius * sphere.radius;
		T len2 = DotProduct(diff,diff);
		T len2mr2 = len2 - r2;
		if ( len2mr2 <= 0 ) 
			return true;

		// test if sphere center is in cone
		T dot1 = DotProduct(cone.target,diff);
		T dot2 = dot1 * dot1;
		T cs = static_cast<T>(cos(cone.angle));
		T cs2 = cs * cs;
		if ( dot2 >= len2*cs2 ) 
			return true;
	
		T ulen = static_cast<T>(sqrt(fabs(len2-dot2)));
		T sn = static_cast<T>(sin(cone.angle));
		T test = cs*dot1 + sn*ulen;
		T discr = test*test - len2mr2;
	
		return discr >= 0 && test >= 0;
	}

	template <typename T>
	inline bool IntersectBoxSphere(const Box<T>& box, const Sphere<T>& sphere)
	{
		T fmin = 0;
		for ( int i=0; i<3; ++i )
		{
			const T& center = sphere.center[i];
			const T& vmin = box.vmin[i];
			const T& vmax = box.vmax[i];

			if ( center < vmin )
			{
				T delta = center - vmin;
				fmin += delta * delta;
			}
			else if ( center > vmax )
			{
				T delta = center - vmax;
				fmin += delta * delta;
			}
		}
		T r2 = sphere.radius * sphere.radius;
		return fmin <= r2 ? true : false;
	}

} // namespace prmath


#endif
