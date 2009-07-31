/*
	Twilight Prophecy SDK
	A multi-platform development system for virtual reality and multimedia.

	Copyright (C) 1997-2003 Twilight 3D Finland Oy Ltd.
*/
/*
	Abstract:
	Meta Expression Template based specialization of prmath::Vector4<float>
	
	Reference material:
	Jim Blinn's Corner: Notation, Notation, Notation    ISBN 1-55860-860-5
	Tomas Arce: Faster Vector Math Using Templates      http://www.flipcode.com/tutorials/tut_fastmath.shtml
*/
#ifndef PRMATH_VECTOR4FLOAT_HPP
#define PRMATH_VECTOR4FLOAT_HPP


#ifndef PRMATH_VECTOR4_HPP
#error prmath: "do not include this header yourself!"
#endif


#include "metavector.hpp"
#ifdef PRMATH_METAVECTOR_ENABLE


namespace prmath
{

	// expression

	template <typename E>
	class MetaExpV4F
	{
		E exp;
		public:
    
		metainline MetaExpV4F(const E& e)
		: exp(e)
		{
		}

		metainline float operator [] (const int index) const
		{
			return exp[index];
		}

		metainline MetaExpV4F operator + () const
		{
			return *this;
		}

		metainline const MetaExpV4F< MetaNEG<float,E> > operator - () const
		{
			return MetaExpV4F< MetaNEG<float,E> >(exp);
		}
	};

	template <>
	class MetaExpV4F<float>
	{
		float exp;
		public:
    
		metainline MetaExpV4F(const float& e)
		: exp(e)
		{
		}

		metainline float operator [] (const int index) const
		{
			return exp;
		}
	};

	// vector4<float>

	template <>
	struct Vector4<float> : BaseVector<float,4>
	{

		// members

		union {	float x,r; };
		union {	float y,g; };
		union {	float z,b; };
		union {	float w,a; };

		// constructors

		metainline Vector4()
		{
		}

		metainline Vector4(float sx, float sy, float sz, float sw)
		: x(sx), y(sy), z(sz), w(sw)
		{
		}

		metainline Vector4(const float v[])
		: x(v[0]), y(v[1]), z(v[2]), w(v[3])
		{
		}

		metainline Vector4(const Vector4& v)
		: x(v.x), y(v.y), z(v.z), w(v.w)
		{
		}

		metainline Vector4(float v)
		: x(v), y(v), z(v), w(v)
		{
		}

		template <typename E>
		metainline Vector4(const MetaExpV4F<E>& exp)
		{
			x = exp[0];
			y = exp[1];
			z = exp[2];
			w = exp[3];
		}

		// operators

		template <typename E>
		metainline Vector4<float>& operator += (const MetaExpV4F<E>& exp)
		{
			x += exp[0];
			y += exp[1];
			z += exp[2];
			w += exp[3];
			return *this;
		}

		template <typename E>
		metainline Vector4<float>& operator -= (const MetaExpV4F<E>& exp)
		{
			x -= exp[0];
			y -= exp[1];
			z -= exp[2];
			w -= exp[3];
			return *this;
		}

		template <typename E>
		metainline Vector4<float>& operator *= (const MetaExpV4F<E>& exp)
		{
			x *= exp[0];
			y *= exp[1];
			z *= exp[2];
			w *= exp[3];
			return *this;
		}

		template <typename E>
		metainline Vector4<float>& operator = (const MetaExpV4F<E>& exp)
		{
			x = exp[0];
			y = exp[1];
			z = exp[2];
			w = exp[3];
			return *this;
		}

		metainline Vector4 operator + () const
		{
			return *this;
		}

		metainline Vector4 operator - () const
		{
			return Vector4(-x,-y,-z,-w);
		}

		metainline Vector4& operator += (const Vector4& v)
		{
			x += v.x;
			y += v.y;
			z += v.z;
			w += v.w;
			return *this;
		}

		metainline Vector4& operator -= (const Vector4& v)
		{
			x -= v.x;
			y -= v.y;
			z -= v.z;
			w -= v.w;
			return *this;
		}

		metainline Vector4& operator *= (const Vector4& v)
		{
			x *= v.x;
			y *= v.y;
			z *= v.z;
			w *= v.w;
			return *this;
		}

		metainline Vector4& operator = (const Vector4& v)
		{
			x = v.x;
			y = v.y;
			z = v.z;
			w = v.w;
			return *this;
		}

		metainline Vector4& operator = (float v)
		{
			x = v;
			y = v;
			z = v;
			w = v;
			return *this;
		}

