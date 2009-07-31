/*
	Twilight Prophecy SDK
	A multi-platform development system for virtual reality and multimedia.

	Copyright (C) 1997-2003 Twilight 3D Finland Oy Ltd.
*/
/*
	Abstract:
	Meta Expression Template based specialization of prmath::Vector3<float>
	
	Reference material:
	Jim Blinn's Corner: Notation, Notation, Notation    ISBN 1-55860-860-5
	Tomas Arce: Faster Vector Math Using Templates      http://www.flipcode.com/tutorials/tut_fastmath.shtml
*/
#ifndef PRMATH_VECTOR3FLOAT_HPP
#define PRMATH_VECTOR3FLOAT_HPP


#ifndef PRMATH_VECTOR3_HPP
#error prmath: "do not include this header yourself!"
#endif


#include "metavector.hpp"
#ifdef PRMATH_METAVECTOR_ENABLE


namespace prmath
{

	// expression

	template <typename E>
	class MetaExpV3F
	{
		E exp;
		public:
    
		metainline MetaExpV3F(const E& e)
		: exp(e)
		{
		}

		metainline float operator [] (const int index) const
		{
			return exp[index];
		}

		metainline MetaExpV3F operator + () const
		{
			return *this;
		}

		metainline const MetaExpV3F< MetaNEG<float,E> > operator - () const
		{
			return MetaExpV3F< MetaNEG<float,E> >(exp);
		}
	};

	template <>
	class MetaExpV3F<float>
	{
		float exp;
		public:
    
		metainline MetaExpV3F(const float& e)
		: exp(e)
		{
		}

		metainline float operator [] (const int index) const
		{
			return exp;
		}
	};

	// vector3<float>

	template <>
	struct Vector3<float> : BaseVector<float,3>
	{

		// members

		union { float x,r; };
		union { float y,g; };
		union { float z,b; };

		// constructors

		metainline Vector3()
		{
		}

		metainline Vector3(float sx, float sy, float sz)
		: x(sx), y(sy), z(sz)
		{
		}

		metainline Vector3(const float v[])
		: x(v[0]), y(v[1]), z(v[2])
		{
		}

		metainline Vector3(const Vector3& v)
		: x(v.x), y(v.y), z(v.z)
		{
		}

		metainline Vector3(float v)
		: x(v), y(v), z(v)
		{
		}

		template <typename E>
		metainline Vector3(const MetaExpV3F<E>& exp)
		{
			x = exp[0];
			y = exp[1];
			z = exp[2];
		}

		// operators

		template <typename E>
		metainline Vector3<float>& operator += (const MetaExpV3F<E>& exp)
		{
			x += exp[0];
			y += exp[1];
			z += exp[2];
			return *this;
		}

		template <typename E>
		metainline Vector3<float>& operator -= (const MetaExpV3F<E>& exp)
		{
			x -= exp[0];
			y -= exp[1];
			z -= exp[2];
			return *this;
		}

		template <typename E>
		metainline Vector3<float>& operator *= (const MetaExpV3F<E>& exp)
		{
			x *= exp[0];
			y *= exp[1];
			z *= exp[2];
			return *this;
		}

		template <typename E>
		metainline Vector3<float>& operator = (const MetaExpV3F<E>& exp)
		{
			x = exp[0];
			y = exp[1];
			z = exp[2];
			return *this;
		}

		metainline Vector3 operator + () const
		{
			return *this;
		}

		metainline Vector3 operator - () const
		{
			return Vector3(-x,-y,-z);
		}

		metainline Vector3& operator += (const Vector3& v)
		{
			x += v.x;
			y += v.y;
			z += v.z;
			return *this;
		}

		metainline Vector3& operator -= (const Vector3& v)
		{
			x -= v.x;
			y -= v.y;
			z -= v.z;
			return *this;
		}

		metainline Vector3& operator *= (const Vector3& v)
		{
			x *= v.x;
			y *= v.y;
			z *= v.z;
			return *this;
		}

		metainline Vector3& operator = (const Vector3& v)
		{
			x = v.x;
			y = v.y;
			z = v.z;
			return *this;
		}

		metainline Vector3& operator = (float v)
		{
			x = v;
			y = v;
			z = v;
			return *this;
		}

		metainline Vector3& operator *= (float s)
		{
			x *= s;
			y *= s;
			z *= s;
			return *this;
		}

