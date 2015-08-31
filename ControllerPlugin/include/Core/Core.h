//////////////////////////////////////////////////////////////////////////
// 
// Header Copyright (c) 2013 Quantum Signal, LLC
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any
// damages arising from the use of this software.
//
// Permission is granted to anyone to use this script file for any
// purpose, including commercial applications, and to alter it and
// redistribute it freely, subject to the following restrictions:
// 
// 1. That the authors recognize that ANVEL is owned and licensed 
// by Quantum Signal, LLC, and respect ANVEL trademark and copyright.
// 
// 2. An acknowledgment of the ANVEL website (www.anvelsim.com) is included 
// in any source distribution or product documentation
// 
// 3. This notice may not be removed or altered from any source
// distribution.
//
// For more information, contact Quantum Signal, LLC at info@anvelsim.com
// 
//////////////////////////////////////////////////////////////////////////
#ifndef VANE_Core_h__
#define VANE_Core_h__

#include "VaneAssert.h"
#include "CoreTypes.h"

#include "Platform.h"
#include "VaneTypes.h"
#include "Logger.h"

// Omit these if building stand-alone plugin (e.g. MITVTIExtension - 
// flag is set by VaneVTIExtension project file when building in standalone
// mode
#ifndef MINIMAL_INCLUDES
#include "VaneMath.h"
#include "Singleton.h"
#include "SharedPtr.h"
#include "Collections.h"
#include "Version.h"
#endif
// OpenCollada breaks without this for some reason.
#include <iostream>

namespace VANE
{
	namespace Core
	{
		///Get the version number of this current release
		///@note, not really used quite yet
		int VaneCoreExport GetVersionNum();

		///Initialize what is necessary for the Core VANE 
		///items to work, such as the logger, info tracker, etc
		///@param pListener Optional log listener to use
		bool VaneCoreExport Initialize( LogListener* pListener );

		///Shutdown all things initialized by the Core and 
		///release all memory used
		void VaneCoreExport Shutdown();
	}
}

#ifndef NULL
#define NULL 0
#endif 

#define SafeDelete(x)		{ delete x; x = NULL; } 
#define SafeDeleteArr(x)	{ delete[] x; x = NULL; } 

#ifndef ARRAYSIZE
	#define ARRAYSIZE(x) ( sizeof(x) / sizeof( (x)[0]) )
#endif 

//If you get an error with this, make sure you have a semi colon at the end!
#define VANE_UNREFERENCED_PARAM(param) ((void)(param))

#if VANE_CONFIG_DEBUG && (VANE_COMPILER == VANE_COMPILER_MSVC)
	#define __VANE_USE_DEBUG_NEW 1
#endif

//Add this below to our standard header for debugging info on mem leaks
#if VANE_CONFIG_DEBUG
	#if _WIN32 && (VANE_COMPILER == VANE_COMPILER_MSVC)
		#define _CRTDBG_MAP_ALLOC
		#include <crtdbg.h>
		#define DEBUG_NEW new(_NORMAL_BLOCK ,__FILE__, __LINE__)
	#else
		#define DEBUG_NEW new 
	#endif
	
#else
	#define DEBUG_NEW new
#endif

//#if VANE_CONFIG_DEBUG && __VANE_USE_DEBUG_NEW
//	#define new DEBUG_NEW							    
//#endif

//Then add this below to any cpp file to get additional info on memory leaks
//#if VANE_CONFIG_DEBUG
//#define new DEBUG_NEW
//#endif


///Set this to 0 if you do not want profiling enabled
#define _VANE_USE_PROFILING 0

// this macro is useful for when a variable/parameter is used in a debug build
// but not in a release build, or vice-versa. Or when an inherited methods parameters are not
// used in a specific implementation
#define VANE_UNREFERENCED_PARAMETER( param ) ((void)param)
#define VANE_UNUSED(param) ((void)(param))

#endif // Core_h__

