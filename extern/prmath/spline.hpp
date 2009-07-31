/*
	Twilight Prophecy SDK
	A multi-platform development system for virtual reality and multimedia.

	Copyright (C) 1997-2003 Twilight 3D Finland Oy Ltd.
*/
#ifndef PRMATH_SPLINE_HPP
#define PRMATH_SPLINE_HPP


namespace prmath
{

	template <typename T, typename S>
	inline T GetBezierSplineValue(const T& a, const T& b, const T& c, const T& d, const S& time)
	{
		S time2 = time * time;
		S time3 = time * time2;

		S tp1 = 1 - time;
		S tp2 = tp1 * tp1;
		return a * tp2 * tp1 + b * 3 * tp2 * time + c * 3 * tp1 * time2 + d * time3;
	}

	template <typename T, typename S>
	inline T GetCatmullSplineValue(const T& a, const T& b, const T& c, const T& d, const S& time)
	{
		S time2 = time * time;
		S time3 = time * time2;

		T a5 = a * static_cast<S>(-0.5);
		T d5 = d * static_cast<S>(0.5);
		return static_cast<T>(time3 * (a5 + 1.5*b - 1.5*c + d5) + time2 * (a - 2.5*b + 2*c - d5) + time * (a5 + 0.5*c) + b);
	}

	template <typename T, typename S>
	inline T GetBiCubicSplineValue(const T& a, const T& b, const T& c, const T& d, const S& time)
	{
		S time2 = time * time;
		S time3 = time * time2;

		T s1 = c - a;
		T s2 = d - b;
		return time3 * (2*b - 2*c + s1 + s2) + time2 * (-3*b + 3*c - 2*s1 - s2) + time * s1 + b;
	}

	template <typename T, typename S>
	inline T GetBSplineValue(const T& a, const T& b, const T& c, const T& d, const S& time)
	{
		S time2 = time * time;
		S time3 = time * time2;

		T c3 = c * 3;
		T a3 = a * 3;
		return (time3 * (-a + 3*b - c3 + d) + time2 * (a3 - 6*b + c3) + time * (-a3 + c3) + (a + 4*b + c)) / 6;
	}

} // namespace prmath


#endif
