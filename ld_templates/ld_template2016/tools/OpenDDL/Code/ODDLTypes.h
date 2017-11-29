/*
	OpenDDL Library Software License
	==================================

	OpenDDL Library, version 1.1
	Copyright 2014-2015, Eric Lengyel
	All rights reserved.

	The OpenDDL Library is free software published on the following website:

		http://openddl.org/

	Redistribution and use in source and binary forms, with or without modification,
	are permitted provided that the following conditions are met:

	1. Redistributions of source code must retain the entire text of this license,
	comprising the above copyright notice, this list of conditions, and the following
	disclaimer.
	
	2. Redistributions of any modified source code files must contain a prominent
	notice immediately following this license stating that the contents have been
	modified from their original form.

	3. Redistributions in binary form must include attribution to the author in any
	listing of credits provided with the distribution. If there is no listing of
	credits, then attribution must be included in the documentation and/or other
	materials provided with the distribution. The attribution must be exactly the
	statement "This software contains the OpenDDL Library by Eric Lengyel" (without
	quotes) in the case that the distribution contains the original, unmodified
	OpenDDL Library, or it must be exactly the statement "This software contains a
	modified version of the OpenDDL Library by Eric Lengyel" (without quotes) in the
	case that the distribution contains a modified version of the OpenDDL Library.

	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
	ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
	WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
	IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
	INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
	NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
	PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
	WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
	ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
	POSSIBILITY OF SUCH DAMAGE.
*/


#ifndef ODDLTypes_h
#define ODDLTypes_h

#include <math.h>
#include <new>

#ifdef __APPLE__
#include <inttypes.h>
#endif

namespace ODDL
{
	#define restrict __restrict


	typedef signed char				int8;
	typedef unsigned char			unsigned_int8;

	typedef short					int16;
	typedef unsigned short			unsigned_int16;

	typedef int						int32;
	typedef unsigned int			unsigned_int32;

#ifdef __APPLE__
	typedef int64_t					int64;
	typedef uint64_t				unsigned_int64;
#else
	typedef __int64					int64;
	typedef unsigned __int64		unsigned_int64;
#endif

	#if defined(_WIN64)

		typedef __int64				machine;
		typedef unsigned __int64	unsigned_machine;

		typedef __int64				machine_int;
		typedef unsigned __int64	unsigned_machine_int;

	#else

		typedef long				machine;
		typedef unsigned long		unsigned_machine;

		typedef long				machine_int;
		typedef unsigned long		unsigned_machine_int;

	#endif


	inline int32 Abs(int32 x)
	{
		int32 a = x >> 31;
		return ((x ^ a) - a);
	}

	inline int32 Min(int32 x, int32 y)
	{
		int32 a = x - y;
		return (x - (a & ~(a >> 31)));
	}

	inline int32 Max(int32 x, int32 y)
	{
		int32 a = x - y;
		return (x - (a & (a >> 31)));
	}

	inline int32 MinZero(int32 x)
	{
		return (x & (x >> 31));
	}

	inline int32 MaxZero(int32 x)
	{
		return (x & ~(x >> 31));
	}


	class Half
	{
		private:

			unsigned_int16		value;

			Half(unsigned_int16 v)
			{
				value = v;
			}

			float GetFloat(void) const
			{
				unsigned_int32 h = value;
				unsigned_int32 s = (h & 0x8000) << 16;
				int32 e = int32((h >> 10) & 0x1F) - 15;

				if (e >= -14)
				{
					unsigned_int32		f;

					if (e <= 15)
					{
						unsigned_int32 m = (h & 0x003FF) << 13;
						f = s | ((e + 112) << 23) | m;
					}
					else
					{
						f = s | 0x7C00;
					}

					return (reinterpret_cast<float&>(f));
				}

				return (reinterpret_cast<float&>(s));
			}

			void SetFloat(float v)
			{
				unsigned_int32 f = reinterpret_cast<unsigned_int32&>(v);
				unsigned_int32 s = (f >> 16) & 0x8000;
				int32 e = int32((f >> 23) & 0xFF) - 127;

				if (e >= -14)
				{
					if (e <= 15)
					{
						unsigned_int32 m = (f >> 13) & 0x03FF;
						value = unsigned_int16(s | ((e + 15) << 10) | m);
					}
					else
					{
						value = unsigned_int16(s | 0x7C00);
					}
				}
				else
				{
					value = unsigned_int16(s);
				}
			}

		public:

			Half() = default;

			Half(float v)
			{
				SetFloat(v);
			}

			Half(double v)
			{
				SetFloat(float(v));
			}

			operator float(void) const
			{
				return (GetFloat());
			}

			Half& operator =(const Half& h) = default;

			Half& operator =(float v)
			{
				SetFloat(v);
				return (*this);
			}

			Half& operator =(double v)
			{
				SetFloat(float(v));
				return (*this);
			}

			Half operator -(const Half& h) const
			{
				return (Half(unsigned_int16(h.value ^ 0x8000)));
			}
	};


	template <class type> class AutoDelete
	{
		private:

			type	*reference;

			AutoDelete(const AutoDelete&) = delete;

		public:

			explicit AutoDelete(type *ptr)
			{
				reference = ptr;
			}

			~AutoDelete()
			{
				delete reference;
			}

			operator type *(void) const
			{
				return (reference);
			}

			type *const *operator &(void) const
			{
				return (&reference);
			}

			type *operator ->(void) const
			{
				return (reference);
			}

			AutoDelete& operator =(type *ptr)
			{
				reference = ptr;
				return (*this);
			}
	};
}


#endif
