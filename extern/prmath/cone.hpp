/*
	Twilight Prophecy SDK
	A multi-platform development system for virtual reality and multimedia.

	Copyright (C) 1997-2003 Twilight 3D Finland Oy Ltd.
*/
#ifndef PRMATH_CONE_HPP
#define PRMATH_CONE_HPP


#include "vector3.hpp"


namespace prmath
{

	template <typename T>	
	struct Cone
	{

		// members

		Vector3<T>	origin;
		Vector3<T>	target;
		T			angle;

		// constructors

		Cone()
		{
		}

		Cone(const Vector3<T>& o, const Vector3<T>& t, const T& a)
		: origin(o), target(t), angle(a)
		{
		}
	};

} // namespace prmath


	#ifndef PRMATH_NOTYPENAME

	typedef prmath::Cone<float>		cone3f;
	typedef prmath::Cone<double>	cone3d;

	#endif // PRMATH_NOTYPENAME


#endif
