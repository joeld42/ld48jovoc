/*
	Twilight Prophecy SDK
	A multi-platform development system for virtual reality and multimedia.

	Copyright (C) 1997-2003 Twilight 3D Finland Oy Ltd.
*/
#ifndef PRMATH_MATRIX4X4_HPP
#define PRMATH_MATRIX4X4_HPP


#include "minmax.hpp"
#include <cmath>
#include <limits>
#include <algorithm>
#include "vector3.hpp"
#include "vector4.hpp"
#include "euler.hpp"
#include "quaternion.hpp"
#include "plane.hpp"


namespace prmath
{

	template <typename T>
	struct Quaternion;


	template <typename T>
	struct Matrix4x4
	{
		/*
	    The prmath matrix convention:

		; offsets

        [ 0  1  2  3]
        [ 4  5  6  7]
        [ 8  9  0 11]
        [12 13 14 15]

		; scaling (sx,sy,sz)

        [sx -- -- --]
        [-- sy -- --]
        [-- -- sz --]
        [-- -- -- --]
    
    	; translation (tx,ty,tz)

        [-- -- -- --]
        [-- -- -- --]
        [-- -- -- --]
        [tx ty tz --]
    
    	; rotation

        [xx xy xz --]  x-axis (xx,xy,xz)
        [yx yy yz --]  y-axis (yx,yy,yz)
        [zx zy zz --]  z-axis (zx,zy,zz)
        [-- -- -- --]

		; v = a * b

		v[0][0] = a[0][0] * b[0][0] + a[0][1] * b[1][0] + a[0][2] * b[2][0] + a[0][3] * b[3][0];
		v[0][1] = a[0][0] * b[0][1] + a[0][1] * b[1][1] + a[0][2] * b[2][1] + a[0][3] * b[3][1];
		v[0][2] = a[0][0] * b[0][2] + a[0][1] * b[1][2] + a[0][2] * b[2][2] + a[0][3] * b[3][2];
		v[0][3] = a[0][0] * b[0][3] + a[0][1] * b[1][3] + a[0][2] * b[2][3] + a[0][3] * b[3][3];
		v[1][0] = a[1][0] * b[0][0] + a[1][1] * b[1][0] + a[1][2] * b[2][0] + a[1][3] * b[3][0];
		v[1][1] = a[1][0] * b[0][1] + a[1][1] * b[1][1] + a[1][2] * b[2][1] + a[1][3] * b[3][1];
		v[1][2] = a[1][0] * b[0][2] + a[1][1] * b[1][2] + a[1][2] * b[2][2] + a[1][3] * b[3][2];
		v[1][3] = a[1][0] * b[0][3] + a[1][1] * b[1][3] + a[1][2] * b[2][3] + a[1][3] * b[3][3];
		v[2][0] = a[2][0] * b[0][0] + a[2][1] * b[1][0] + a[2][2] * b[2][0] + a[2][3] * b[3][0];
		v[2][1] = a[2][0] * b[0][1] + a[2][1] * b[1][1] + a[2][2] * b[2][1] + a[2][3] * b[3][1];
		v[2][2] = a[2][0] * b[0][2] + a[2][1] * b[1][2] + a[2][2] * b[2][2] + a[2][3] * b[3][2];
		v[2][3] = a[2][0] * b[0][3] + a[2][1] * b[1][3] + a[2][2] * b[2][3] + a[2][3] * b[3][3];
		v[3][0] = a[3][0] * b[0][0] + a[3][1] * b[1][0] + a[3][2] * b[2][0] + a[3][3] * b[3][0];
		v[3][1] = a[3][0] * b[0][1] + a[3][1] * b[1][1] + a[3][2] * b[2][1] + a[3][3] * b[3][1];
		v[3][2] = a[3][0] * b[0][2] + a[3][1] * b[1][2] + a[3][2] * b[2][2] + a[3][3] * b[3][2];
		v[3][3] = a[3][0] * b[0][3] + a[3][1] * b[1][3] + a[3][2] * b[2][3] + a[3][3] * b[3][3];

		*/

    	// members

    	union
    	{
    		T m16[16];
    		T m44[4][4];
    	};

		// constructors

		Matrix4x4()
		{
		}

		Matrix4x4(const T& s)
		{
			m44[0][0] = s; m44[0][1] = 0; m44[0][2] = 0; m44[0][3] = 0;
			m44[1][0] = 0; m44[1][1] = s; m44[1][2] = 0; m44[1][3] = 0;
			m44[2][0] = 0; m44[2][1] = 0; m44[2][2] = s; m44[2][3] = 0;
			m44[3][0] = 0; m44[3][1] = 0; m44[3][2] = 0; m44[3][3] = 1;
		}

		Matrix4x4(const Matrix4x4& u)
		{
			for ( int i=0; i<4; ++i )
			{
				m44[i][0] = u.m44[i][0];
				m44[i][1] = u.m44[i][1];
				m44[i][2] = u.m44[i][2];
				m44[i][3] = u.m44[i][3];
			}
		}

		Matrix4x4(const Quaternion<T>& q)
		{
			*this = q;
		}

		// operators

		Matrix4x4 operator + (const Matrix4x4& u) const
		{
			Matrix4x4 r;
			for ( int i=0; i<4; ++i )
			{
				r.m44[i][0] = m44[i][0] + u.m44[i][0];
				r.m44[i][1] = m44[i][1] + u.m44[i][1];
				r.m44[i][2] = m44[i][2] + u.m44[i][2];
				r.m44[i][3] = m44[i][3] + u.m44[i][3];
			}
			return r;
		}

		Matrix4x4 operator - (const Matrix4x4& u) const
		{
			Matrix4x4 r;
			for ( int i=0; i<4; ++i )
			{
				r.m44[i][0] = m44[i][0] - u.m44[i][0];
				r.m44[i][1] = m44[i][1] - u.m44[i][1];
				r.m44[i][2] = m44[i][2] - u.m44[i][2];
				r.m44[i][3] = m44[i][3] - u.m44[i][3];
			}
			return r;
		}

		Matrix4x4 operator * (const Matrix4x4& u) const
		{
			Matrix4x4 r;
			for ( int i=0; i<4; ++i )
			{
				const T* v = m16 + i * 4;
				r.m44[i][0] = v[0]*u.m44[0][0] + v[1]*u.m44[1][0] + v[2]*u.m44[2][0] + v[3]*u.m44[3][0];
				r.m44[i][1] = v[0]*u.m44[0][1] + v[1]*u.m44[1][1] + v[2]*u.m44[2][1] + v[3]*u.m44[3][1];
				r.m44[i][2] = v[0]*u.m44[0][2] + v[1]*u.m44[1][2] + v[2]*u.m44[2][2] + v[3]*u.m44[3][2];
				r.m44[i][3] = v[0]*u.m44[0][3] + v[1]*u.m44[1][3] + v[2]*u.m44[2][3] + v[3]*u.m44[3][3];
			}
			return r;
		}

		Matrix4x4& operator += (const Matrix4x4& u)
		{
			for ( int i=0; i<4; ++i )
			{
				m44[i][0] += u.m44[i][0];
				m44[i][1] += u.m44[i][1];
				m44[i][2] += u.m44[i][2];
				m44[i][3] += u.m44[i][3];
			}
			return *this;
		}

		Matrix4x4& operator -= (const Matrix4x4& u)
		{
			for ( int i=0; i<4; ++i )
			{
				m44[i][0] -= u.m44[i][0];
				m44[i][1] -= u.m44[i][1];
				m44[i][2] -= u.m44[i][2];
				m44[i][3] -= u.m44[i][3];
			}
			return *this;
		}

		Matrix4x4& operator *= (const Matrix4x4& u)
		{
			for ( int i=0; i<4; ++i )
			{
				T v[4];
				v[0] = m44[i][0];
				v[1] = m44[i][1];
				v[2] = m44[i][2];
				v[3] = m44[i][3];
				m44[i][0] = v[0]*u.m44[0][0] + v[1]*u.m44[1][0] + v[2]*u.m44[2][0] + v[3]*u.m44[3][0];
				m44[i][1] = v[0]*u.m44[0][1] + v[1]*u.m44[1][1] + v[2]*u.m44[2][1] + v[3]*u.m44[3][1];
				m44[i][2] = v[0]*u.m44[0][2] + v[1]*u.m44[1][2] + v[2]*u.m44[2][2] + v[3]*u.m44[3][2];
				m44[i][3] = v[0]*u.m44[0][3] + v[1]*u.m44[1][3] + v[2]*u.m44[2][3] + v[3]*u.m44[3][3];
			}
			return *this;
		}

