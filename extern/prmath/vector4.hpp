/*
	Twilight Prophecy SDK
	A multi-platform development system for virtual reality and multimedia.

	Copyright (C) 1997-2003 Twilight 3D Finland Oy Ltd.
*/
#ifndef PRMATH_VECTOR4_HPP
#define PRMATH_VECTOR4_HPP


#include <cassert>
#include <cmath>
#include "basevector.hpp"


namespace prmath
{

	template <typename T>
	struct Vector4 : BaseVector<T,4>
	{

		// members

		union {	T x,r; };
		union {	T y,g; };
		union {	T z,b; };
		union {	T w,a; };

		// constructors

		Vector4()
		{
		}

		Vector4(const T& vx, const T& vy, const T& vz, const T& vw)
		: x(vx), y(vy), z(vz), w(vw)
		{
		}

		Vector4(const T v[])
		: x(v[0]), y(v[1]), z(v[2]), w(v[3])
		{
		}

		Vector4(const Vector4& v)
		: x(v.x), y(v.y), z(v.z), w(v.w)
		{
		}

		Vector4(const T& v)
		: x(v), y(v), z(v), w(v)
		{
		}

		// operators

		Vector4 operator + () const
		{
			return *this;
		}

		Vector4 operator - () const
		{
			return Vector4(-x, -y, -z, -w);
		}

		Vector4 operator + (const Vector4& v) const
		{
			return Vector4(x + v.x, y + v.y, z + v.z, w + v.w);
		}

		Vector4 operator - (const Vector4& v) const
		{
			return Vector4(x - v.x, y - v.y, z - v.z, w - v.w);
		}

		Vector4 operator * (const Vector4& v) const
		{
			return Vector4(x * v.x, y * v.y, z * v.z, w * v.w);
		}

		Vector4 operator * (const T& s) const
		{
			return Vector4(x * s, y * s, z * s, w * s);
		}

		Vector4 operator / (const T& s) const
		{
			assert( s != 0 );
			T is = 1 / s;
			return Vector4(x * is, y * is, z * is, w * is);
		}

		Vector4& operator += (const Vector4& v)
		{
			x += v.x;
			y += v.y;
			z += v.z;
			w += v.w;
			return *this;
		}

		Vector4& operator -= (const Vector4& v)
		{
			x -= v.x;
			y -= v.y;
			z -= v.z;
			w -= v.w;
			return *this;
		}

		Vector4& operator *= (const Vector4& v)
		{
			x *= v.x;
			y *= v.y;
			z *= v.z;
			w *= v.w;
			return *this;
		}

		Vector4& operator *= (const T& s)
		{
			x *= s;
			y *= s;
			z *= s;
			w *= s;
			return *this;
		}

		Vector4& operator /= (const T& s)
		{
			assert( s != 0 );
			T is = 1 / s;
			x *= is;
			y *= is;
			z *= is;
			w *= is;
			return *this;
		}

		Vector4& operator = (const Vector4& v)
		{
			x = v.x;
			y = v.y;
			z = v.z;
			w = v.w;
			return *this;
		}

		Vector4& operator = (const T& v)
		{
			x = v;
			y = v;
			z = v;
			w = v;
			return *this;
		}

		// methods

		void SetVector(const T& vx, const T& vy, const T& vz, const T& vw)
		{
			x = vx;
			y = vy;
			z = vz;
			w = vw;
		}

		void SetVector(const T& v)
		{
			x = v;
			y = v;
			z = v;
			w = v;
		}
	};

	// inline functions

	template <typename T>
	inline T DotProduct(const Vector4<T>& a, const Vector4<T>& b)
	{
		return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
	}

	template <typename T>
	inline Vector4<T> Lerp(const Vector4<T>& a, const Vector4<T>& b, const T& time)
	{
		return Vector4<T>(
			a.x + (b.x - a.x) * time,
			a.y + (b.y - a.y) * time,
			a.z + (b.z - a.z) * time,
			a.w + (b.w - a.w) * time);
	}

	template <typename T>
	inline Vector4<T> operator * (const T& s, const Vector4<T>& v)
	{
		return Vector4<T>(v.x * s, v.y * s, v.z * s, v.w * s);
	}

} // namespace prmath


	#include "vector4float.hpp"


	#ifndef PRMATH_NOTYPENAME

	typedef prmath::Vector4<float>	float4;
	typedef prmath::Vector4<float>	point4f;
	typedef prmath::Vector4<float>	color4f;
	typedef prmath::Vector4<float>	vec4f;

	typedef prmath::Vector4<double>	double4;
	typedef prmath::Vector4<double>	point4d;
	typedef prmath::Vector4<double>	color4d;
	typedef prmath::Vector4<double>	vec4d;

	#endif // PRMATH_NOTYPENAME


#endif
