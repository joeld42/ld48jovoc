/*
	Twilight Prophecy SDK
	A multi-platform development system for virtual reality and multimedia.

	Copyright (C) 1997-2003 Twilight 3D Finland Oy Ltd.
*/
#ifndef PRMATH_QUATERNION_HPP
#define PRMATH_QUATERNION_HPP


#include "minmax.hpp"
#include <cmath>
#include <limits>
#include "vector3.hpp"
#include "euler.hpp"
#include "matrix4x4.hpp"


namespace prmath
{

	template <typename T>
	struct Matrix4x4;


	template <typename T>
	struct Quaternion
	{

		// members

		T x;
		T y;
		T z;
		T w;
		
		// constructors

		Quaternion()
		{
		}

		Quaternion(const T& qx, const T& qy, const T& qz, const T& qw)
		: x(qx), y(qy), z(qz), w(qw)
		{
		}

		Quaternion(const T v[])
		: x(v[0]), y(v[1]), z(v[2]), w(v[3])
		{
		}

		Quaternion(const Quaternion& q)
		: x(q.x), y(q.y), z(q.z), w(q.w)
		{
		}

		Quaternion(const T& angle, const Vector3<T>& axis)
		{
			SetAngleAxis(angle,axis);
		}

		Quaternion(const Matrix4x4<T>& u)
		{
			*this = u;
		}

		// operators

		Quaternion operator + () const
		{
			return *this;
		}

		Quaternion operator - () const
		{
			return Quaternion(-x, -y, -z, -w);
		}

		Quaternion operator + (const Quaternion& q) const
		{
			return Quaternion(x + q.x, y + q.y, z + q.z, w + q.w);
		}

		Quaternion operator - (const Quaternion& q) const
		{
			return Quaternion(x - q.x, y - q.y, z - q.z, w - q.w);
		}

		Quaternion operator * (const Quaternion& q) const
		{
			return Quaternion(
				w*q.x + x*q.w + y*q.z - z*q.y,
				w*q.y + y*q.w + z*q.x - x*q.z,
				w*q.z + z*q.w + x*q.y - y*q.x,
				w*q.w - x*q.x - y*q.y - z*q.z);
		}

		Quaternion operator * (const T& s) const
		{
			return Quaternion(x * s, y * s, z * s, w * s);
		}

		Quaternion& operator += (const Quaternion& q)
		{
			x += q.x;
			y += q.y; 
			z += q.z;
			w += q.w;
			return *this;
		}

		Quaternion& operator -= (const Quaternion& q)
		{
			x -= q.x;
			y -= q.y; 
			z -= q.z;
			w -= q.w;
			return *this;
		}

		Quaternion& operator *= (const Quaternion& q)
		{
			T sx = w*q.x + x*q.w + y*q.z - z*q.y;
			T sy = w*q.y + y*q.w + z*q.x - x*q.z;
			T sz = w*q.z + z*q.w + x*q.y - y*q.x;
			T sw = w*q.w - x*q.x - y*q.y - z*q.z;
			x = sx;
			y = sy;
			z = sz;
			w = sw;
			return *this;
		}

		Quaternion& operator *= (const T& s)
		{
			x *= s;
			y *= s;
			z *= s;
			w *= s;
			return *this;
		}

		Quaternion& operator = (const Quaternion& q)
		{
			x = q.x;
			y = q.y;
			z = q.z;
			w = q.w;
			return *this;
		}

