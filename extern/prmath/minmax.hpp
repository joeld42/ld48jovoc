/*
	Twilight Prophecy SDK
	A multi-platform development system for virtual reality and multimedia.

	Copyright (C) 1997-2003 Twilight 3D Finland Oy Ltd.
*/
#ifndef PRMATH_MINMAX_HPP
#define PRMATH_MINMAX_HPP


	// windows specific fix to prmath
	//
	// <windows.h> defines min and max macros which do not work
	// with standard headers <limits> and <algorithm>
	//
	// usual practise is:
	// #define NOMINMAX
	// #include <windows.h>
	//
	// but so many times PRSDK users do not do this resulting mass of bogus
	// error reports that we decided to step on this. hard.

	#ifdef min
	#undef min
	#endif

	#ifdef max
	#undef max
	#endif


	// there should be std::min() and std::max() macros in <algorithm> header
	// however, VC6 has not got around to fixin this in past 5 years so we aren't holding
	// our breath and take some measures to help ourselves
	//
	// obviously now the std::min() and std::max() are ALWAYS available regardless of including
	// <algorithm> or not, but so what? better than nothing..
	// p.s. we don't want to rely on certain include guards to be present so we do it this way.. =(

	#if defined(_MSC_VER) && (_MSC_VER <= 1200) && !defined(_STLPORT_VERSION)
	namespace std
	{
		template <typename T>
		inline T min(const T& a, const T& b)
		{
			return a < b ? a : b;
		}

		template <typename T>
		inline T max(const T& a, const T& b)
		{
			return a > b ? a : b;
		}
	} // namespace std
	#endif


#endif