		Matrix4x4& operator *= (const T& s)
		{
			for ( int i=0; i<16; ++i )
			{
				m16[i] *= s;
			}
			return *this;
		}

		void operator = (const T& s)
		{
			m44[0][0] = s; m44[0][1] = 0; m44[0][2] = 0; m44[0][3] = 0;
			m44[1][0] = 0; m44[1][1] = s; m44[1][2] = 0; m44[1][3] = 0;
			m44[2][0] = 0; m44[2][1] = 0; m44[2][2] = s; m44[2][3] = 0;
			m44[3][0] = 0; m44[3][1] = 0; m44[3][2] = 0; m44[3][3] = 1;
		}

		void operator = (const Matrix4x4& u)
		{
			for ( int i=0; i<4; ++i )
			{
				m44[i][0] = u.m44[i][0];
				m44[i][1] = u.m44[i][1];
				m44[i][2] = u.m44[i][2];
				m44[i][3] = u.m44[i][3];
			}
		}

		void operator = (const Quaternion<T>& q)
		{
			T sx = q.x * 2;
			T sy = q.y * 2;
			T sz = q.z * 2;
			T wx = q.w * sx;
			T wy = q.w * sy;
			T wz = q.w * sz;
			T xx = q.x * sx;
			T xy = q.x * sy;
			T xz = q.x * sz;
			T yy = q.y * sy;
			T yz = q.y * sz;
			T zz = q.z * sz;
	
			m44[0][0] = 1 - yy - zz;
			m44[0][1] = xy + wz;
			m44[0][2] = xz - wy;
			m44[0][3] = 0;
			m44[1][0] = xy - wz;
			m44[1][1] = 1 - xx - zz;
			m44[1][2] = yz + wx;
			m44[1][3] = 0;
			m44[2][0] = xz + wy;
			m44[2][1] = yz - wx;
			m44[2][2] = 1 - xx - yy;
			m44[2][3] = 0;
			m44[3][0] = 0;
			m44[3][1] = 0;
			m44[3][2] = 0;
			m44[3][3] = 1;
		}

		Vector4<T>& operator [] (int index)
		{
			assert( index>=0 && index<=3 );
			return reinterpret_cast< Vector4<T>* >(this)[index];
		}

		const Vector4<T>& operator [] (int index) const
		{
			assert( index>=0 && index<=3 );
			return reinterpret_cast< const Vector4<T>* >(this)[index];
		}

		// methods

		void Identity()
		{
			m44[0][0] = 1; m44[0][1] = 0; m44[0][2] = 0; m44[0][3] = 0;
			m44[1][0] = 0; m44[1][1] = 1; m44[1][2] = 0; m44[1][3] = 0;
			m44[2][0] = 0; m44[2][1] = 0; m44[2][2] = 1; m44[2][3] = 0;
			m44[3][0] = 0; m44[3][1] = 0; m44[3][2] = 0; m44[3][3] = 1;
		}

		void Scale(const T& x, const T& y, const T& z)
		{
			m44[0][0] = x; m44[0][1] = 0; m44[0][2] = 0; m44[0][3] = 0;
			m44[1][0] = 0; m44[1][1] = y; m44[1][2] = 0; m44[1][3] = 0;
			m44[2][0] = 0; m44[2][1] = 0; m44[2][2] = z; m44[2][3] = 0;
			m44[3][0] = 0; m44[3][1] = 0; m44[3][2] = 0; m44[3][3] = 1;
		}

		void Scale(const Vector3<T>& scale)
		{
			Scale(scale.x,scale.y,scale.z);
		}

		void Translate(const T& x, const T& y, const T& z)
		{
			m44[0][0] = 1; m44[0][1] = 0; m44[0][2] = 0; m44[0][3] = 0;
			m44[1][0] = 0; m44[1][1] = 1; m44[1][2] = 0; m44[1][3] = 0;
			m44[2][0] = 0; m44[2][1] = 0; m44[2][2] = 1; m44[2][3] = 0;
			m44[3][0] = x; m44[3][1] = y; m44[3][2] = z; m44[3][3] = 1;
		}

		void Translate(const Vector3<T>& translate)
		{
			Translate(translate.x,translate.y,translate.z);
		}

		void RotateX(const T& angle)
		{
			T s = static_cast<T>(sin(angle));
			T c = static_cast<T>(cos(angle));
			
			m44[0][0] = 1; m44[0][1] = 0; m44[0][2] = 0; m44[0][3] = 0;
			m44[1][0] = 0; m44[1][1] = c; m44[1][2] = s; m44[1][3] = 0;
			m44[2][0] = 0; m44[2][1] =-s; m44[2][2] = c; m44[2][3] = 0;
			m44[3][0] = 0; m44[3][1] = 0; m44[3][2] = 0; m44[3][3] = 1;
		}

		void RotateY(const T& angle)
		{
			T s = static_cast<T>(sin(angle));
			T c = static_cast<T>(cos(angle));
			
			m44[0][0] = c; m44[0][1] = 0; m44[0][2] =-s; m44[0][3] = 0;
			m44[1][0] = 0; m44[1][1] = 1; m44[1][2] = 0; m44[1][3] = 0;
			m44[2][0] = s; m44[2][1] = 0; m44[2][2] = c; m44[2][3] = 0;
			m44[3][0] = 0; m44[3][1] = 0; m44[3][2] = 0; m44[3][3] = 1;
		}

		void RotateZ(const T& angle)
		{
			T s = static_cast<T>(sin(angle));
			T c = static_cast<T>(cos(angle));
			
			m44[0][0] = c; m44[0][1] = s; m44[0][2] = 0; m44[0][3] = 0;
			m44[1][0] =-s; m44[1][1] = c; m44[1][2] = 0; m44[1][3] = 0;
			m44[2][0] = 0; m44[2][1] = 0; m44[2][2] = 1; m44[2][3] = 0;
			m44[3][0] = 0; m44[3][1] = 0; m44[3][2] = 0; m44[3][3] = 1;
		}

		void RotateXYZ(const T& pitch, const T& yaw, const T& roll)
		{
			T sp = static_cast<T>(sin(pitch));
			T cp = static_cast<T>(cos(pitch));
			T sy = static_cast<T>(sin(yaw));
			T cy = static_cast<T>(cos(yaw));
			T sr = static_cast<T>(sin(roll));
			T cr = static_cast<T>(cos(roll));
			T sysp = sy * sp; 
			T sycp = sy * cp; 

			m44[0][0] = cr * cy;
			m44[0][1] = sr * cy;
			m44[0][2] = -sy;
			m44[0][3] = 0;
			m44[1][0] = cr * sysp - sr * cp;
			m44[1][1] = sr * sysp + cr * cp;
			m44[1][2] = cy * sp;
			m44[1][3] = 0;
			m44[2][0] = cr * sycp + sr * sp;
			m44[2][1] = sr * sycp - cr * sp;
			m44[2][2] = cy * cp;
			m44[2][3] = 0;
			m44[3][0] = 0;
			m44[3][1] = 0;
			m44[3][2] = 0;
			m44[3][3] = 1;
		} 

		void RotateEuler(const Vector3<T>& v, EulerOrder euler)
		{
			switch ( euler )
			{
				case EULER_XYZ: RotateX(v.x); PostMultRotateY(v.y); PostMultRotateZ(v.z); break;
				case EULER_XZY: RotateX(v.x); PostMultRotateZ(v.z); PostMultRotateY(v.y); break;
				case EULER_YXZ: RotateY(v.y); PostMultRotateX(v.x); PostMultRotateZ(v.z); break;
				case EULER_YZX: RotateY(v.y); PostMultRotateZ(v.z); PostMultRotateX(v.x); break;
				case EULER_ZXY: RotateZ(v.z); PostMultRotateX(v.x); PostMultRotateY(v.y); break;
				case EULER_ZYX: RotateZ(v.z); PostMultRotateY(v.y); PostMultRotateX(v.x); break;
				default: break;
			}
		}

