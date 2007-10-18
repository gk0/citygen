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
	
	#ifndef _WIN32_IE			// Allow use of features specific to IE 6.0 or later.
	#define _WIN32_IE 0x0600	// Change this to the appropriate value to target other versions of IE.
	#endif
	
	#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
	
	// Windows Header Files:
	#include <windows.h>

	#ifdef __MINGW_H
		#define USE_XPM_BITMAPS 1
	
		//STL Port stuff,  don't think i need it anymore
		#define __GNUWIN32__
		#ifdef DEBUG
		  #define _STLP_DEBUG
		 #endif 
		 //add cflags -mthreads
		#define _STLP_NO_CUSTOM_IO 
		#define _STLP_USE_DYNAMIC_LIB
	#endif
#else
	#define USE_XPM_BITMAPS 1
#endif

// Visual Leak Detector
// http://dmoulding.googlepages.com/vld
// helps find memory leaks
//#define DLLCODE
//#include <vld.h>

// C RunTime Header Files
#include <stdlib.h>  
#include <malloc.h>
#include <memory.h>

#ifdef WIN32
#include <tchar.h>
#endif

// C++ Headers
#include <iostream>
#include <sstream>
#include <vector>
#include <list>

//BOOST Includes
#include <boost/foreach.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/depth_first_search.hpp>
#include <boost/graph/visitors.hpp>

//OGRE includes
#include <Ogre.h>
//include order, umm doesn't really work
//http://www.ogre3d.org/phpBB2/viewtopic.php?p=30510&sid=e9e02f7efef7818408020b90d775904e
#include <OgreNoMemoryMacros.h>

// TODO: reference additional headers your program requires here
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
	#include <wx/wx.h>
#endif
#include <wx/frame.h>
#include <wx/image.h>
#include <wx/log.h>
#include <wx/msgdlg.h>
#include <wx/textctrl.h>
#include <wx/timer.h>
#include <wx/toolbar.h>
#include <wx/propgrid/manager.h>
#include <wx/propgrid/advprops.h>
#include <wx/aui/aui.h>
#include <wx/datetime.h>

// wxString
#if wxUSE_UNICODE
	#define _UU(x,y) wxString((x),(y))
	#define _CC(x,y) (x).mb_str((y))
#else
	#define _UU(x,y) (x)
	#define _CC(x,y) (x)
#endif

#define _U(x) _UU((x),wxConvUTF8)
#define _C(x) _CC((x),wxConvUTF8)


//TinyXML Includes
#include <tinyxml.h>
#include <OgreMemoryMacros.h>
