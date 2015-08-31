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
// 2. An acknowledgement of the ANVEL website (www.anvelsim.com) is included 
// in any source distribution or product documentation
// 
// 3. This notice may not be removed or altered from any source
// distribution.
//
// For more information, contact Quantum Signal, LLC at info@anvelsim.com
// 
//////////////////////////////////////////////////////////////////////////


#ifndef Anvel_Sensor_Plugin_h__
#define Anvel_Sensor_Plugin_h__

#include "Core/Plugin.h"
#include "Simulation/Sensor.h"

#ifdef ANVEL_SENSOR_PLUGIN_EXPORT
#define SENSOR_API __declspec(dllexport)
#else
#define SENSOR_API __declspec(dllimport)
#endif

namespace VANE
{
	namespace Plugins
	{
		///Plugin for our sample sensor plugin
		class SampleSensorPlugin: public Plugins::Plugin
		{
		public: //[Plugin interface]

			///Called to initialize anything that this plugin needs
			///called by plugin host after it has been loaded
			void Initialize();

			///Called by plugin host when this plugin should be
			///shutdown
			void Shutdown();

			///Get the name of this plugin
			String GetName() const;

		private:
			//our vti model factory
			ISensorFactory* m_pSensorFactory;
		};
	}
}


#endif 