		void RotateAngleAxis(const T& angle, const Vector3<T>& axis)
		{
			T length2 = LengthSquared(axis);
			if ( length2 == 0 )
			{
				Identity();
				return;
			}
	
			Vector3<T> n = axis / static_cast<T>(sqrt(length2));
			T s = static_cast<T>(sin(angle));
			T c = static_cast<T>(cos(angle));
			T k = 1 - c;

			T xx = n.x * n.x * k + c;
			T yy = n.y * n.y * k + c;
			T zz = n.z * n.z * k + c;
			T xy = n.x * n.y * k;
			T yz = n.y * n.z * k;
			T zx = n.z * n.x * k;
			T xs = n.x * s;
			T ys = n.y * s;
			T zs = n.z * s;
	
			m44[0][0] = xx;
			m44[0][1] = xy + zs;
			m44[0][2] = zx - ys;
			m44[0][3] = 0;
			m44[1][0] = xy - zs;
			m44[1][1] = yy;
			m44[1][2] = yz + xs;
			m44[1][3] = 0;
			m44[2][0] = zx + ys;
			m44[2][1] = yz - xs;
			m44[2][2] = zz;
			m44[2][3] = 0;
			m44[3][0] = 0;
			m44[3][1] = 0;
			m44[3][2] = 0;
			m44[3][3] = 1;
		}

		// post-multiplication

		void PostMultScale(const T& sx, const T& sy, const T& sz)
		{
			for ( int i=0; i<4; ++i )
			{
				m44[i][0] *= sx;
				m44[i][1] *= sy;
				m44[i][2] *= sz;
			}
		}

		void PostMultScale(const Vector3<T>& scale)
		{
			PostMultScale(scale.x,scale.y,scale.z);
		}

		void PostMultTranslate(const T& tx, const T& ty, const T& tz)
		{
			for ( int i=0; i<4; ++i )
			{
				T e = m44[i][3];
				m44[i][0] += e * tx;
				m44[i][1] += e * ty;
				m44[i][2] += e * tz;
			}
		}

		void PostMultTranslate(const Vector3<T>& translate)
		{
			PostMultTranslate(translate.x,translate.y,translate.z);
		}

		void PostMultRotateX(const T& angle)
		{
			T s = static_cast<T>(sin(angle));
			T c = static_cast<T>(cos(angle));
	
			for ( int i=0; i<4; ++i )
			{
				T y = m44[i][1];
				T z = m44[i][2];
				m44[i][1] = y * c - z * s;
				m44[i][2] = z * c + y * s;
			}
		}

		void PostMultRotateY(const T& angle)
		{
			T s = static_cast<T>(sin(angle));
			T c = static_cast<T>(cos(angle));
	
			for ( int i=0; i<4; ++i )
			{
				T x = m44[i][0];
				T z = m44[i][2];
				m44[i][0] = x * c + z * s;
				m44[i][2] = z * c - x * s;
			}
		}

		void PostMultRotateZ(const T& angle)
		{
			T s = static_cast<T>(sin(angle));
			T c = static_cast<T>(cos(angle));
	
			for ( int i=0; i<4; ++i )
			{
				T x = m44[i][0];
				T y = m44[i][1];
				m44[i][0] = x * c - y * s;
				m44[i][1] = x * s + y * c;
			}
		}

		void PostMultRotateXYZ(const T& pitch, const T& yaw, const T& roll)
		{
			T sp = static_cast<T>(sin(pitch));
			T cp = static_cast<T>(cos(pitch));
			T sy = static_cast<T>(sin(yaw));
			T cy = static_cast<T>(cos(yaw));
			T sr = static_cast<T>(sin(roll));
			T cr = static_cast<T>(cos(roll));
			T sysp = sy * sp; 
			T sycp = sy * cp; 

			T m00 = cr * cy;
			T m01 = sr * cy;
			T m02 = -sy;
			T m10 = cr * sysp - sr * cp;
			T m11 = sr * sysp + cr * cp;
			T m12 = cy * sp;
			T m20 = cr * sycp + sr * sp;
			T m21 = sr * sycp - cr * sp;
			T m22 = cy * cp;

			for ( int i=0; i<4; ++i )
			{
				T x = m44[i][0];
				T y = m44[i][1];
				T z = m44[i][2];
				m44[i][0] = x * m00 + y * m10 + z * m20;
				m44[i][1] = x * m01 + y * m11 + z * m21;
				m44[i][2] = x * m02 + y * m12 + z * m22;
			}
		}

		void PostMultRotateEuler(const Vector3<T>& v, EulerOrder euler)
		{
			switch ( euler )
			{
				case EULER_XYZ: PostMultRotateX(v.x); PostMultRotateY(v.y); PostMultRotateZ(v.z); break;
				case EULER_XZY: PostMultRotateX(v.x); PostMultRotateZ(v.z); PostMultRotateY(v.y); break;
				case EULER_YXZ: PostMultRotateY(v.y); PostMultRotateX(v.x); PostMultRotateZ(v.z); break;
				case EULER_YZX: PostMultRotateY(v.y); PostMultRotateZ(v.z); PostMultRotateX(v.x); break;
				case EULER_ZXY: PostMultRotateZ(v.z); PostMultRotateX(v.x); PostMultRotateY(v.y); break;
				case EULER_ZYX: PostMultRotateZ(v.z); PostMultRotateY(v.y); PostMultRotateX(v.x); break;
				default: break;
			}
		}

		void PostMultMatrix4x4(const Matrix4x4& u)
		{
			for ( int i=0; i<4; ++i )
			{
				T v0 = m44[i][0];
				T v1 = m44[i][1];
				T v2 = m44[i][2];
				T v3 = m44[i][3];
				m44[i][0] = v0 * u[0][0] + v1 * u[1][0] + v2 * u[2][0] + v3 * u[3][0];
				m44[i][1] = v0 * u[0][1] + v1 * u[1][1] + v2 * u[2][1] + v3 * u[3][1];
				m44[i][2] = v0 * u[0][2] + v1 * u[1][2] + v2 * u[2][2] + v3 * u[3][2];
				m44[i][3] = v0 * u[0][3] + v1 * u[1][3] + v2 * u[2][3] + v3 * u[3][3];
			}
		}

		void PostMultMatrix3x4(const Matrix4x4& u)
		{
			for ( int i=0; i<4; ++i )
			{
				T v[3];
				v[0] = m44[i][0];
				v[1] = m44[i][1];
				v[2] = m44[i][2];
				m44[i][0] = v[0]*u.m44[0][0] + v[1]*u.m44[1][0] + v[2]*u.m44[2][0];
				m44[i][1] = v[0]*u.m44[0][1] + v[1]*u.m44[1][1] + v[2]*u.m44[2][1];
				m44[i][2] = v[0]*u.m44[0][2] + v[1]*u.m44[1][2] + v[2]*u.m44[2][2];
			}
			m44[3][0] += u.m44[3][0];
			m44[3][1] += u.m44[3][1];
			m44[3][2] += u.m44[3][2];
		}

		void PostMultMatrix3x3(const Matrix4x4& u)
		{
			for ( int i=0; i<4; ++i )
			{
				T v[3];
				v[0] = m44[i][0];
				v[1] = m44[i][1];
				v[2] = m44[i][2];
				m44[i][0] = v[0]*u.m44[0][0] + v[1]*u.m44[1][0] + v[2]*u.m44[2][0];
				m44[i][1] = v[0]*u.m44[0][1] + v[1]*u.m44[1][1] + v[2]*u.m44[2][1];
				m44[i][2] = v[0]*u.m44[0][2] + v[1]*u.m44[1][2] + v[2]*u.m44[2][2];
			}
		}

		void PostMultInverseMatrix3x3(const Matrix4x4& u)
		{
			T m00 = m44[0][0];
			T m01 = m44[0][1];
			T m02 = m44[0][2];
			T m10 = m44[1][0];
			T m11 = m44[1][1];
			T m12 = m44[1][2];
			T m20 = m44[2][0];
			T m21 = m44[2][1];
			T m22 = m44[2][2];

			for ( int i=0; i<4; ++i )
			{
				const T* v = u.m16 + i * 4;
				m44[i][0] = v[0]*m00 + v[1]*m10 + v[2]*m20;
				m44[i][1] = v[0]*m01 + v[1]*m11 + v[2]*m21;
				m44[i][2] = v[0]*m02 + v[1]*m12 + v[2]*m22;
			}
		}