		void operator = (const Matrix4x4<T>& u)
		{
			T tr = u.m44[0][0] + u.m44[1][1] + u.m44[2][2];
			if ( tr > 0 )
			{
				T s = static_cast<T>(sqrt(tr + 1));
				w = s * static_cast<T>(0.5);
				s = static_cast<T>(0.5) / s;
				x = (u.m44[1][2] - u.m44[2][1]) * s;
				y = (u.m44[2][0] - u.m44[0][2]) * s;
				z = (u.m44[0][1] - u.m44[1][0]) * s;
			}
			else
			{
				int i = 0;
				if ( u.m44[1][1] > u.m44[0][0] ) i = 1;
				if ( u.m44[2][2] > u.m44[i][i] ) i = 2;

				int j = i + 1; if ( j > 2 ) j = 0;
				int k = j + 1; if ( k > 2 ) k = 0;

				T s = static_cast<T>(sqrt(u.m44[i][i] - u.m44[j][j] - u.m44[k][k] + 1));
				if ( s < std::numeric_limits<T>::epsilon() )
				{
					x = y = z = 0;
					w = 1;
				}
				else
				{
					T* pq = reinterpret_cast<T*>(this);
					pq[i] = s * static_cast<T>(0.5);
					s = static_cast<T>(0.5) / s;
					pq[3] = (u.m44[j][k] - u.m44[k][j]) * s;
					pq[j] = (u.m44[i][j] + u.m44[j][i]) * s;
					pq[k] = (u.m44[i][k] + u.m44[k][i]) * s;
				}
			}
		}

		bool operator == (const Quaternion& q) const
		{
			return x == q.x && y == q.y && z == q.z && w == q.w;
		}

		bool operator != (const Quaternion& q) const
		{
			return x != q.x || y != q.y || z != q.z || w != q.w;
		}

		// methods

		void SetQuaternion(const T& qx, const T& qy, const T& qz, const T& qw)
		{
			x = qx;
			y = qy;
			z = qz;
			w = qw;
		}

		void SetAngleAxis(const T& angle, const Vector3<T>& axis)
		{
			T theta = angle * static_cast<T>(0.5);
			T s = static_cast<T>(sin(theta) / Length(axis));
			T c = static_cast<T>(cos(theta));
	
			x = axis.x * s;
			y = axis.y * s;
			z = axis.z * s;
			w = c;
		}

		void GetAngleAxis(T& angle, Vector3<T>& axis) const
		{
			T s = 1 / static_cast<T>(sqrt(1 - w*w));

			angle = static_cast<T>(acos(w) * 2.0);
			axis.x = x * s;
			axis.y = y * s;
			axis.z = z * s;
		}

		void SetEuler(const Vector3<T>& anglevec, EulerOrder euler)
		{
			T ex = 0;
			T ey = 0;
			T ez = 0;
			switch ( euler )
			{
				case EULER_XYZ: ex = anglevec.x; ey = anglevec.y; ez = anglevec.z; break;
				case EULER_XZY: ex = anglevec.x; ey = anglevec.z; ez = anglevec.y; break;
				case EULER_YXZ: ex = anglevec.y; ey = anglevec.x; ez = anglevec.z; break;
				case EULER_YZX: ex = anglevec.y; ey = anglevec.z; ez = anglevec.x; break;
				case EULER_ZXY: ex = anglevec.z; ey = anglevec.x; ez = anglevec.y; break;
				case EULER_ZYX: ex = anglevec.z; ey = anglevec.y; ez = anglevec.x; break;
			}

			ex *= static_cast<T>(0.5);
			ey *= static_cast<T>(0.5);
			ez *= static_cast<T>(0.5);

			T cx = static_cast<T>(cos(ex));
			T cy = static_cast<T>(cos(ey));
			T cz = static_cast<T>(cos(ez));
			T sx = static_cast<T>(sin(ex));
			T sy = static_cast<T>(sin(ey));
			T sz = static_cast<T>(sin(ez));

			T cycz = cy * cz;
			T sysz = sy * sz;
			T sycz = sy * cz;
			T cysz = cy * sz;

			x = sx * cycz - cx * sysz;
			y = sx * cysz + cx * sycz;
			z = cx * cysz - sx * sycz;
			w = cx * cycz + sx * sysz;
		}

		T Norm() const
		{
			return x*x + y*y + z*z + w*w;
		}

		T Mod() const
		{
			return static_cast<T>(sqrt(x*x + y*y + z*z + w*w));
		}