		metainline Vector4& operator *= (float s)
		{
			x *= s;
			y *= s;
			z *= s;
			w *= s;
			return *this;
		}

		metainline Vector4& operator /= (float s)
		{
			assert( s != 0 );
			float is = 1 / s;
			x *= is;
			y *= is;
			z *= is;
			w *= is;
			return *this;
		}

		// methods

		void SetVector(float sx, float sy, float sz, float sw)
		{
			x = sx;
			y = sy;
			z = sz;
			w = sw;
		}

		void SetVector(float v)
		{
			x = v;
			y = v;
			z = v;
			w = v;
		}
	};

	// meta operators

	// exp + exp
	template <typename A, typename B>
	metainline const MetaExpV4F< MetaADD< float,MetaExpV4F<A>,MetaExpV4F<B> > >
	operator + (const MetaExpV4F<A>& a, const MetaExpV4F<B>& b)
	{
		typedef MetaADD< float,MetaExpV4F<A>,MetaExpV4F<B> > exp;
		return MetaExpV4F<exp>(exp(a,b));
	}

	// exp + vec4f
	template <typename A>
	metainline const MetaExpV4F< MetaADD< float,MetaExpV4F<A>,MetaExpV4F< Vector4<float> > > >
	operator + (const MetaExpV4F<A>& a, const Vector4<float>& b)
	{
		typedef MetaADD< float,MetaExpV4F<A>,MetaExpV4F< Vector4<float> > > exp;
		return MetaExpV4F<exp>(exp(a,b));
	}

	// vec4f + exp
	template <typename B>
	metainline const MetaExpV4F< MetaADD< float,MetaExpV4F< Vector4<float> >,MetaExpV4F<B> > >
	operator + (const Vector4<float>& a, const MetaExpV4F<B>& b)
	{
		typedef MetaADD< float,MetaExpV4F< Vector4<float> >,MetaExpV4F<B> > exp;
		return MetaExpV4F<exp>(exp(a,b));
	}

	// vec4f + vec4f
	metainline const MetaExpV4F< MetaADD< float,MetaExpV4F< Vector4<float> >,MetaExpV4F< Vector4<float> > > >
	operator + (const Vector4<float>& a, const Vector4<float>& b)
	{
		typedef MetaADD< float,MetaExpV4F< Vector4<float> >,MetaExpV4F< Vector4<float> > > exp;
		return MetaExpV4F<exp>(exp(a,b));
	}

	// exp - exp
	template <typename A, typename B>
	metainline const MetaExpV4F< MetaSUB< float,MetaExpV4F<A>,MetaExpV4F<B> > >
	operator - (const MetaExpV4F<A>& a, const MetaExpV4F<B>& b)
	{
		typedef MetaSUB< float,MetaExpV4F<A>,MetaExpV4F<B> > exp;
		return MetaExpV4F<exp>(exp(a,b));
	}

	// exp - vec4f
	template <typename A>
	metainline const MetaExpV4F< MetaSUB< float,MetaExpV4F<A>,MetaExpV4F< Vector4<float> > > >
	operator - (const MetaExpV4F<A>& a, const Vector4<float>& b)
	{
		typedef MetaSUB< float,MetaExpV4F<A>,MetaExpV4F< Vector4<float> > > exp;
		return MetaExpV4F<exp>(exp(a,b));
	}

	// vec4f - exp
	template <typename B>
	metainline const MetaExpV4F< MetaSUB< float,MetaExpV4F< Vector4<float> >,MetaExpV4F<B> > >
	operator - (const Vector4<float>& a, const MetaExpV4F<B>& b)
	{
		typedef MetaSUB< float,MetaExpV4F< Vector4<float> >,MetaExpV4F<B> > exp;
		return MetaExpV4F<exp>(exp(a,b));
	}

	// vec4f - vec4f
	metainline const MetaExpV4F< MetaSUB< float,MetaExpV4F< Vector4<float> >,MetaExpV4F< Vector4<float> > > >
	operator - (const Vector4<float>& a, const Vector4<float>& b)
	{
		typedef MetaSUB< float,MetaExpV4F< Vector4<float> >,MetaExpV4F< Vector4<float> > > exp;
		return MetaExpV4F<exp>(exp(a,b));
	}

	// exp * exp
	template <typename A, typename B>
	metainline const MetaExpV4F< MetaMUL< float,MetaExpV4F<A>,MetaExpV4F<B> > >
	operator * (const MetaExpV4F<A>& a, const MetaExpV4F<B>& b)
	{
		typedef MetaMUL< float,MetaExpV4F<A>,MetaExpV4F<B> > exp;
		return MetaExpV4F<exp>(exp(a,b));
	}