		// pre-multiplication

		void PreMultScale(const T& sx, const T& sy, const T& sz)
		{
			for ( int i=0; i<4; ++i )
			{
				m44[0][i] *= sx;
				m44[1][i] *= sy;
				m44[2][i] *= sz;
			}
		}

		void PreMultScale(const Vector3<T>& scale)
		{
			PreMultScale(scale.x,scale.y,scale.z);
		}

		void PreMultTranslate(const T& tx, const T& ty, const T& tz)
		{
			for ( int i=0; i<4; ++i )
			{
				m44[3][i] = tx * m44[0][i] + ty * m44[1][i] + tz * m44[2][i] + m44[3][i];
			}
		}

		void PreMultTranslate(const Vector3<T>& translate)
		{
			PreMultTranslate(translate.x,translate.y,translate.z);
		}

		void PreMultRotateX(const T& angle)
		{
			// TODO: optimize this code
			Matrix4x4<T> temp;
			temp.RotateX(angle);
			*this = temp * *this;
		}

		void PreMultRotateY(const T& angle)
		{
			// TODO: optimize this code
			Matrix4x4<T> temp;
			temp.RotateY(angle);
			*this = temp * *this;
		}

		void PreMultRotateZ(const T& angle)
		{
			// TODO: optimize this code
			Matrix4x4<T> temp;
			temp.RotateZ(angle);
			*this = temp * *this;
		}

		void PreMultRotateXYZ(const T& yaw, const T& pitch, const T& roll)
		{
			// TODO: optimize this code
			Matrix4x4<T> temp;
			temp.RotateXYZ(yaw,pitch,roll);
			*this = temp * *this;
		}

		void PreMultRotateEuler(const Vector3<T>& v, EulerOrder euler)
		{
			// TODO: optimize this code
			Matrix4x4<T> temp;
			temp.RotateEuler(v,euler);
			*this = temp * *this;
		}

		void PreMultMatrix4x4(const Matrix4x4& u)
		{
			for ( int i=0; i<4; ++i )
			{
				T v0 = m44[0][i];
				T v1 = m44[1][i];
				T v2 = m44[2][i];
				T v3 = m44[3][i];
				m44[0][i] = u[0][0] * v0 + u[0][1] * v1 + u[0][2] * v2 + u[0][3] * v3;
				m44[1][i] = u[1][0] * v0 + u[1][1] * v1 + u[1][2] * v2 + u[1][3] * v3;
				m44[2][i] = u[2][0] * v0 + u[2][1] * v1 + u[2][2] * v2 + u[2][3] * v3;
				m44[3][i] = u[3][0] * v0 + u[3][1] * v1 + u[3][2] * v2 + u[3][3] * v3;
			}
		}

		void PreMultMatrix3x4(const Matrix4x4& u)
		{
			// TODO: optimize this code
			Matrix4x4<T> b = u;
			b.PostMultMatrix3x4(*this);
			*this = b;
		}

		void PreMultMatrix3x3(const Matrix4x4& u)
		{
			// TODO: optimize this code
			Matrix4x4<T> b = u;
			b.PostMultMatrix3x3(*this);
			*this = b;
		}

		void PreMultInverseMatrix3x3(const Matrix4x4& u)
		{
			// TODO: optimize this code
			Matrix4x4<T> b = u;
			b.PostMultInverseMatrix3x3(*this);
			*this = b;
		}

		void Transpose()
		{
			std::swap(m44[0][1],m44[1][0]);
			std::swap(m44[0][2],m44[2][0]);
			std::swap(m44[0][3],m44[3][0]);
			std::swap(m44[1][2],m44[2][1]);
			std::swap(m44[1][3],m44[3][1]);
			std::swap(m44[2][3],m44[3][2]);
		}

		void Transpose(const Matrix4x4& u)
		{
			m44[0][0] = u.m44[0][0];
			m44[0][1] = u.m44[1][0];
			m44[0][2] = u.m44[2][0];
			m44[0][3] = u.m44[3][0];
			m44[1][0] = u.m44[0][1];
			m44[1][1] = u.m44[1][1];
			m44[1][2] = u.m44[2][1];
			m44[1][3] = u.m44[3][1];
			m44[2][0] = u.m44[0][2];
			m44[2][1] = u.m44[1][2];
			m44[2][2] = u.m44[2][2];
			m44[2][3] = u.m44[3][2];
			m44[3][0] = u.m44[0][3];
			m44[3][1] = u.m44[1][3];
			m44[3][2] = u.m44[2][3];
			m44[3][3] = u.m44[3][3];
		}

		void Adjoint()
		{
			T v00 = m44[1][1]*m44[2][2] - m44[1][2]*m44[2][1];
			T v01 = m44[2][1]*m44[0][2] - m44[2][2]*m44[0][1];
			T v02 = m44[0][1]*m44[1][2] - m44[0][2]*m44[1][1];
			T v10 = m44[1][2]*m44[2][0] - m44[1][0]*m44[2][2];
			T v11 = m44[2][2]*m44[0][0] - m44[2][0]*m44[0][2];
			T v12 = m44[0][2]*m44[1][0] - m44[0][0]*m44[1][2];
			T v20 = m44[1][0]*m44[2][1] - m44[1][1]*m44[2][0];
			T v21 = m44[2][0]*m44[0][1] - m44[2][1]*m44[0][0];
			T v22 = m44[0][0]*m44[1][1] - m44[0][1]*m44[1][0];
			T v30 = -(m44[0][0]*m44[3][0] + m44[1][0]*m44[3][1] + m44[2][0]*m44[3][2]);
			T v31 = -(m44[0][1]*m44[3][0] + m44[1][1]*m44[3][1] + m44[2][1]*m44[3][2]);
			T v32 = -(m44[0][2]*m44[3][0] + m44[1][2]*m44[3][1] + m44[2][2]*m44[3][2]);
			m44[0][0] = v00;
			m44[0][1] = v01;
			m44[0][2] = v02;
			m44[1][0] = v10;
			m44[1][1] = v11;
			m44[1][2] = v12;
			m44[2][0] = v20;
			m44[2][1] = v21;
			m44[2][2] = v22;
			m44[3][0] = v30;
			m44[3][1] = v31;
			m44[3][2] = v32;
		}

		void Adjoint(const Matrix4x4& u)
		{
			m44[0][0] = u.m44[1][1]*u.m44[2][2] - u.m44[1][2]*u.m44[2][1];
			m44[0][1] = u.m44[2][1]*u.m44[0][2] - u.m44[2][2]*u.m44[0][1];
			m44[0][2] = u.m44[0][1]*u.m44[1][2] - u.m44[0][2]*u.m44[1][1];
			m44[0][3] = u.m44[0][3];
			m44[1][0] = u.m44[1][2]*u.m44[2][0] - u.m44[1][0]*u.m44[2][2];
			m44[1][1] = u.m44[2][2]*u.m44[0][0] - u.m44[2][0]*u.m44[0][2];
			m44[1][2] = u.m44[0][2]*u.m44[1][0] - u.m44[0][0]*u.m44[1][2];
			m44[1][3] = u.m44[1][3];
			m44[2][0] = u.m44[1][0]*u.m44[2][1] - u.m44[1][1]*u.m44[2][0];
			m44[2][1] = u.m44[2][0]*u.m44[0][1] - u.m44[2][1]*u.m44[0][0];
			m44[2][2] = u.m44[0][0]*u.m44[1][1] - u.m44[0][1]*u.m44[1][0];
			m44[2][3] = u.m44[2][3];
			m44[3][0] = -(u.m44[0][0]*u.m44[3][0] + u.m44[1][0]*u.m44[3][1] + u.m44[2][0]*u.m44[3][2]);
			m44[3][1] = -(u.m44[0][1]*u.m44[3][0] + u.m44[1][1]*u.m44[3][1] + u.m44[2][1]*u.m44[3][2]);
			m44[3][2] = -(u.m44[0][2]*u.m44[3][0] + u.m44[1][2]*u.m44[3][1] + u.m44[2][2]*u.m44[3][2]);
			m44[3][3] = u.m44[3][3];
		}

