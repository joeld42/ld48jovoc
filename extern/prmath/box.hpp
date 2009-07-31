/*
	Twilight Prophecy SDK
	A multi-platform development system for virtual reality and multimedia.

	Copyright (C) 1997-2003 Twilight 3D Finland Oy Ltd.
*/
#ifndef PRMATH_BOX_HPP
#define PRMATH_BOX_HPP


#include "minmax.hpp"
#include <limits>
#include "vector3.hpp"
#include "matrix4x4.hpp"


namespace prmath
{

	template <typename T>
	struct Box
	{

		// members

		Vector3<T> vmin;
		Vector3<T> vmax;

		// constructors

		Box()
		: vmin( std::numeric_limits<T>::max(), std::numeric_limits<T>::max(), std::numeric_limits<T>::max()),
		  vmax(-std::numeric_limits<T>::max(),-std::numeric_limits<T>::max(),-std::numeric_limits<T>::max())
		{
		}

		Box(const Vector3<T>& umin, const Vector3<T>& umax)
		: vmin(umin), vmax(umax)
		{
		}

		Box(const Box& a, const Box& b)
		{
			vmin.x = a.vmin.x < b.vmin.x ? a.vmin.x : b.vmin.x;
			vmin.y = a.vmin.y < b.vmin.y ? a.vmin.y : b.vmin.y;
			vmin.z = a.vmin.z < b.vmin.z ? a.vmin.z : b.vmin.z;
			vmax.x = a.vmax.x > b.vmax.x ? a.vmax.x : b.vmax.x;
			vmax.y = a.vmax.y > b.vmax.y ? a.vmax.y : b.vmax.y;
			vmax.z = a.vmax.z > b.vmax.z ? a.vmax.z : b.vmax.z;
		}

		// methods

		void UpdateBox(const Vector3<T>& umin, const Vector3<T>& umax)
		{
			if ( umin.x < vmin.x) vmin.x = umin.x;
			if ( umin.y < vmin.y) vmin.y = umin.y;
			if ( umin.z < vmin.z) vmin.z = umin.z;
			if ( umax.x > vmax.x) vmax.x = umax.x;
			if ( umax.y > vmax.y) vmax.y = umax.y;
			if ( umax.z > vmax.z) vmax.z = umax.z;
		}

		void UpdateBox(const Vector3<T>& v)
		{
			if ( v.x < vmin.x ) vmin.x = v.x; if ( v.x > vmax.x ) vmax.x = v.x;
			if ( v.y < vmin.y ) vmin.y = v.y; if ( v.y > vmax.y ) vmax.y = v.y;
			if ( v.z < vmin.z ) vmin.z = v.z; if ( v.z > vmax.z ) vmax.z = v.z;
		}

		Vector3<T> GetCenter() const
		{
			return (vmin + vmax) * static_cast<T>(0.5);
		}

		Vector3<T> GetSize() const
		{
			return vmax - vmin;
		}

		bool IsInside(const Vector3<T>& point) const
		{
			if ( point.x < vmin.x || point.y < vmin.y || point.z < vmin.z ) return false;
			if ( point.x > vmax.x || point.y > vmax.y || point.z > vmax.z ) return false;
			return true;
		}

		void GetVertexArray8(Vector3<T> vertex[]) const
		{
			vertex[0] = Vector3<T>(vmin.x,vmin.y,vmin.z);
			vertex[1] = Vector3<T>(vmax.x,vmin.y,vmin.z);
			vertex[2] = Vector3<T>(vmin.x,vmax.y,vmin.z);
			vertex[3] = Vector3<T>(vmax.x,vmax.y,vmin.z);
			vertex[4] = Vector3<T>(vmin.x,vmin.y,vmax.z);
			vertex[5] = Vector3<T>(vmax.x,vmin.y,vmax.z);
			vertex[6] = Vector3<T>(vmin.x,vmax.y,vmax.z);
			vertex[7] = Vector3<T>(vmax.x,vmax.y,vmax.z);
		}

		void GetVertexArray8(Vector3<T> vertex[], const Matrix4x4<T>& transform) const
		{
			vertex[0] = Vector3<T>(vmin.x,vmin.y,vmin.z) * transform;
			vertex[1] = Vector3<T>(vmax.x,vmin.y,vmin.z) * transform;
			vertex[2] = Vector3<T>(vmin.x,vmax.y,vmin.z) * transform;
			vertex[3] = Vector3<T>(vmax.x,vmax.y,vmin.z) * transform;
			vertex[4] = Vector3<T>(vmin.x,vmin.y,vmax.z) * transform;
			vertex[5] = Vector3<T>(vmax.x,vmin.y,vmax.z) * transform;
			vertex[6] = Vector3<T>(vmin.x,vmax.y,vmax.z) * transform;
			vertex[7] = Vector3<T>(vmax.x,vmax.y,vmax.z) * transform;
		}
	};

} // namespace prmath


	#ifndef PRMATH_NOTYPENAME

	typedef prmath::Box<float>		box3f;
	typedef prmath::Box<double>		box3d;

	#endif // PRMATH_NOTYPENAME


#endif