	// exp * vec4f
	template <typename A>
	metainline const MetaExpV4F< MetaMUL< float,MetaExpV4F<A>,MetaExpV4F< Vector4<float> > > >
	operator * (const MetaExpV4F<A>& a, const Vector4<float>& b)
	{
		typedef MetaMUL< float,MetaExpV4F<A>,MetaExpV4F< Vector4<float> > > exp;
		return MetaExpV4F<exp>(exp(a,b));
	}

	// exp * float
	template <typename A>
	metainline const MetaExpV4F< MetaMUL< float,MetaExpV4F<A>,MetaExpV4F<float> > >
	operator * (const MetaExpV4F<A>& a, const float& b)
	{
		typedef MetaMUL< float,MetaExpV4F<A>,MetaExpV4F<float> > exp;
		return MetaExpV4F<exp>(exp(a,b));
	}

	// vec4f * exp
	template <typename B>
	metainline const MetaExpV4F< MetaMUL< float,MetaExpV4F< Vector4<float> >,MetaExpV4F<B> > >
	operator * (const Vector4<float>& a, const MetaExpV4F<B>& b)
	{
		typedef MetaMUL< float,MetaExpV4F< Vector4<float> >,MetaExpV4F<B> > exp;
		return MetaExpV4F<exp>(exp(a,b));
	}

	// vec4f * vec4f
	metainline const MetaExpV4F< MetaMUL< float,MetaExpV4F< Vector4<float> >,MetaExpV4F< Vector4<float> > > >
	operator * (const Vector4<float>& a, const Vector4<float>& b)
	{
		typedef MetaMUL< float,MetaExpV4F< Vector4<float> >,MetaExpV4F< Vector4<float> > > exp;
		return MetaExpV4F<exp>(exp(a,b));
	}

	// vec4f * float
	metainline const MetaExpV4F< MetaMUL< float,MetaExpV4F< Vector4<float> >,MetaExpV4F<float> > >
	operator * (const Vector4<float>& a, const float& b)
	{
		typedef MetaMUL< float,MetaExpV4F< Vector4<float> >,MetaExpV4F<float> > exp;
		return MetaExpV4F<exp>(exp(a,b));
	}

	// float * exp
	template <typename B>
	metainline const MetaExpV4F< MetaMUL< float,MetaExpV4F<float>,MetaExpV4F<B> > >
	operator * (const float& a, const MetaExpV4F<B>& b)
	{
		typedef MetaMUL< float,MetaExpV4F<float>,MetaExpV4F<B> > exp;
		return MetaExpV4F<exp>(exp(a,b));
	}

	// float * vec4f
	metainline const MetaExpV4F< MetaMUL< float,MetaExpV4F<float>,MetaExpV4F< Vector4<float> > > >
	operator * (const float& a, Vector4<float>& b)
	{
		typedef MetaMUL< float,MetaExpV4F<float>,MetaExpV4F< Vector4<float> > > exp;
		return MetaExpV4F<exp>(exp(a,b));
	}

	// exp / exp
	template <typename A, typename B>
	metainline const MetaExpV4F< MetaDIV< float,MetaExpV4F<A>,MetaExpV4F<B> > >
	operator / (const MetaExpV4F<A>& a, const MetaExpV4F<B>& b)
	{
		typedef MetaDIV< float,MetaExpV4F<A>,MetaExpV4F<B> > exp;
		return MetaExpV4F<exp>(exp(a,b));
	}

	// exp / vec4f
	template <typename A>
	metainline const MetaExpV4F< MetaDIV< float,MetaExpV4F<A>,MetaExpV4F< Vector4<float> > > >
	operator / (const MetaExpV4F<A>& a, const Vector4<float>& b)
	{
		typedef MetaDIV< float,MetaExpV4F<A>,MetaExpV4F< Vector4<float> > > exp;
		return MetaExpV4F<exp>(exp(a,b));
	}

	// exp / float
	template <typename A>
	metainline const MetaExpV4F< MetaDIV< float,MetaExpV4F<A>,MetaExpV4F<float> > >
	operator / (const MetaExpV4F<A>& a, const float& b)
	{
		typedef MetaDIV< float,MetaExpV4F<A>,MetaExpV4F<float> > exp;
		return MetaExpV4F<exp>(exp(a,b));
	}