		void Inverse(bool affine = true)
		{
			if ( affine )
			{
				T s = 1 / GetDeterminant();
				T v00 = (m44[1][1]*m44[2][2] - m44[1][2]*m44[2][1]) * s;
				T v01 = (m44[2][1]*m44[0][2] - m44[2][2]*m44[0][1]) * s;
				T v02 = (m44[0][1]*m44[1][2] - m44[0][2]*m44[1][1]) * s;
				T v10 = (m44[1][2]*m44[2][0] - m44[1][0]*m44[2][2]) * s;
				T v11 = (m44[2][2]*m44[0][0] - m44[2][0]*m44[0][2]) * s;
				T v12 = (m44[0][2]*m44[1][0] - m44[0][0]*m44[1][2]) * s;
				T v20 = (m44[1][0]*m44[2][1] - m44[1][1]*m44[2][0]) * s;
				T v21 = (m44[2][0]*m44[0][1] - m44[2][1]*m44[0][0]) * s;
				T v22 = (m44[0][0]*m44[1][1] - m44[0][1]*m44[1][0]) * s;
				T v30 = -(v00*m44[3][0] + v10*m44[3][1] + v20*m44[3][2]);
				T v31 = -(v01*m44[3][0] + v11*m44[3][1] + v21*m44[3][2]);
				T v32 = -(v02*m44[3][0] + v12*m44[3][1] + v22*m44[3][2]);
				m44[0][0] = v00;
				m44[0][1] = v01;
				m44[0][2] = v02;
				m44[1][0] = v10;
				m44[1][1] = v11;
				m44[1][2] = v12;
				m44[2][0] = v20;
				m44[2][1] = v21;
				m44[2][2] = v22;
				m44[3][0] = v30;
				m44[3][1] = v31;
				m44[3][2] = v32;
			}
			else
			{
				// full 4x4 matrix inverse using Cramer's Rule
				// original code: Intel Corp., adapted to prmath

				// transpose matrix
				T src[16];
				for ( int i=0; i<4; ++i )
				{
					src[i]      = m16[i*4];
					src[i + 4]  = m16[i*4 + 1];
					src[i + 8]  = m16[i*4 + 2];
					src[i + 12] = m16[i*4 + 3];
				}

				// calculate pairs for first 8 elements (cofactors)
				T tmp[12]; // temp array for pairs
				tmp[0]  = src[10] * src[15];
				tmp[1]  = src[11] * src[14];
				tmp[2]  = src[9]  * src[15];
				tmp[3]  = src[11] * src[13];
				tmp[4]  = src[9]  * src[14];
				tmp[5]  = src[10] * src[13];
				tmp[6]  = src[8]  * src[15];
				tmp[7]  = src[11] * src[12];
				tmp[8]  = src[8]  * src[14];
				tmp[9]  = src[10] * src[12];
				tmp[10] = src[8]  * src[13];
				tmp[11] = src[9]  * src[12];

				// calculate first 8 elements (cofactors)
				m16[0] = (tmp[0] * src[5] + tmp[3] * src[6] + tmp[4]  * src[7]) - (tmp[1] * src[5] + tmp[2] * src[6] + tmp[5]  * src[7]);
				m16[1] = (tmp[1] * src[4] + tmp[6] * src[6] + tmp[9]  * src[7]) - (tmp[0] * src[4] + tmp[7] * src[6] + tmp[8]  * src[7]);
				m16[2] = (tmp[2] * src[4] + tmp[7] * src[5] + tmp[10] * src[7]) - (tmp[3] * src[4] + tmp[6] * src[5] + tmp[11] * src[7]);
				m16[3] = (tmp[5] * src[4] + tmp[8] * src[5] + tmp[11] * src[6]) - (tmp[4] * src[4] + tmp[9] * src[5] + tmp[10] * src[6]);
				m16[4] = (tmp[1] * src[1] + tmp[2] * src[2] + tmp[5]  * src[3]) - (tmp[0] * src[1] + tmp[3] * src[2] + tmp[4]  * src[3]);
				m16[5] = (tmp[0] * src[0] + tmp[7] * src[2] + tmp[8]  * src[3]) - (tmp[1] * src[0] + tmp[6] * src[2] + tmp[9]  * src[3]);
				m16[6] = (tmp[3] * src[0] + tmp[6] * src[1] + tmp[11] * src[3]) - (tmp[2] * src[0] + tmp[7] * src[1] + tmp[10] * src[3]);
				m16[7] = (tmp[4] * src[0] + tmp[9] * src[1] + tmp[10] * src[2]) - (tmp[5] * src[0] + tmp[8] * src[1] + tmp[11] * src[2]);

				// calculate pairs for second 8 elements (cofactors)
				tmp[0]  = src[2] * src[7];
				tmp[1]  = src[3] * src[6];
				tmp[2]  = src[1] * src[7];
				tmp[3]  = src[3] * src[5];
				tmp[4]  = src[1] * src[6];
				tmp[5]  = src[2] * src[5];
				tmp[6]  = src[0] * src[7];
				tmp[7]  = src[3] * src[4];
				tmp[8]  = src[0] * src[6];
				tmp[9]  = src[2] * src[4];
				tmp[10] = src[0] * src[5];
				tmp[11] = src[1] * src[4];
			
				// calculate second 8 elements (cofactors)
				m16[8]  = (tmp[0]  * src[13] + tmp[3]  * src[14] + tmp[4]  * src[15]) - (tmp[1]  * src[13] + tmp[2]  * src[14] + tmp[5]  * src[15]);
				m16[9]  = (tmp[1]  * src[12] + tmp[6]  * src[14] + tmp[9]  * src[15]) - (tmp[0]  * src[12] + tmp[7]  * src[14] + tmp[8]  * src[15]);
				m16[10] = (tmp[2]  * src[12] + tmp[7]  * src[13] + tmp[10] * src[15]) - (tmp[3]  * src[12] + tmp[6]  * src[13] + tmp[11] * src[15]);
				m16[11] = (tmp[5]  * src[12] + tmp[8]  * src[13] + tmp[11] * src[14]) - (tmp[4]  * src[12] + tmp[9]  * src[13] + tmp[10] * src[14]);
				m16[12] = (tmp[2]  * src[10] + tmp[5]  * src[11] + tmp[1]  * src[9]) - (tmp[4]  * src[11] + tmp[0]  * src[9]  + tmp[3]  * src[10]);
				m16[13] = (tmp[8]  * src[11] + tmp[0]  * src[8]  + tmp[7]  * src[10]) - (tmp[6]  * src[10] + tmp[9]  * src[11] + tmp[1]  * src[8]);
				m16[14] = (tmp[6]  * src[9]  + tmp[11] * src[11] + tmp[3]  * src[8]) - (tmp[10] * src[11] + tmp[2]  * src[8]  + tmp[7]  * src[9]);
				m16[15] = (tmp[10] * src[10] + tmp[4]  * src[8]  + tmp[9]  * src[9]) - (tmp[8]  * src[9]  + tmp[11] * src[10] + tmp[5]  * src[8]);

				// calculate determinant
				T det = src[0]*m16[0]+src[1]*m16[1]+src[2]*m16[2]+src[3]*m16[3];

				// calculate matrix inverse
				det = 1 / det;
				for ( int j=0; j<16; ++j )
					m16[j] *= det;
			}
		}

