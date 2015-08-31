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


#include "SensorPlugin.h"

#include "SampleSensor.h"

using namespace VANE;
using namespace VANE::Plugins;


//////////////////////////////////////////////////////////////////////////
//         Plugin DLL Code    
//////////////////////////////////////////////////////////////////////////

const char* kPluginName = "SensorPlugin";

//////////////////////////////////////////////////////////////////////////

void SampleSensorPlugin::Initialize()
{
	m_pSensorFactory = new SampleSensorFactory();
}

//////////////////////////////////////////////////////////////////////////

String SampleSensorPlugin::GetName() const
{
	return String(kPluginName);
}

//////////////////////////////////////////////////////////////////////////

void SampleSensorPlugin::Shutdown()
{
	delete m_pSensorFactory;
}

//////////////////////////////////////////////////////////////////////////
//
// Plugin Export Code
//
//////////////////////////////////////////////////////////////////////////

SampleSensorPlugin* pPlugin;

extern "C" SENSOR_API void RegisterPlugin(VANE::Plugins::PluginHost* host) 
{
	pPlugin = new SampleSensorPlugin();
	host->RegisterPlugin( pPlugin );
}

extern "C" SENSOR_API void UnregisterPlugin(VANE::Plugins::PluginHost* host) 
{
	host->UnregisterPlugin(kPluginName, pPlugin);
	delete pPlugin;
}

