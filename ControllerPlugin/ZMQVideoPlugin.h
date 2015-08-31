#ifndef ANVEL_SAMPLE_CONTROLLER_PLUGIN_h
#define ANVEL_SAMPLE_CONTROLLER_PLUGIN_h

#include "Core/Plugin.h"
#include "Core/ControllerInterface.h"

//if we are exporting this to a dll (ie. Building the plugin)
//we should have SAMPLE_CONTROLLER_PLUGIN_EXPORT defined in the compiler flags
#ifdef ZMQ_VIDEO_PLUGIN_EXPORT
#define ZMQ_VIDEO_API __declspec(dllexport)
#else
#define ZMQ_VIDEO_API __declspec(dllimport)
#endif

namespace VANE
{
	namespace Controller { class ControllerFactory; }

	namespace Plugins
	{
		class ZMQVideoPlugin : public Plugins::Plugin
		{
			public: //[Plugin interface]
				//Called to initialize anything that this plugin needs after it has been loaded
				void Initialize();

				//Called by plugin host when this plugin should be shutdown
				void Shutdown();

				//Get the name of this plugin
				String GetName() const;

			private:
				//The factory that will create instances of our controller
				Controller::IControllerFactory* m_pControllerFactory;
		};
	}
}

#endif 