		void Inverse(const Matrix4x4& u, bool affine = true)
		{
			if ( affine )
			{
				T s = 1 / u.GetDeterminant();
				m44[0][0] = (u.m44[1][1]*u.m44[2][2] - u.m44[1][2]*u.m44[2][1]) * s;
				m44[0][1] = (u.m44[2][1]*u.m44[0][2] - u.m44[2][2]*u.m44[0][1]) * s;
				m44[0][2] = (u.m44[0][1]*u.m44[1][2] - u.m44[0][2]*u.m44[1][1]) * s;
				m44[0][3] =  u.m44[0][3];
				m44[1][0] = (u.m44[1][2]*u.m44[2][0] - u.m44[1][0]*u.m44[2][2]) * s;
				m44[1][1] = (u.m44[2][2]*u.m44[0][0] - u.m44[2][0]*u.m44[0][2]) * s;
				m44[1][2] = (u.m44[0][2]*u.m44[1][0] - u.m44[0][0]*u.m44[1][2]) * s;
				m44[1][3] =  u.m44[1][3];
				m44[2][0] = (u.m44[1][0]*u.m44[2][1] - u.m44[1][1]*u.m44[2][0]) * s;
				m44[2][1] = (u.m44[2][0]*u.m44[0][1] - u.m44[2][1]*u.m44[0][0]) * s;
				m44[2][2] = (u.m44[0][0]*u.m44[1][1] - u.m44[0][1]*u.m44[1][0]) * s;
				m44[2][3] =  u.m44[2][3];
				m44[3][0] =  -(m44[0][0]*u.m44[3][0] + m44[1][0]*u.m44[3][1] + m44[2][0]*u.m44[3][2]);
				m44[3][1] =  -(m44[0][1]*u.m44[3][0] + m44[1][1]*u.m44[3][1] + m44[2][1]*u.m44[3][2]);
				m44[3][2] =  -(m44[0][2]*u.m44[3][0] + m44[1][2]*u.m44[3][1] + m44[2][2]*u.m44[3][2]);
				m44[3][3] =  u.m44[3][3];
			}
			else
			{
				// full 4x4 matrix inverse using Cramer's Rule
				// original code: Intel Corp., adapted to prmath

				// transpose matrix
				T src[16];
				for ( int i=0; i<4; ++i )
				{
					src[i]      = u.m16[i*4];
					src[i + 4]  = u.m16[i*4 + 1];
					src[i + 8]  = u.m16[i*4 + 2];
					src[i + 12] = u.m16[i*4 + 3];
				}

				// calculate pairs for first 8 elements (cofactors)
				T tmp[12]; // temp array for pairs
				tmp[0]  = src[10] * src[15];
				tmp[1]  = src[11] * src[14];
				tmp[2]  = src[9]  * src[15];
				tmp[3]  = src[11] * src[13];
				tmp[4]  = src[9]  * src[14];
				tmp[5]  = src[10] * src[13];
				tmp[6]  = src[8]  * src[15];
				tmp[7]  = src[11] * src[12];
				tmp[8]  = src[8]  * src[14];
				tmp[9]  = src[10] * src[12];
				tmp[10] = src[8]  * src[13];
				tmp[11] = src[9]  * src[12];

				// calculate first 8 elements (cofactors)
				m16[0] = (tmp[0] * src[5] + tmp[3] * src[6] + tmp[4]  * src[7]) - (tmp[1] * src[5] + tmp[2] * src[6] + tmp[5]  * src[7]);
				m16[1] = (tmp[1] * src[4] + tmp[6] * src[6] + tmp[9]  * src[7]) - (tmp[0] * src[4] + tmp[7] * src[6] + tmp[8]  * src[7]);
				m16[2] = (tmp[2] * src[4] + tmp[7] * src[5] + tmp[10] * src[7]) - (tmp[3] * src[4] + tmp[6] * src[5] + tmp[11] * src[7]);
				m16[3] = (tmp[5] * src[4] + tmp[8] * src[5] + tmp[11] * src[6]) - (tmp[4] * src[4] + tmp[9] * src[5] + tmp[10] * src[6]);
				m16[4] = (tmp[1] * src[1] + tmp[2] * src[2] + tmp[5]  * src[3]) - (tmp[0] * src[1] + tmp[3] * src[2] + tmp[4]  * src[3]);
				m16[5] = (tmp[0] * src[0] + tmp[7] * src[2] + tmp[8]  * src[3]) - (tmp[1] * src[0] + tmp[6] * src[2] + tmp[9]  * src[3]);
				m16[6] = (tmp[3] * src[0] + tmp[6] * src[1] + tmp[11] * src[3]) - (tmp[2] * src[0] + tmp[7] * src[1] + tmp[10] * src[3]);
				m16[7] = (tmp[4] * src[0] + tmp[9] * src[1] + tmp[10] * src[2]) - (tmp[5] * src[0] + tmp[8] * src[1] + tmp[11] * src[2]);

				// calculate pairs for second 8 elements (cofactors)
				tmp[0]  = src[2] * src[7];
				tmp[1]  = src[3] * src[6];
				tmp[2]  = src[1] * src[7];
				tmp[3]  = src[3] * src[5];
				tmp[4]  = src[1] * src[6];
				tmp[5]  = src[2] * src[5];
				tmp[6]  = src[0] * src[7];
				tmp[7]  = src[3] * src[4];
				tmp[8]  = src[0] * src[6];
				tmp[9]  = src[2] * src[4];
				tmp[10] = src[0] * src[5];
				tmp[11] = src[1] * src[4];
			
				// calculate second 8 elements (cofactors)
				m16[8]  = (tmp[0]  * src[13] + tmp[3]  * src[14] + tmp[4]  * src[15]) - (tmp[1]  * src[13] + tmp[2]  * src[14] + tmp[5]  * src[15]);
				m16[9]  = (tmp[1]  * src[12] + tmp[6]  * src[14] + tmp[9]  * src[15]) - (tmp[0]  * src[12] + tmp[7]  * src[14] + tmp[8]  * src[15]);
				m16[10] = (tmp[2]  * src[12] + tmp[7]  * src[13] + tmp[10] * src[15]) - (tmp[3]  * src[12] + tmp[6]  * src[13] + tmp[11] * src[15]);
				m16[11] = (tmp[5]  * src[12] + tmp[8]  * src[13] + tmp[11] * src[14]) - (tmp[4]  * src[12] + tmp[9]  * src[13] + tmp[10] * src[14]);
				m16[12] = (tmp[2]  * src[10] + tmp[5]  * src[11] + tmp[1]  * src[9]) - (tmp[4]  * src[11] + tmp[0]  * src[9]  + tmp[3]  * src[10]);
				m16[13] = (tmp[8]  * src[11] + tmp[0]  * src[8]  + tmp[7]  * src[10]) - (tmp[6]  * src[10] + tmp[9]  * src[11] + tmp[1]  * src[8]);
				m16[14] = (tmp[6]  * src[9]  + tmp[11] * src[11] + tmp[3]  * src[8]) - (tmp[10] * src[11] + tmp[2]  * src[8]  + tmp[7]  * src[9]);
				m16[15] = (tmp[10] * src[10] + tmp[4]  * src[8]  + tmp[9]  * src[9]) - (tmp[8]  * src[9]  + tmp[11] * src[10] + tmp[5]  * src[8]);

				// calculate determinant
				T det = src[0]*m16[0]+src[1]*m16[1]+src[2]*m16[2]+src[3]*m16[3];

				// calculate matrix inverse
				det = 1 / det;
				for ( int j=0; j<16; ++j )
					m16[j] *= det;
			}
		}

		void Inverse(const Quaternion<T>& q)
		{
			// quaternion inverse
			T n = -1 / q.Norm();
			T qx = q.x * n;
			T qy = q.y * n;
			T qz = q.z * n;
			T qw = q.w * -n;

			// quaternion to matrix
			T sx = qx * 2;
			T sy = qy * 2;
			T sz = qz * 2;
			T wx = qw * sx;
			T wy = qw * sy;
			T wz = qw * sz;
			T xx = qx * sx;
			T xy = qx * sy;
			T xz = qx * sz;
			T yy = qy * sy;
			T yz = qy * sz;
			T zz = qz * sz;
			
			m44[0][0] = 1 - yy - zz;
			m44[0][1] = xy + wz;
			m44[0][2] = xz - wy;
			m44[0][3] = 0;
			m44[1][0] = xy - wz;
			m44[1][1] = 1 - xx - zz;
			m44[1][2] = yz + wx;
			m44[1][3] = 0;
			m44[2][0] = xz + wy;
			m44[2][1] = yz - wx;
			m44[2][2] = 1 - xx - yy;
			m44[2][3] = 0;
			m44[3][0] = 0;
			m44[3][1] = 0;
			m44[3][2] = 0;
			m44[3][3] = 1;
		}

		void OrthoNormalize()
		{
			Vector3<T> x = GetX();
			Vector3<T> y = GetY();
			Vector3<T> z = GetZ();

			x = Normalize(x);
			y -= x * DotProduct(x,y);
			y = Normalize(y);
			z = CrossProduct(x,y);

			SetX(x);
			SetY(y);
			SetZ(z);
		}