		metainline Vector3& operator /= (float s)
		{
			assert( s != 0 );
			float is = 1 / s;
			x *= is;
			y *= is;
			z *= is;
			return *this;
		}

		// methods

		metainline void Normalize()
		{
			float s = x*x + y*y + z*z;
			if ( s != 0 )
			{
				s = 1 / static_cast<float>(sqrt(s));
				x *= s;
				y *= s;
				z *= s;
			}
		}

		metainline void SetVector(float sx, float sy, float sz)
		{
			x = sx;
			y = sy;
			z = sz;
		}

		metainline void SetVector(float v)
		{
			x = v;
			y = v;
			z = v;
		}
	};

	// meta operators

	// exp + exp
	template <typename A, typename B>
	metainline const MetaExpV3F< MetaADD< float,MetaExpV3F<A>,MetaExpV3F<B> > >
	operator + (const MetaExpV3F<A>& a, const MetaExpV3F<B>& b)
	{
		typedef MetaADD< float,MetaExpV3F<A>,MetaExpV3F<B> > exp;
		return MetaExpV3F<exp>(exp(a,b));
	}

	// exp + vec3f
	template <typename A>
	metainline const MetaExpV3F< MetaADD< float,MetaExpV3F<A>,MetaExpV3F< Vector3<float> > > >
	operator + (const MetaExpV3F<A>& a, const Vector3<float>& b)
	{
		typedef MetaADD< float,MetaExpV3F<A>,MetaExpV3F< Vector3<float> > > exp;
		return MetaExpV3F<exp>(exp(a,b));
	}

	// vec3f + exp
	template <typename B>
	metainline const MetaExpV3F< MetaADD< float,MetaExpV3F< Vector3<float> >,MetaExpV3F<B> > >
	operator + (const Vector3<float>& a, const MetaExpV3F<B>& b)
	{
		typedef MetaADD< float,MetaExpV3F< Vector3<float> >,MetaExpV3F<B> > exp;
		return MetaExpV3F<exp>(exp(a,b));
	}

	// vec3f + vec3f
	metainline const MetaExpV3F< MetaADD< float,MetaExpV3F< Vector3<float> >,MetaExpV3F< Vector3<float> > > >
	operator + (const Vector3<float>& a, const Vector3<float>& b)
	{
		typedef MetaADD< float,MetaExpV3F< Vector3<float> >,MetaExpV3F< Vector3<float> > > exp;
		return MetaExpV3F<exp>(exp(a,b));
	}

	// exp - exp
	template <typename A, typename B>
	metainline const MetaExpV3F< MetaSUB< float,MetaExpV3F<A>,MetaExpV3F<B> > >
	operator - (const MetaExpV3F<A>& a, const MetaExpV3F<B>& b)
	{
		typedef MetaSUB< float,MetaExpV3F<A>,MetaExpV3F<B> > exp;
		return MetaExpV3F<exp>(exp(a,b));
	}

	// exp - vec3f
	template <typename A>
	metainline const MetaExpV3F< MetaSUB< float,MetaExpV3F<A>,MetaExpV3F< Vector3<float> > > >
	operator - (const MetaExpV3F<A>& a, const Vector3<float>& b)
	{
		typedef MetaSUB< float,MetaExpV3F<A>,MetaExpV3F< Vector3<float> > > exp;
		return MetaExpV3F<exp>(exp(a,b));
	}

	// vec3f - exp
	template <typename B>
	metainline const MetaExpV3F< MetaSUB< float,MetaExpV3F< Vector3<float> >,MetaExpV3F<B> > >
	operator - (const Vector3<float>& a, const MetaExpV3F<B>& b)
	{
		typedef MetaSUB< float,MetaExpV3F< Vector3<float> >,MetaExpV3F<B> > exp;
		return MetaExpV3F<exp>(exp(a,b));
	}

	// vec3f - vec3f
	metainline const MetaExpV3F< MetaSUB< float,MetaExpV3F< Vector3<float> >,MetaExpV3F< Vector3<float> > > >
	operator - (const Vector3<float>& a, const Vector3<float>& b)
	{
		typedef MetaSUB< float,MetaExpV3F< Vector3<float> >,MetaExpV3F< Vector3<float> > > exp;
		return MetaExpV3F<exp>(exp(a,b));
	}