		void Identity()
		{
			x = y = z = 0;
			w = 1;
		}

		void Normalize()
		{
			T s = Norm();
			if ( s == 0 )
			{
				x = y = z = 0;
				w = 1;
			}
			else
			{
				T is = 1 / static_cast<T>(sqrt(s));
				x *= is;
				y *= is;
				z *= is;
				w *= is;
			}
		}

		void Conjugate()
		{
			x = -x;
			y = -y;
			z = -z;
		}

		void Inverse()
		{
			T n = -1 / Norm();
			x *= n;
			y *= n;
			z *= n;
			w *= -n;
		}

		void Negate()
		{
			T nim = -1 / Mod();
			x *= nim;
			y *= nim;
			z *= nim;
			w *= -nim;
		}

		void Exp()
		{
			T s = static_cast<T>(sqrt(x*x + y*y + z*z));
			w = static_cast<T>(cos(s));
			if ( s > std::numeric_limits<T>::epsilon() * 100 )
			{
				s = static_cast<T>(sin(s)) / s;
				x *= s;
				y *= s;
				z *= s;
			}
		}

		void Log()
		{
			T s = w ? static_cast<T>(atan2(sqrt(x*x + y*y + z*z),w)) : static_cast<T>(pi * 2);
			x *= s;
			y *= s;
			z *= s;
			w = 0;
		}

		void LnDif(const Quaternion& q)
		{
			Quaternion invp = *this;
			invp.Inverse();
			Quaternion mulp = invp * q;
	
			T length = static_cast<T>(sqrt(mulp.x*mulp.x + mulp.y*mulp.y + mulp.z*mulp.z));
			T scale = x*x + y*y + z*z + w*w;
			T mval = scale ? static_cast<T>(atan2(length,scale)) : static_cast<T>(pi * 2);
			if ( length != 0 ) mval /= length;
	
			x = mulp.x * mval;
			y = mulp.y * mval;
			z = mulp.z * mval;
			w = 0;
		}
	};

	// inline functions

	template <typename T>
	inline T DotProduct(const Quaternion<T>& a, const Quaternion<T>& b)
	{
		return a.x*b.x + a.y*b.y + a.z*b.z + a.w*b.w;
	}

	template <typename T>
	inline Quaternion<T> Lerp(const Quaternion<T>& a, const Quaternion<T>& b, const T& time)
	{
		return Quaternion<T>(
			a.x + (b.x - a.x) * time,
			a.y + (b.y - a.y) * time,
			a.z + (b.z - a.z) * time,
			a.w + (b.w - a.w) * time);
	}