		void Mirror(const Matrix4x4& transform, const Plane<T>& plane)
		{
			// components
			Vector3<T> x = transform.GetX();
			Vector3<T> y = transform.GetY();
			Vector3<T> z = transform.GetZ();
			Vector3<T> t = transform.GetT();
			Vector3<T> n2 = plane.normal * -2;

			// mirror translation
			Vector3<T> mt = t + n2 * (DotProduct(t,plane.normal) - plane.dist);
	
			// mirror x rotation
			x += t;
			x += n2 * (DotProduct(x,plane.normal) - plane.dist);
			x -= mt;

			// mirror y rotation
			y += t;
			y += n2 * (DotProduct(y,plane.normal) - plane.dist);
			y -= mt;

			// mirror z rotation
			z += t;
			z += n2 * (DotProduct(z,plane.normal) - plane.dist);
			z -= mt;

			// write result
			SetX(x);
			SetY(y);
			SetZ(z);
			SetT(mt);
			m44[0][3] = 0; 
			m44[1][3] = 0; 
			m44[2][3] = 0; 
			m44[3][3] = 1;
		}

		void LookAt(const Vector3<T>& target, const Vector3<T>& view, const Vector3<T>& up)
		{
			Vector3<T> z = Normalize(target - view);
			Vector3<T> x = Normalize(CrossProduct(up,z));
			Vector3<T> y = CrossProduct(z,x);

    		m44[0][0] = x.x;
    		m44[1][0] = x.y;
    		m44[2][0] = x.z;
    		m44[3][0] = -DotProduct(x,view);
    		m44[0][1] = y.x;
    		m44[1][1] = y.y;
		    m44[2][1] = y.z;
    		m44[3][1] = -DotProduct(y,view);
    		m44[0][2] = z.x;
    		m44[1][2] = z.y;
    		m44[2][2] = z.z;
    		m44[3][2] = -DotProduct(z,view);
    		m44[0][3] = 0;
    		m44[1][3] = 0;
	   		m44[2][3] = 0;
    		m44[3][3] = 1;
		}

		T GetDeterminant() const
		{
			return
				m44[0][0] * m44[1][1] * m44[2][2] +
				m44[0][1] * m44[1][2] * m44[2][0] +
				m44[0][2] * m44[1][0] * m44[2][1] -
				m44[0][2] * m44[1][1] * m44[2][0] -
				m44[0][1] * m44[1][0] * m44[2][2] -
				m44[0][0] * m44[1][2] * m44[2][1];
		}

		Vector3<T> GetEuler(EulerOrder euler = EULER_XYZ) const
		{
			// =========================================================================
			// original implementation from the Graphics Gems IV, Academic Press, 1994.
			// =========================================================================
			#define EulSafe "\000\001\002\000"
			#define EulNext "\001\002\000\001"
			#define EulGetOrd(ord,i,j,k,n,s,f) {unsigned o=ord;f=o&1;s=o&1; n=o&1;i=EulSafe[o&3];j=EulNext[i+n];k=EulNext[i+1-n];}
			#define EulOrd(i,p,r,f) (((((((i)<<1)+(p))<<1)+(r))<<1)+(f))

			#define EulOrdXYZs EulOrd(0,0,0,0)
			#define EulOrdXYXs EulOrd(0,0,1,0)
			#define EulOrdXZYs EulOrd(0,1,0,0)
			#define EulOrdXZXs EulOrd(0,1,1,0)
			#define EulOrdYZXs EulOrd(1,0,0,0)
			#define EulOrdYZYs EulOrd(1,0,1,0)
			#define EulOrdYXZs EulOrd(1,1,0,0)
			#define EulOrdYXYs EulOrd(1,1,1,0)
			#define EulOrdZXYs EulOrd(2,0,0,0)
			#define EulOrdZXZs EulOrd(2,0,1,0)
			#define EulOrdZYXs EulOrd(2,1,0,0)
			#define EulOrdZYZs EulOrd(2,1,1,0)

			#define EulOrdZYXr EulOrd(0,0,0,1)
			#define EulOrdXYXr EulOrd(0,0,1,1)
			#define EulOrdYZXr EulOrd(0,1,0,1)
			#define EulOrdXZXr EulOrd(0,1,1,1)
			#define EulOrdXZYr EulOrd(1,0,0,1)
			#define EulOrdYZYr EulOrd(1,0,1,1)
			#define EulOrdZXYr EulOrd(1,1,0,1)
			#define EulOrdYXYr EulOrd(1,1,1,1)
			#define EulOrdYXZr EulOrd(2,0,0,1)
			#define EulOrdZXZr EulOrd(2,0,1,1)
			#define EulOrdXYZr EulOrd(2,1,0,1)
			#define EulOrdZYZr EulOrd(2,1,1,1)

			int order = 0;
			switch ( euler )
			{
				case EULER_XYZ: order = EulOrdXYZs; break;
				case EULER_XZY: order = EulOrdXZYs; break;
				case EULER_YXZ: order = EulOrdYXZs; break;
				case EULER_YZX: order = EulOrdYZXs; break;
				case EULER_ZXY: order = EulOrdZXYs; break;
				case EULER_ZYX: order = EulOrdZYXs; break;
			}

			int i,j,k,n,s,f;
			EulGetOrd(order,i,j,k,n,s,f);
			
			T x,y,z;
			
			if ( s == 1 )
			{
				T sy = static_cast<T>(sqrt(m44[j][i]*m44[j][i] + m44[k][i]*m44[k][i]));
				if ( sy > std::numeric_limits<T>::epsilon() * 16 )
				{
					x = static_cast<T>(atan2(m44[j][i],m44[k][i]));
					y = static_cast<T>(atan2(sy,m44[i][i]));
					z = static_cast<T>(atan2(m44[i][j],-m44[i][k]));
				}
				else
				{
					x = static_cast<T>(atan2(-m44[k][j],m44[j][j]));
					y = static_cast<T>(atan2(sy,m44[i][i]));
					z = 0;
				}
			}
			else
			{
				T cy = static_cast<T>(sqrt(m44[i][i]*m44[i][i] + m44[i][j]*m44[i][j]));
				if ( cy > std::numeric_limits<T>::epsilon() * 16 )
				{
					x = static_cast<T>(atan2(m44[j][k],m44[k][k]));
					y = static_cast<T>(atan2(-m44[i][k],cy));
					z = static_cast<T>(atan2(m44[i][j],m44[i][i]));
				} 
				else 
				{
					x = static_cast<T>(atan2(-m44[k][j],m44[j][j]));
					y = static_cast<T>(atan2(-m44[i][k],cy));
					z = 0;
				}
			}

			if ( n == 1 )
			{
				return f == 1 ?
					Vector3<T>(-z,-y,-x) : Vector3<T>(-x,-y,-z);
			}

			return f == 1 ?
				Vector3<T>(z,y,x) : Vector3<T>(x,y,z);
		}

		void SetX(const T& xx, const T& xy, const T& xz)
		{
			m44[0][0] = xx;
			m44[0][1] = xy;
			m44[0][2] = xz;
		}

		void SetY(const T& yx, const T& yy, const T& yz)
		{
			m44[1][0] = yx;
			m44[1][1] = yy;
			m44[1][2] = yz;
		}

		void SetZ(const T& zx, const T& zy, const T& zz)
		{
			m44[2][0] = zx;
			m44[2][1] = zy;
			m44[2][2] = zz;
		}

		void SetT(const T& tx, const T& ty, const T& tz)
		{
			m44[3][0] = tx;
			m44[3][1] = ty;
			m44[3][2] = tz;
		}

		void SetX(const Vector3<T>& x)
		{
			m44[0][0] = x.x;
			m44[0][1] = x.y;
			m44[0][2] = x.z;
		}

		void SetY(const Vector3<T>& y)
		{
			m44[1][0] = y.x;
			m44[1][1] = y.y;
			m44[1][2] = y.z;
		}

		void SetZ(const Vector3<T>& z)
		{
			m44[2][0] = z.x;
			m44[2][1] = z.y;
			m44[2][2] = z.z;
		}

		void SetT(const Vector3<T>& t)
		{
			m44[3][0] = t.x;
			m44[3][1] = t.y;
			m44[3][2] = t.z;
		}

		Vector3<T>& GetX()
		{
			return *reinterpret_cast<Vector3<T>*>(m16+0);
		}

		Vector3<T>& GetY()
		{
			return *reinterpret_cast<Vector3<T>*>(m16+4);
		}

