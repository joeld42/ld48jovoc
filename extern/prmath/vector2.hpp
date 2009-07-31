/*
	Twilight Prophecy SDK
	A multi-platform development system for virtual reality and multimedia.

	Copyright (C) 1997-2003 Twilight 3D Finland Oy Ltd.
*/
#ifndef PRMATH_VECTOR2_HPP
#define PRMATH_VECTOR2_HPP


#include <cassert>
#include <cmath>
#include "basevector.hpp"


namespace prmath
{

	template <typename T>
	struct Vector2 : BaseVector<T,2>
	{

		// members

		union { T x,u,s; };
		union { T y,v,t; };

		// constructors

		Vector2()
		{
		}

		Vector2(const T& vx, const T& vy)
		: x(vx), y(vy)
		{
		}

		Vector2(const T v[])
		: x(v[0]), y(v[1])
		{
		}

		Vector2(const Vector2& v)
		: x(v.x), y(v.y)
		{
		}

		Vector2(const T& v)
		: x(v), y(v)
		{
		}

		// operators

		Vector2 operator + () const
		{
			return *this;
		}

		Vector2 operator - () const
		{
			return Vector2(-x, -y);
		}

		Vector2 operator + (const Vector2& v) const
		{
			return Vector2(x + v.x, y + v.y);
		}

		Vector2 operator - (const Vector2& v) const
		{
			return Vector2(x - v.x, y - v.y);
		}

		Vector2 operator * (const Vector2& v) const
		{
			return Vector2(x * v.x, y * v.y);
		}

		Vector2 operator * (const T& s) const
		{
			return Vector2(x * s, y * s);
		}

		Vector2 operator / (const T& s) const
		{
			assert( s != 0 );
			T is = 1 / s;
			return Vector2(x * is, y * is);
		}

		Vector2& operator += (const Vector2& v)
		{
			x += v.x;
			y += v.y;
			return *this;
		}

		Vector2& operator -= (const Vector2& v)
		{
			x -= v.x;
			y -= v.y;
			return *this;
		}

		Vector2& operator *= (const Vector2& v)
		{
			x *= v.x;
			y *= v.y;
			return *this;
		}

		Vector2& operator *= (const T& s)
		{
			x *= s;
			y *= s;
			return *this;
		}

		Vector2& operator /= (const T& s)
		{
			assert( s != 0 );
			T is = 1 / s;
			x *= is;
			y *= is;
			return *this;
		}

		Vector2& operator = (const Vector2& v)
		{
			x = v.x;
			y = v.y;
			return *this;
		}

		Vector2& operator = (const T& v)
		{
			x = v;
			y = v;
			return *this;
		}

		// methods

		void Normalize()
		{
			T s = x*x + y*y;
			if ( s != 0 )
			{
				s = 1 / static_cast<T>(sqrt(s));
				x *= s;
				y *= s;
			}
		}

		void SetVector(const T& vx, const T& vy)
		{
			x = vx;
			y = vy;
		}

		void SetVector(const T& v)
		{
			x = v;
			y = v;
		}
	};

	// inline functions

	template <typename T>
	inline T LengthSquared(const Vector2<T>& v)
	{
		return v.x*v.x + v.y*v.y;
	}

	template <typename T>
	inline T Length(const Vector2<T>& v)
	{
		return static_cast<T>(sqrt(v.x*v.x + v.y*v.y));
	}

	template <typename T>
	inline T DotProduct(const Vector2<T>& a, const Vector2<T>& b)
	{
		return a.x * b.x + a.y * b.y;
	}

	template <typename T>
	inline Vector2<T> CrossProduct(const Vector2<T>& v)
	{
		return Vector2<T>(-v.y, v.x);
	}

	template <typename T>
	inline Vector2<T> Normalize(const Vector2<T>& v)
	{
		T s = static_cast<T>(sqrt(v.x*v.x + v.y*v.y));
		if ( s != 0 )
			s = 1 / s;

		return Vector2<T>(v.x * s, v.y * s);
	}

	template <typename T>
	inline Vector2<T> RotateZ(const Vector2<T>& v, T angle)
	{
		T cosa = static_cast<T>(cos(angle));
		T sina = static_cast<T>(sin(angle));
		return Vector2<T>(
			cosa * v.x - sina * v.y,
			sina * v.x + cosa * v.y);
	}

	template <typename T>
	inline Vector2<T> Lerp(const Vector2<T>& a, const Vector2<T>& b, const T& time)
	{
		return Vector2<T>(
			a.x + (b.x - a.x) * time,
			a.y + (b.y - a.y) * time);
	}

	template <typename T>
	inline Vector2<T> operator * (const T& s, const Vector2<T>& v)
	{
		return Vector2<T>(v.x * s, v.y * s);
	}

} // namespace prmath


	#ifndef PRMATH_NOTYPENAME

	typedef prmath::Vector2<float>	float2;
	typedef prmath::Vector2<float>	point2f;
	typedef prmath::Vector2<float>	vec2f;

	typedef prmath::Vector2<double>	double2;
	typedef prmath::Vector2<double>	point2d;
	typedef prmath::Vector2<double>	vec2d;

	#endif // PRMATH_NOTYPENAME


#endif