	// vec4f / exp
	template <typename B>
	metainline const MetaExpV4F< MetaDIV< float,MetaExpV4F< Vector4<float> >,MetaExpV4F<B> > >
	operator / (const Vector4<float>& a, const MetaExpV4F<B>& b)
	{
		typedef MetaDIV< float,MetaExpV4F< Vector4<float> >,MetaExpV4F<B> > exp;
		return MetaExpV4F<exp>(exp(a,b));
	}

	// vec4f / vec4f
	metainline const MetaExpV4F< MetaDIV< float,MetaExpV4F< Vector4<float> >,MetaExpV4F< Vector4<float> > > >
	operator / (const Vector4<float>& a, const Vector4<float>& b)
	{
		typedef MetaDIV< float,MetaExpV4F< Vector4<float> >,MetaExpV4F< Vector4<float> > > exp;
		return MetaExpV4F<exp>(exp(a,b));
	}

	// vec4f / float
	metainline const MetaExpV4F< MetaDIV< float,MetaExpV4F< Vector4<float> >,MetaExpV4F<float> > >
	operator / (const Vector4<float>& a, const float& b)
	{
		typedef MetaDIV< float,MetaExpV4F< Vector4<float> >,MetaExpV4F<float> > exp;
		return MetaExpV4F<exp>(exp(a,b));
	}

	// exp == exp
	template <typename A, typename B>
	metainline bool operator == (const MetaExpV4F<A>& a, const MetaExpV4F<B>& b)
	{
		for ( int i=0; i<4; ++i )
		{
			if ( a[i] != b[i] )
				return false;
		}
		return true;
	}

	// vec4f == exp
	template <typename B>
	metainline bool operator == (const Vector4<float>& a, const MetaExpV4F<B>& b)
	{
		for ( int i=0; i<4; ++i )
		{
			if ( a[i] != b[i] )
				return false;
		}
		return true;
	}

	// exp == vec4f
	template <typename A>
	metainline bool operator == (const MetaExpV4F<A>& a, const Vector4<float>& b)
	{
		for ( int i=0; i<4; ++i )
		{
			if ( a[i] != b[i] )
				return false;
		}
		return true;
	}

	// exp != exp
	template <typename A, typename B>
	metainline bool operator != (const MetaExpV4F<A>& a, const MetaExpV4F<B>& b)
	{
		for ( int i=0; i<4; ++i )
		{
			if ( a[i] != b[i] )
				return true;
		}
		return false;
	}

	// vec4f != exp
	template <typename B>
	metainline bool operator != (const Vector4<float>& a, const MetaExpV4F<B>& b)
	{
		for ( int i=0; i<4; ++i )
		{
			if ( a[i] != b[i] )
				return true;
		}
		return false;
	}

	// exp != vec4f
	template <typename A>
	metainline bool operator != (const MetaExpV4F<A>& a, const Vector4<float>& b)
	{
		for ( int i=0; i<4; ++i )
		{
			if ( a[i] != b[i] )
				return true;
		}
		return false;
	}

	// meta functions

	// exp . exp
	template <typename A, typename B>
	metainline float DotProduct(const MetaExpV4F<A>& a, const MetaExpV4F<B>& b)
	{
		return a[0] * b[0] + a[1] * b[1] + a[2] * b[2] + a[3] * b[3];
	}

	// exp . vec4f
	template <typename A>
	metainline float DotProduct(const MetaExpV4F<A>& a, const Vector4<float>& b)
	{
		return a[0] * b.x + a[1] * b.y + a[2] * b.z + a[3] * b.w;
	}

	// vec4f . exp
	template <typename B>
	metainline float DotProduct(const Vector4<float>& a, const MetaExpV4F<B>& b)
	{
		return a.x * b[0] + a.y * b[1] + a.z * b[2] + a.w * b[3];
	}

	// vec4f . vec4f
	metainline float DotProduct(const Vector4<float>& a, const Vector4<float>& b)
	{
		return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
	}

	template <typename A, typename B>
	metainline Vector4<float> Lerp(const MetaExpV4F<A>& a, const MetaExpV4F<B>& b, float time)
	{
		return Vector4<float>(a + (b - a) * time);
	}

	template <typename B>
	metainline Vector4<float> Lerp(const Vector4<float>& a, const MetaExpV4F<B>& b, float time)
	{
		return Vector4<float>(a + (b - a) * time);
	}

	template <typename A>
	metainline Vector4<float> Lerp(const MetaExpV4F<A>& a, const Vector4<float>& b, float time)
	{
		return Vector4<float>(a + (b - a) * time);
	}

} // namespace prmath


#undef metainline
#undef PRMATH_METAVECTOR_ENABLE
#endif


#endif