		Vector3<T>& GetZ()
		{
			return *reinterpret_cast<Vector3<T>*>(m16+8);
		}

		Vector3<T>& GetT()
		{
			return *reinterpret_cast<Vector3<T>*>(m16+12);
		}

		const Vector3<T>& GetX() const
		{
			return *reinterpret_cast<const Vector3<T>*>(m16+0);
		}

		const Vector3<T>& GetY() const
		{
			return *reinterpret_cast<const Vector3<T>*>(m16+4);
		}

		const Vector3<T>& GetZ() const
		{
			return *reinterpret_cast<const Vector3<T>*>(m16+8);
		}

		const Vector3<T>& GetT() const
		{
			return *reinterpret_cast<const Vector3<T>*>(m16+12);
		}

		// backward compatibililty layer

		void MultScale(const T& sx, const T& sy, const T& sz)
		{
			PostMultScale(sx,sy,sz);
		}

		void MultScale(const Vector3<T>& scale)
		{
			PostMultScale(scale);
		}

		void MultTranslate(const T& tx, const T& ty, const T& tz)
		{
			PostMultTranslate(tx,ty,tz);
		}

		void MultTranslate(const Vector3<T>& translate)
		{
			PostMultTranslate(translate);
		}

		void MultRotateX(const T& angle)
		{
			PostMultRotateX(angle);
		}

		void MultRotateY(const T& angle)
		{
			PostMultRotateY(angle);
		}

		void MultRotateZ(const T& angle)
		{
			PostMultRotateZ(angle);
		}

		void MultRotateXYZ(const T& yaw, const T& pitch, const T& roll)
		{
			PostMultRotateXYZ(yaw,pitch,roll);
		}

		void MultRotateEuler(const Vector3<T>& v, EulerOrder euler)
		{
			PostMultRotateEuler(v,euler);
		}

		void MultMatrix4x4(const Matrix4x4& u)
		{
			PostMultMatrix4x4(u);
		}

		void MultMatrix3x4(const Matrix4x4& u)
		{
			PostMultMatrix3x4(u);
		}

		void MultMatrix3x3(const Matrix4x4& u)
		{
			PostMultMatrix3x3(u);
		}

		void MultInverseMatrix3x3(const Matrix4x4& u)
		{
			PostMultInverseMatrix3x3(u);
		}
	};

	// inline templates

	template <typename T>
	inline Matrix4x4<T> MatrixIdentity()
	{
		Matrix4x4<T> m;
		m.Identity();
		return m;
	}

	template <typename T>
	inline Matrix4x4<T> MatrixScale(const T& x, const T& y, const T& z)
	{
		Matrix4x4<T> m;
		m.Scale(x,y,z);
		return m;
	}

	template <typename T>
	inline Matrix4x4<T> MatrixScale(const Vector3<T>& scale)
	{
		Matrix4x4<T> m;
		m.Scale(scale);
		return m;
	}

	template <typename T>
	inline Matrix4x4<T> MatrixTranslate(const T& x, const T& y, const T& z)
	{
		Matrix4x4<T> m;
		m.Translate(x,y,z);
		return m;
	}

	template <typename T>
	inline Matrix4x4<T> MatrixTranslate(const Vector3<T>& scale)
	{
		Matrix4x4<T> m;
		m.Translate(scale);
		return m;
	}

	template <typename T>
	inline Matrix4x4<T> MatrixRotateX(const T& angle)
	{
		Matrix4x4<T> m;
		m.RotateX(angle);
		return m;
	}

	template <typename T>
	inline Matrix4x4<T> MatrixRotateY(const T& angle)
	{
		Matrix4x4<T> m;
		m.RotateY(angle);
		return m;
	}

	template <typename T>
	inline Matrix4x4<T> MatrixRotateZ(const T& angle)
	{
		Matrix4x4<T> m;
		m.RotateZ(angle);
		return m;
	}

	template <typename T>
	inline Matrix4x4<T> MatrixRotateXYZ(const T& yaw, const T& pitch, const T& roll)
	{
		Matrix4x4<T> m;
		m.RotateXYZ(yaw,pitch,roll);
		return m;
	}

	template <typename T>
	inline Matrix4x4<T> MatrixRotateEuler(const Vector3<T>& v, EulerOrder euler)
	{
		Matrix4x4<T> m;
		m.RotateEuler(v,euler);
		return m;
	}

	template <typename T>
	inline Matrix4x4<T> MatrixRotateAngleAxis(const T& angle, const Vector3<T>& axis)
	{
		Matrix4x4<T> m;
		m.RotateAngleAxis(angle,axis);
		return m;
	}

	template <typename T>
	inline Matrix4x4<T> MatrixTranspose(const Matrix4x4<T>& m)
	{
		Matrix4x4<T> r;
		r.Transpose(m);
		return r;
	}

	template <typename T>
	inline Matrix4x4<T> MatrixAdjoint(const Matrix4x4<T>& m)
	{
		Matrix4x4<T> r;
		r.Adjoint(m);
		return r;
	}

	template <typename T>
	inline Matrix4x4<T> MatrixInverse(const Matrix4x4<T>& m, bool affine = true)
	{
		Matrix4x4<T> r;
		r.Inverse(m,affine);
		return r;
	}

	template <typename T>
	inline Matrix4x4<T> MatrixInverse(const Quaternion<T>& q)
	{
		Matrix4x4<T> r;
		r.Inverse(q);
		return r;
	}

	// inline operators

	template <typename T>
	inline void operator *= (Vector3<T>& v, const Matrix4x4<T>& m)
	{
		v =	Vector3<T>(
			v.x*m.m44[0][0] + v.y*m.m44[1][0] + v.z*m.m44[2][0] + m.m44[3][0],
			v.x*m.m44[0][1] + v.y*m.m44[1][1] + v.z*m.m44[2][1] + m.m44[3][1],
			v.x*m.m44[0][2] + v.y*m.m44[1][2] + v.z*m.m44[2][2] + m.m44[3][2]);
	}

	template <typename T>
	inline void operator *= (Vector4<T>& v, const Matrix4x4<T>& m)
	{
		v = Vector4<T>(
			v.x*m.m44[0][0] + v.y*m.m44[1][0] + v.z*m.m44[2][0] + v.w*m.m44[3][0],
			v.x*m.m44[0][1] + v.y*m.m44[1][1] + v.z*m.m44[2][1] + v.w*m.m44[3][1],
			v.x*m.m44[0][2] + v.y*m.m44[1][2] + v.z*m.m44[2][2] + v.w*m.m44[3][2],
			v.x*m.m44[0][3] + v.y*m.m44[1][3] + v.z*m.m44[2][3] + v.w*m.m44[3][3]);
	}

	template <typename T>
	inline Vector3<T> operator * (const Vector3<T>& v, const Matrix4x4<T>& m)
	{
		return Vector3<T>(
			v.x*m.m44[0][0] + v.y*m.m44[1][0] + v.z*m.m44[2][0] + m.m44[3][0],
			v.x*m.m44[0][1] + v.y*m.m44[1][1] + v.z*m.m44[2][1] + m.m44[3][1],
			v.x*m.m44[0][2] + v.y*m.m44[1][2] + v.z*m.m44[2][2] + m.m44[3][2]);
	}

	template <typename T>
	inline Vector4<T> operator * (const Vector4<T>& v, const Matrix4x4<T>& m)
	{
		return Vector4<T>(
			v.x*m.m44[0][0] + v.y*m.m44[1][0] + v.z*m.m44[2][0] + v.w*m.m44[3][0],
			v.x*m.m44[0][1] + v.y*m.m44[1][1] + v.z*m.m44[2][1] + v.w*m.m44[3][1],
			v.x*m.m44[0][2] + v.y*m.m44[1][2] + v.z*m.m44[2][2] + v.w*m.m44[3][2],
			v.x*m.m44[0][3] + v.y*m.m44[1][3] + v.z*m.m44[2][3] + v.w*m.m44[3][3]);
	}

} // namespace prmath


	#ifndef PRMATH_NOTYPENAME

	typedef prmath::Matrix4x4<float>	float4x4;
	typedef prmath::Matrix4x4<float>	matrix4x4f;

	typedef prmath::Matrix4x4<double>	double4x4;
	typedef prmath::Matrix4x4<double>	matrix4x4d;

	#endif // PRMATH_NOTYPENAME


#endif
