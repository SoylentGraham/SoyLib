#pragma once


//	some c++11 anomlies so highlight them and maybe one day we can remove them
//	currently need VS2013 for windows 7
//	http://stackoverflow.com/questions/70013/how-to-detect-if-im-compiling-code-with-visual-studio-2008
//	note: this is for the COMPILER not for the SDK (below)
#if defined(_MSC_VER) && _MSC_VER<=1800
#define OLD_VISUAL_STUDIO
#endif

//	when vs2013 is set to windows vista/7 SDK, it adds this define
#if defined(_USING_V110_SDK71_) && (WINVER >= _WIN32_WINNT_WIN7)
	#define WINDOWS_TARGET_SDK	7
#elif (WINVER >= _WIN32_WINNT_WIN8)
	#define WINDOWS_TARGET_SDK	8
#else
	#error Not sure which windows SDK we're building against.
#endif

//	see ofConstants
#define WIN32_LEAN_AND_MEAN

#define NOMINMAX
//http://stackoverflow.com/questions/1904635/warning-c4003-and-errors-c2589-and-c2059-on-x-stdnumeric-limitsintmax

#include <windows.h>
#include <process.h>
#include <vector>
#include <mmsystem.h>
#include <direct.h>
#pragma comment(lib,"winmm.lib")

//	get rid of some annoying windows macros
#undef CreateWindow


#define __func__		__FUNCTION__
#define __PRETTY_FUNCTION__		__FUNCTION__
#define __thread		__declspec( thread )
// Attribute to make function be exported from a plugin
#define __export		extern "C" __declspec(dllexport)
#define __pure
#define __unused		//	can't find a declpec for this :/s

//	gr: delspec's need to go BEFORE function declarations on windows... find a nice workaround that isn't __deprecated(int myfunc());
#define __noexcept		//	__declspec(nothrow)
#define __deprecated	//	__declspec(deprecated)
#define __noexcept_prefix	__declspec(nothrow)
#define __deprecated_prefix	__declspec(deprecated)


#include <math.h>
#include <stdint.h>

/*
typedef signed __int32		int32;
typedef unsigned __int32	uint32;
typedef signed __int64		int64;
typedef unsigned __int64	uint64;
*/
typedef SSIZE_T				ssize_t;

