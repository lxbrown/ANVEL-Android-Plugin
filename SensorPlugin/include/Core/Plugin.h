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
#ifndef VANE_Plugin_h__
#define VANE_Plugin_h__

#include "Platform.h"
#include "Core.h"

#include <string>
#include <vector>

namespace VANE
{
	///Encapsulates all of our Plugin system logic
	namespace Plugins
	{
		class VaneCoreExport Plugin;

		///Simple structure to get basic info from a plugin before actually loading it
		///@note Not currently used, but this should be returned by a c style function
		///that will be called along the same lines as the load plugin function
		struct PluginInfo
		{
			uint16 versionMajor;
			uint16 versionMinor;
			uint32 versionBuild;
			char pluginName[64];
			char pluginSummary[64];
		};

		///Lightweight interface to a generic plugin host.  This
		///allows plugins to know virtually nothing about VANE
		class PluginHost
		{
		public:
			
			///Register a plugin with the Plugin host.  This should be called from
			///inside a plugin dll in the extern "C" void RegisterPlugin function required to be a plugin
			///@param pPlugin - an instance of a plugin 
			virtual void RegisterPlugin( Plugins::Plugin* pPlugin )=0;
			
			///Unregister a plugin with the plugin host.  This also should be called from the dll
			///in the extern "C" void UnregisterPlugin function that is also required.
			///@param[in] name Name of the plugin to unregister
			///@param[in] pPlugin
			virtual void UnregisterPlugin( const String& name, Plugins::Plugin* pPlugin)=0;
		};

		///Our plugin base class
		class VaneCoreExport Plugin
		{
		public:
			///Standard plugin constructor
			Plugin() {;}

			virtual ~Plugin() {;}

			///Get this plugin's name
			///@return name The unique name for this plugin type
			virtual String GetName() const=0;

			///Perform all plugin initialization
			virtual void Initialize()=0;

			///Shutdown this plugin and destroy all resources it has
			virtual void Shutdown()=0;
		};
	}
}

#endif // Plugin_h__
