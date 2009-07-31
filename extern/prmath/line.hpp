/*
	Twilight Prophecy SDK
	A multi-platform development system for virtual reality and multimedia.

	Copyright (C) 1997-2003 Twilight 3D Finland Oy Ltd.
*/
#ifndef PRMATH_LINE_HPP
#define PRMATH_LINE_HPP


#include "vector3.hpp"


namespace prmath
{

	template <typename T>
	struct Line
	{

		// members

		Vector3<T> a;
		Vector3<T> b;

		// constructors

		Line()
		{
		}

		Line(const Vector3<T>& pa, const Vector3<T>& pb)
		: a(pa), b(pb)
		{
		}

		// Copyright 2001, softSurfer (www.softsurfer.com)
		// This code may be freely used and modified for any purpose
		// providing that this copyright notice is included with it.
		// SoftSurfer makes no warranty for this code, and cannot be held
		// liable for any real or imagined damage resulting from its use.
		// Users of this code must verify correctness for their application.

		T PointToLineDistance(const Vector3<T>& p) const
		{
			Vector3<T> v = b - a;
			Vector3<T> w = p - a;

			T c1 = DotProduct(w,v);
			T c2 = DotProduct(v,v);

			T di = c1 / c2;

			Vector3<T> pb = a + di * v;
			return Length(p - pb);
		}

		T PointToLineSegmentDistance(const Vector3<T>& p) const
		{
			Vector3<T> v = b - a;
			Vector3<T> w = p - a;

			T c1 = DotProduct(w,v);
			if ( c1 <= 0 )
				return Length(p - a);

			T c2 = DotProduct(v,v);
			if ( c2 <= c1 )
				return Length(p - b);

			T di = c1 / c2;

			Vector3<T> pb = a + di * v;
			return Length(p - pb);
		}
	};

} // namespace prmath


	#ifndef PRMATH_NOTYPENAME

	typedef prmath::Line<float>		line3f;
	typedef prmath::Line<double>	line3d;

	#endif // PRMATH_NOTYPENAME


#endif
