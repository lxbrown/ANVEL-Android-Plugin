#include "ZMQVideoPlugin.h"
#include "ZMQVideo.h"

//... Other includes here...

using namespace VANE;
using namespace VANE::Plugins;

//////////////////////////////////////////////////////////////////////////
//         Plugin DLL Code    
//////////////////////////////////////////////////////////////////////////

const char* kPluginName = "ZMQVideoPlugin";

//////////////////////////////////////////////////////////////////////////

void ZMQVideoPlugin::Initialize()
{
	// Register Factories, Data types, etc., here
	m_pControllerFactory = new Controller::ZMQVideoFactory();
}

//////////////////////////////////////////////////////////////////////////

void ZMQVideoPlugin::Shutdown()
{
	//Do plugin shutdown work here
	delete m_pControllerFactory;
}

//////////////////////////////////////////////////////////////////////////

String ZMQVideoPlugin::GetName() const
{
	return String(kPluginName);
}

//////////////////////////////////////////////////////////////////////////
//
// Plugin Export Code
//
//////////////////////////////////////////////////////////////////////////

ZMQVideoPlugin* pPlugin;

extern "C" ZMQ_VIDEO_API void RegisterPlugin( VANE::Plugins::PluginHost* host) 
{
	pPlugin = new ZMQVideoPlugin();
	host->RegisterPlugin( pPlugin );
}

extern "C" ZMQ_VIDEO_API void UnregisterPlugin( VANE::Plugins::PluginHost* host) 
{
	host->UnregisterPlugin(kPluginName, pPlugin);
	delete pPlugin;
}