	template <typename T>
	inline Quaternion<T> Slerp(const Quaternion<T>& a, const Quaternion<T>& b, const T& time)
	{

		// ====================================================
		// AART - Advanced Animation and Rendering Techniques
		// ====================================================

		T cosom = a.x*b.x + a.y*b.y + a.z*b.z + a.w*b.w;

		if ( (1 + cosom) > std::numeric_limits<T>::epsilon() )
		{
			T sp;
			T sq;

			if ( (1 - cosom) > std::numeric_limits<T>::epsilon() )
			{
				double omega = acos(cosom);
				double sinom = 1.0 / sin(omega);

				sp = static_cast<T>(sin((1 - time) * omega) * sinom);
				sq = static_cast<T>(sin(time * omega) * sinom);
			}
			else
			{
				sp = 1 - time;
				sq = time;
			}

			return Quaternion<T>(
				a.x*sp + b.x*sq,
				a.y*sp + b.y*sq,
				a.z*sp + b.z*sq,
				a.w*sp + b.w*sq);
		}
		else
		{
			T halfpi = static_cast<T>(pi / 2);
			T sp = static_cast<T>(sin((1 - time) * halfpi));
			T sq = static_cast<T>(sin(time * halfpi));

			return Quaternion<T>(
				a.x*sp - a.y*sq,
				a.y*sp + a.x*sq,
				a.z*sp - a.w*sq,
				a.z);
		}

		// ====================================================
		// Rusty's code
		// ====================================================

		/*
		T om, sinom; 
		T sp, sq; 
		Quaternion<T> qq, qret; 

		T l = a.x*b.x + a.y*b.y + a.z*b.z + a.w*b.w; 
		if ( (1 + l) > std::numeric_limits<T>::epsilon() ) 
		{ 
			T al = fabsf(l); 
			if ( al > 1 ) 
			l /= al; 

			om = static_cast<T>(acosf(l));
			sinom = static_cast<T>(sin(om));

			if ( fabsf(sinom) > std::numeric_limits<T>::epsilon() )
			{ 
				sp = static_cast<T>(sin((1-time)*om)) / sinom; 
				sq = static_cast<T>(sin(time*om)) / sinom; 
			} 
			else 
			{ 
				sp = 1 - time; 
				sq = time; 
			} 
			qret.x = sp * a.x + sq * b.x; 
			qret.y = sp * a.y + sq * b.y; 
			qret.z = sp * a.z + sq * b.z; 
			qret.w = sp * a.w + sq * b.w; 
		} 
		else 
		{ 
			qq.x = -a.y;
			qq.y = a.x;
			qq.z = -a.w;
			qq.w = a.z;

			T twopi = static_cast<T>(pi * 2);
			sp = static_cast<T>(sin((1-time) * twopi));
			sq = static_cast<T>(sin(time * twopi));

			qret.x = sp * a.x + sq * b.x; 
			qret.y = sp * a.y + sq * b.y; 
			qret.z = sp * a.z + sq * b.z; 
			qret.w = sp * a.w + sq * b.w; 
		} 
		return qret; 
 		*/

		// ====================================================
		// Original Version
		// ====================================================

		/*
		T cs = DotProduct(a,b);
		T sn = static_cast<T>(sqrt(fabs(1-cs*cs)));
	
		if ( fabs(sn) < std::numeric_limits<T>::epsilon() * 100 )
			return a;
	
		if ( cs < 0 )
			cs = -cs;
		
		T angle = static_cast<T>(atan2(sn,cs));
		sn = 1 / sn;
	
		return a * sn * static_cast<T>(sin(angle*(1-time))) + b * sn * static_cast<T>(sin(angle*time));
		*/
	}

	template <typename T>
	inline Quaternion<T> Slerp(const Quaternion<T>& a, const Quaternion<T>& b, const T& time, int spin)
	{
		T bflip = 1;
		T tcos = DotProduct(a,b);
	
		if ( tcos < 0 )
		{
			tcos = -tcos;
			bflip = -1;
		}
	
		if ( (1 - tcos) < std::numeric_limits<T>::epsilon() * 100 )
		{
			// linear interpolate
			return a * (1 - time) + b * (time * bflip);
		}
	
		T theta = static_cast<T>(acos(tcos));
		T phi = theta + static_cast<T>(spin * pi);
		T tsin = static_cast<T>(sin(theta));
		T beta = static_cast<T>(sin(theta - time*phi)) / tsin;
		T alpha = static_cast<T>(sin(time*phi)) / tsin * bflip;
	
		return a * beta + b * alpha;
	}

	template <typename T>
	inline Quaternion<T> Squad(const Quaternion<T>& p, const Quaternion<T>& a, const Quaternion<T>& b, const Quaternion<T>& q, const T& time)
	{
		Quaternion<T> qa = Slerp(p,q,time,0);
		Quaternion<T> qb = Slerp(a,b,time,0);
		T qtime = 2 * time * (1 - time);
		return Slerp(qa,qb,qtime,0);
	}

} // namespace prmath


	#ifndef PRMATH_NOTYPENAME

	typedef prmath::Quaternion<float>	quat4f;
	typedef prmath::Quaternion<double>	quat4d;

	#endif // PRMATH_NOTYPENAME


#endif
