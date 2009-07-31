/*
	Twilight Prophecy SDK
	A multi-platform development system for virtual reality and multimedia.

	Copyright (C) 1997-2003 Twilight 3D Finland Oy Ltd.
*/
#ifndef PRMATH_RAY_HPP
#define PRMATH_RAY_HPP


#include "vector3.hpp"
#include "line.hpp"
#include "box.hpp"


namespace prmath
{

	template <typename T>
	struct Ray
	{

		// members

		Vector3<T> origin;
		Vector3<T> direction;

		// constructors

		Ray()
		{
		}

		Ray(const Vector3<T>& a, const Vector3<T>& b)
		: origin(a), direction(b-a)
		{
		}

		Ray(const Line<T>& line)
		: origin(line.a), direction(line.b-line.a)
		{
		}

		// ============================================
		// fast box intersection extension
		// based on a paper by:
		// Amy Williams, Steve Barrus, R. Keith Morley and Peter Shirley
		// ============================================

		T invdir[3];
		int sign[3];

		void InitFastIntersectBox()
		{
			T dx = direction.x;
			T dy = direction.y;
			T dz = direction.z;

			// fixme:
			if ( !dx ) dx = static_cast<T>(0.00001);
			if ( !dy ) dy = static_cast<T>(0.00001);
			if ( !dz ) dz = static_cast<T>(0.00001);

			invdir[0] = 1 / dx;
			invdir[1] = 1 / dy;
			invdir[2] = 1 / dz;
			sign[0] = invdir[0] < 0;
			sign[1] = invdir[1] < 0;
			sign[2] = invdir[2] < 0;
		}

		bool FastIntersectBox(const Box<T>& box, T& enter, T& leave) const
		{
			const Vector3<T>* v = &box.vmin;

			T tmin  = (v[sign[0]].x - origin.x) * invdir[0];
			T tymax = (v[1-sign[1]].y - origin.y) * invdir[1];
			if ( tmin > tymax )
				return false;

			T tmax  = (v[1-sign[0]].x - origin.x) * invdir[0];
			T tymin = (v[sign[1]].y - origin.y) * invdir[1];
			if ( tmax < tymin )
				return false;

			if ( tymin > tmin ) tmin = tymin;
			if ( tymax < tmax ) tmax = tymax;

			T tzmin = (v[  sign[2]].z - origin.z) * invdir[2];
			if ( tmax > tzmin )
				return false;

			T tzmax = (v[1-sign[2]].z - origin.z) * invdir[2];
			if ( tmin > tzmax )
				return false;

			if ( tzmin > tmin ) tmin = tzmin;
			if ( tzmax < tmax ) tmax = tzmax;

			enter = tmin;
			leave = tmax;

			return true;
		}
	};

} // namespace prmath


	#ifndef PRMATH_NOTYPENAME

	typedef prmath::Ray<float>		ray3f;
	typedef prmath::Ray<double>		ray3d;

	#endif // PRMATH_NOTYPENAME


#endif