	// exp * exp
	template <typename A, typename B>
	metainline const MetaExpV3F< MetaMUL< float,MetaExpV3F<A>,MetaExpV3F<B> > >
	operator * (const MetaExpV3F<A>& a, const MetaExpV3F<B>& b)
	{
		typedef MetaMUL< float,MetaExpV3F<A>,MetaExpV3F<B> > exp;
		return MetaExpV3F<exp>(exp(a,b));
	}

	// exp * vec3f
	template <typename A>
	metainline const MetaExpV3F< MetaMUL< float,MetaExpV3F<A>,MetaExpV3F< Vector3<float> > > >
	operator * (const MetaExpV3F<A>& a, const Vector3<float>& b)
	{
		typedef MetaMUL< float,MetaExpV3F<A>,MetaExpV3F< Vector3<float> > > exp;
		return MetaExpV3F<exp>(exp(a,b));
	}

	// exp * float
	template <typename A>
	metainline const MetaExpV3F< MetaMUL< float,MetaExpV3F<A>,MetaExpV3F<float> > >
	operator * (const MetaExpV3F<A>& a, const float& b)
	{
		typedef MetaMUL< float,MetaExpV3F<A>,MetaExpV3F<float> > exp;
		return MetaExpV3F<exp>(exp(a,b));
	}

	// vec3f * exp
	template <typename B>
	metainline const MetaExpV3F< MetaMUL< float,MetaExpV3F< Vector3<float> >,MetaExpV3F<B> > >
	operator * (const Vector3<float>& a, const MetaExpV3F<B>& b)
	{
		typedef MetaMUL< float,MetaExpV3F< Vector3<float> >,MetaExpV3F<B> > exp;
		return MetaExpV3F<exp>(exp(a,b));
	}

	// vec3f * vec3f
	metainline const MetaExpV3F< MetaMUL< float,MetaExpV3F< Vector3<float> >,MetaExpV3F< Vector3<float> > > >
	operator * (const Vector3<float>& a, const Vector3<float>& b)
	{
		typedef MetaMUL< float,MetaExpV3F< Vector3<float> >,MetaExpV3F< Vector3<float> > > exp;
		return MetaExpV3F<exp>(exp(a,b));
	}

	// vec3f * float
	metainline const MetaExpV3F< MetaMUL< float,MetaExpV3F< Vector3<float> >,MetaExpV3F<float> > >
	operator * (const Vector3<float>& a, const float& b)
	{
		typedef MetaMUL< float,MetaExpV3F< Vector3<float> >,MetaExpV3F<float> > exp;
		return MetaExpV3F<exp>(exp(a,b));
	}

	// float * exp
	template <typename B>
	metainline const MetaExpV3F< MetaMUL< float,MetaExpV3F<float>,MetaExpV3F<B> > >
	operator * (const float& a, const MetaExpV3F<B>& b)
	{
		typedef MetaMUL< float,MetaExpV3F<float>,MetaExpV3F<B> > exp;
		return MetaExpV3F<exp>(exp(a,b));
	}

	// float * vec3f
	metainline const MetaExpV3F< MetaMUL< float,MetaExpV3F<float>,MetaExpV3F< Vector3<float> > > >
	operator * (const float& a, Vector3<float>& b)
	{
		typedef MetaMUL< float,MetaExpV3F<float>,MetaExpV3F< Vector3<float> > > exp;
		return MetaExpV3F<exp>(exp(a,b));
	}

	// exp / exp
	template <typename A, typename B>
	metainline const MetaExpV3F< MetaDIV< float,MetaExpV3F<A>,MetaExpV3F<B> > >
	operator / (const MetaExpV3F<A>& a, const MetaExpV3F<B>& b)
	{
		typedef MetaDIV< float,MetaExpV3F<A>,MetaExpV3F<B> > exp;
		return MetaExpV3F<exp>(exp(a,b));
	}

	// exp / vec3f
	template <typename A>
	metainline const MetaExpV3F< MetaDIV< float,MetaExpV3F<A>,MetaExpV3F< Vector3<float> > > >
	operator / (const MetaExpV3F<A>& a, const Vector3<float>& b)
	{
		typedef MetaDIV< float,MetaExpV3F<A>,MetaExpV3F< Vector3<float> > > exp;
		return MetaExpV3F<exp>(exp(a,b));
	}

