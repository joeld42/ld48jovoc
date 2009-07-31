/*
	Twilight Prophecy SDK
	A multi-platform development system for virtual reality and multimedia.

	Copyright (C) 1997-2003 Twilight 3D Finland Oy Ltd.
*/
#ifndef PRMATH_METAVECTOR_HPP
#define PRMATH_METAVECTOR_HPP


// disable Meta Expression Template support:
//   - in debug build, since it only hurts the performance
//   - for Borland Builder, no proper template inlining
//   - for Intel C++, since it generates better code with default implementation (good RVO in the compiler?)
//     07/28/2003 note: Intel C++ 7.1 seems to generate ~+ 5% faster code with expression templates
//   - for g++ 2.95-10, incorrect evaluation
//   - for g++ 3.2, correct evaluation, but slower than default implementation
#if !defined(_DEBUG) && !defined(__BORLANDC__) && !defined(__INTEL_COMPILER) && !defined(__GNUC__)


#if defined(_MSC_VER) && (_MSC_VER < 1300)

	// Microsoft Visual C++ 6 and older versions will use default code

#else


namespace prmath
{

	// we pass the test above:
	// enable meta expression template code generation
	#ifndef PRMATH_METAVECTOR_ENABLE
	#define PRMATH_METAVECTOR_ENABLE
	#endif


	// help microsoft visual c++ to generate optimal code
	#if (_MSC_VER >= 1000) || defined(__VISUALC__)
	#pragma inline_depth(255)
	#pragma inline_recursion(on)
	#pragma auto_inline(on)
	#define metainline __forceinline
	#endif


	// default action for inline keyword
	#ifndef metainline
	#define metainline inline
	#endif


	// evaluation types

	template <typename S, typename E>
	class MetaNEG
	{
		const E exp;
		public:

		metainline MetaNEG(const E& e)
		: exp(e)
		{
		}

		metainline S operator [] (const int index) const
		{
			return -exp[index];
		}
	};

	template <typename S, typename L, typename R>
	class MetaADD
	{
		const L left;
		const R right;
		public:

		metainline MetaADD(const L& vl, const R& vr)
		: left(vl),right(vr)
		{
		}

		metainline S operator [] (const int index) const
		{
			return left[index] + right[index];
		}
	};

	template <typename S, typename L, typename R>
	class MetaSUB
	{
		const L left;
		const R right;
		public:

		metainline MetaSUB(const L& vl, const R& vr)
		: left(vl),right(vr)
		{
		}

		metainline S operator [] (const int index) const
		{
			return left[index] - right[index];
		}
	};

	template <typename S, typename L, typename R>
	class MetaMUL
	{
		const L left;
		const R right;
		public:

		metainline MetaMUL(const L& vl, const R& vr)
		: left(vl),right(vr)
		{
		}

		metainline S operator [] (const int index) const
		{
			return left[index] * right[index];
		}
	};

	template <typename S, typename L, typename R>
	class MetaDIV
	{
		const L left;
		const R right;
		public:

		metainline MetaDIV(const L& vl, const R& vr)
		: left(vl),right(vr)
		{
		}

		metainline S operator [] (const int index) const
		{
			return left[index] / right[index];
		}
	};

	template <typename S, typename L, typename R>
	class MetaCROSS3
	{
		const L left;
		const R right;
		public:

		metainline MetaCROSS3(const L& vl, const R& vr)
		: left(vl),right(vr)
		{
		}

		metainline S operator [] (const int index) const
		{
			switch ( index )
			{
				case 0: return left[1] * right[2] - left[2] * right[1];
				case 1: return left[2] * right[0] - left[0] * right[2];
				case 2: return left[0] * right[1] - left[1] * right[0];
				default: return 0;
			}
		}
	};

} // namespace prmath


#endif
#endif
#endif
