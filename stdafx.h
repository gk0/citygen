// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once 

#ifdef WIN32
	// Modify the following defines if you have to target a platform prior to the ones specified below.
	// Refer to MSDN for the latest info on corresponding values for different platforms.
	#ifndef WINVER				// Allow use of features specific to Windows XP or later.
	#define WINVER 0x0501		// Change this to the appropriate value to target other versions of Windows.
	#endif
	
	#ifndef _WIN32_WINNT		// Allow use of features specific to Windows XP or later.                   
	#define _WIN32_WINNT 0x0501	// Change this to the appropriate value to target other versions of Windows.
	#endif						
	
	#ifndef _WIN32_WINDOWS		// Allow use of features specific to Windows 98 or later.
	#define _WIN32_WINDOWS 0x0410 // Change this to the appropriate value to target Windows Me or later.
	#endif

	#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
	
	// Windows Header Files:
	#include <windows.h>
#else
	#define USE_XPM_BITMAPS 1
#endif

// Visual Leak Detector
// http://dmoulding.googlepages.com/vld
// helps find memory leaks
//#define DLLCODE
//#include <vld.h>

#ifdef WIN32
#include <tchar.h>
#endif

// BOOST Includes
#include <boost/foreach.hpp>

// wxString conversion macros
#include <wx/string.h>
#include <wx/stattext.h>

#if wxUSE_UNICODE
	#define _UU(x,y) wxString((x),(y))
	#define _CC(x,y) (x).mb_str((y))
#else
	#define _UU(x,y) (x)
	#define _CC(x,y) (x)
#endif

#define _U(x) _UU((x),wxConvUTF8)
#define _C(x) _CC((x),wxConvUTF8)