	// exp / float
	template <typename A>
	metainline const MetaExpV3F< MetaDIV< float,MetaExpV3F<A>,MetaExpV3F<float> > >
	operator / (const MetaExpV3F<A>& a, const float& b)
	{
		typedef MetaDIV< float,MetaExpV3F<A>,MetaExpV3F<float> > exp;
		return MetaExpV3F<exp>(exp(a,b));
	}

	// vec3f / exp
	template <typename B>
	metainline const MetaExpV3F< MetaDIV< float,MetaExpV3F< Vector3<float> >,MetaExpV3F<B> > >
	operator / (const Vector3<float>& a, const MetaExpV3F<B>& b)
	{
		typedef MetaDIV< float,MetaExpV3F< Vector3<float> >,MetaExpV3F<B> > exp;
		return MetaExpV3F<exp>(exp(a,b));
	}

	// vec3f / vec3f
	metainline const MetaExpV3F< MetaDIV< float,MetaExpV3F< Vector3<float> >,MetaExpV3F< Vector3<float> > > >
	operator / (const Vector3<float>& a, const Vector3<float>& b)
	{
		typedef MetaDIV< float,MetaExpV3F< Vector3<float> >,MetaExpV3F< Vector3<float> > > exp;
		return MetaExpV3F<exp>(exp(a,b));
	}

	// vec3f / float
	metainline const MetaExpV3F< MetaDIV< float,MetaExpV3F< Vector3<float> >,MetaExpV3F<float> > >
	operator / (const Vector3<float>& a, const float& b)
	{
		typedef MetaDIV< float,MetaExpV3F< Vector3<float> >,MetaExpV3F<float> > exp;
		return MetaExpV3F<exp>(exp(a,b));
	}

	// exp == exp
	template <typename A, typename B>
	metainline bool operator == (const MetaExpV3F<A>& a, const MetaExpV3F<B>& b)
	{
		for ( int i=0; i<3; ++i )
		{
			if ( a[i] != b[i] )
				return false;
		}
		return true;
	}

	// vec3f == exp
	template <typename B>
	metainline bool operator == (const Vector3<float>& a, const MetaExpV3F<B>& b)
	{
		for ( int i=0; i<3; ++i )
		{
			if ( a[i] != b[i] )
				return false;
		}
		return true;
	}

	// exp == vec3f
	template <typename A>
	metainline bool operator == (const MetaExpV3F<A>& a, const Vector3<float>& b)
	{
		for ( int i=0; i<3; ++i )
		{
			if ( a[i] != b[i] )
				return false;
		}
		return true;
	}

	// exp != exp
	template <typename A, typename B>
	metainline bool operator != (const MetaExpV3F<A>& a, const MetaExpV3F<B>& b)
	{
		for ( int i=0; i<3; ++i )
		{
			if ( a[i] != b[i] )
				return true;
		}
		return false;
	}

	// vec3f != exp
	template <typename B>
	metainline bool operator != (const Vector3<float>& a, const MetaExpV3F<B>& b)
	{
		for ( int i=0; i<3; ++i )
		{
			if ( a[i] != b[i] )
				return true;
		}
		return false;
	}

	// exp != vec3f
	template <typename A>
	metainline bool operator != (const MetaExpV3F<A>& a, const Vector3<float>& b)
	{
		for ( int i=0; i<3; ++i )
		{
			if ( a[i] != b[i] )
				return true;
		}
		return false;
	}

	// meta functions

	// exp x exp
	template <typename A, typename B>
	metainline const MetaExpV3F< MetaCROSS3< float,MetaExpV3F<A>,MetaExpV3F<B> > >
	CrossProduct(const MetaExpV3F<A>& a, const MetaExpV3F<B>& b)
	{
		typedef MetaCROSS3< float,MetaExpV3F<A>,MetaExpV3F<B> > exp;
		return MetaExpV3F<exp>(exp(a,b));
	}

	// exp x vec3f
	template <typename A>
	metainline const MetaExpV3F< MetaCROSS3< float,MetaExpV3F<A>,MetaExpV3F< Vector3<float> > > >
	CrossProduct(const MetaExpV3F<A>& a, const Vector3<float>& b)
	{
		typedef MetaCROSS3< float,MetaExpV3F<A>,MetaExpV3F< Vector3<float> > > exp;
		return MetaExpV3F<exp>(exp(a,b));
	}

