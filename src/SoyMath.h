#pragma once

//	include defines in cmath
#define _USE_MATH_DEFINES

#include "SoyTypes.h"
#include <cmath>
#include "SoyVector.h"
#include <algorithm>

#if defined(TARGET_WINDOWS)
#include <math.h>
//	turn the double into a float - todo: make sure this is done at compile time
#define PIf	static_cast<float>(M_PI)
#else
	#define PIf	M_PI
#endif

namespace Soy
{
	template<typename TYPE>
	bool		IsValidFloat(const TYPE& f)
	{
		return std::isfinite(f);	//	not infinite and not nan
	};

	inline float RadToDeg(float Radians)
	{
		return Radians * (180.f/ PIf);
	}
	
	inline float DegToRad(float Degrees)
	{
		return Degrees * (PIf / 180.f);
	}
	
	
	float	AngleDegDiff(float Angle,float Base);	//	get the smallest signed difference from Base
	
	template<typename T>
	T	Lerp(const T& Start,const T& End,float Time)
	{
		return Start + static_cast<T>(static_cast<float>(End-Start) * Time);
	}
	
	//	gr: was "GetMathTime". Range doesn't scream "unlerp" to me, but still, this is the conventional name (I think it's in glsl too)
	template<typename T>
	float	Range(const T& Value,const T& Start,const T& End)
	{
		return static_cast<float>(Value-Start) / static_cast<float>(End-Start);
	}
	
	//	maybe not "math" ?
	class THsl;
	class TRgb;
	
	typedef vec3x<uint8> TRgb8;
	typedef vec4x<uint8> TRgba8;
}


namespace SoyMath
{
	float3x3	GetFlipMatrix3x3();

	uint32_t	GetNextPower2(uint32_t x);	//	returns X if already power. 0 returns 0.
	bool		IsPower2(uint32_t x);
};


//	expanded std functions
namespace std
{
	template<typename T>
	inline const T&	min(const T& a,const T& b,const T& c)
	{
		return std::min<T>( a, std::min<T>( b, c ) );
	}
	
	template<typename T>
	inline const T&	max(const T& a,const T& b,const T& c)
	{
		return std::max<T>( a, std::max<T>( b, c ) );
	}
	
	template<typename T>
	inline void			clamp(T& a,const T& Min,const T& Max)
	{
		//	gr: simple comparisons, not min/max funcs in case this is complex
		if ( a < Min )
			a = Min;
		if ( a > Max )
			a = Max;
	}
	
	template<typename T>
	inline const T&		clamped(const T& a,const T& Min,const T& Max)
	{
		//	gr: simple comparisons, not min/max funcs in case this is complex
		if ( a < Min )
			return Min;
			
		if ( a > Max )
			return Max;
		
		return a;
	}
}



class Soy::THsl
{
public:
	THsl()					{}
	THsl(float h,float s,float l) :
		mHsl	( h, s, l )
	{
	}
	THsl(const TRgb& rgb);
	
	float&			h()			{	return mHsl.x;	}
	float&			s()			{	return mHsl.y;	}
	float&			l()			{	return mHsl.z;	}
	const float&	h() const	{	return mHsl.x;	}
	const float&	s() const	{	return mHsl.y;	}
	const float&	l() const	{	return mHsl.z;	}
	
	TRgb			rgb() const;

public:
	vec3f		mHsl;
};
DECLARE_NONCOMPLEX_TYPE( Soy::THsl );



class Soy::TRgb
{
public:
	TRgb() :
		TRgb	( 0, 0, 0 )
	{
	}
	TRgb(float r,float g,float b) :
		mRgb	( r,g,b )
	{		
	}
	TRgb(const THsl& Hsl);
	
	float&			r()			{	return mRgb.x;	}
	float&			g()			{	return mRgb.y;	}
	float&			b()			{	return mRgb.z;	}
	const float&	r() const	{	return mRgb.x;	}
	const float&	g() const	{	return mRgb.y;	}
	const float&	b() const	{	return mRgb.z;	}
	THsl			hsl() const;
	
public:
	vec3f		mRgb;
};
DECLARE_NONCOMPLEX_TYPE( Soy::TRgb );






namespace Soy
{
	inline	TRgb	THsl::rgb() const	{	return TRgb(*this);	}
	inline	THsl	TRgb::hsl() const	{	return THsl(*this);	}
}

