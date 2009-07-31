/*
	Twilight Prophecy SDK
	A multi-platform development system for virtual reality and multimedia.

	Copyright (C) 1997-2003 Twilight 3D Finland Oy Ltd.
*/
#ifndef PRMATH_EULER_HPP
#define PRMATH_EULER_HPP


namespace prmath
{

	enum EulerOrder
	{
		EULER_XYZ,
		EULER_XZY,
		EULER_YXZ,
		EULER_YZX,
		EULER_ZXY,
		EULER_ZYX
	};

	const float pi = 3.14159265358979323846264338327950288419716939937510f;

	template <typename T>
	inline T DegToRad(const T& angle)
	{
		return static_cast<T>(angle * (pi / 180.0));
	}

	template <typename T>
	inline T RadToDeg(const T& angle)
	{
		return static_cast<T>(angle * (180.0 / pi));
	}
	
} // namespace prmath


#endif