	// vec3f x exp
	template <typename B>
	metainline const MetaExpV3F< MetaCROSS3< float,MetaExpV3F< Vector3<float> >,MetaExpV3F<B> > >
	CrossProduct(const Vector3<float>& a, const MetaExpV3F<B>& b)
	{
		typedef MetaCROSS3< float,MetaExpV3F< Vector3<float> >,MetaExpV3F<B> > exp;
		return MetaExpV3F<exp>(exp(a,b));
	}

	// vec3f x vec3f
/*	metainline const MetaExpV3F< MetaCROSS3< float,MetaExpV3F< Vector3<float> >,MetaExpV3F< Vector3<float> > > >
	CrossProduct(const Vector3<float>& a, const Vector3<float>& b)
	{
		typedef MetaCROSS3< float,MetaExpV3F< Vector3<float> >,MetaExpV3F< Vector3<float> > > exp;
		return MetaExpV3F<exp>(exp(a,b));
	}*/
	metainline Vector3<float>
	CrossProduct(const Vector3<float>& a, const Vector3<float>& b)
	{
		return Vector3<float>(
			a.y * b.z - a.z * b.y,
			a.z * b.x - a.x * b.z,
			a.x * b.y - a.y * b.x);
	}

	// exp . exp
	template <typename A, typename B>
	metainline float DotProduct(const MetaExpV3F<A>& a, const MetaExpV3F<B>& b)
	{
		return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
	}

	// exp . vec3f
	template <typename A>
	metainline float DotProduct(const MetaExpV3F<A>& a, const Vector3<float>& b)
	{
		return a[0] * b.x + a[1] * b.y + a[2] * b.z;
	}

	// vec3f . exp
	template <typename B>
	metainline float DotProduct(const Vector3<float>& a, const MetaExpV3F<B>& b)
	{
		return a.x * b[0] + a.y * b[1] + a.z * b[2];
	}

	// vec3f . vec3f
	metainline float DotProduct(const Vector3<float>& a, const Vector3<float>& b)
	{
		return a.x * b.x + a.y * b.y + a.z * b.z;
	}

	template <typename E>
	metainline float LengthSquared(const MetaExpV3F<E>& exp)
	{
		return DotProduct(exp,exp);
	}

	template <typename E>
	metainline float Length(const MetaExpV3F<E>& exp)
	{
		return static_cast<float>(sqrt(LengthSquared(exp)));
	}

	template <typename E>
	metainline Vector3<float> Normalize(const MetaExpV3F<E>& exp)
	{
		float s = LengthSquared(exp);
		if ( s == 0 )
			return Vector3<float>(0,0,0);
		
		s = 1 / static_cast<float>(sqrt(s));
		return Vector3<float>(exp * s);
	}

	template <typename A, typename B>
	metainline Vector3<float> Reflect(const MetaExpV3F<A>& v, const MetaExpV3F<B>& normal)
	{
		float s = DotProduct(v,normal) * 2;
		return Vector3<float>(v + normal * s);
	}

	template <typename B>
	metainline Vector3<float> Reflect(const Vector3<float>& v, const MetaExpV3F<B>& normal)
	{
		float s = DotProduct(v,normal) * 2;
		return Vector3<float>(v + normal * s);
	}

	template <typename A>
	metainline Vector3<float> Reflect(const MetaExpV3F<A>& v, const Vector3<float>& normal)
	{
		float s = DotProduct(v,normal) * 2;
		return Vector3<float>(v + normal * s);
	}

	template <typename A, typename B>
	metainline Vector3<float> Lerp(const MetaExpV3F<A>& a, const MetaExpV3F<B>& b, float time)
	{
		return Vector3<float>(a + (b - a) * time);
	}

	template <typename B>
	metainline Vector3<float> Lerp(const Vector3<float>& a, const MetaExpV3F<B>& b, float time)
	{
		return Vector3<float>(a + (b - a) * time);
	}

	template <typename A>
	metainline Vector3<float> Lerp(const MetaExpV3F<A>& a, const Vector3<float>& b, float time)
	{
		return Vector3<float>(a + (b - a) * time);
	}

} // namespace prmath


#undef metainline
#undef PRMATH_METAVECTOR_ENABLE
#endif


#endif
