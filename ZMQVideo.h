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

#ifndef ZMQVideo_h__
#define ZMQVideo_h__

#include <fstream>
#include <math.h>
#include <algorithm>

#include "Core/PropertyManager.h"
#include "Core/Property.h"
#include "Core/ControllerInterface.h"
#include "Core/Core.h"
#include "Core/Commands.h"
#include "Simulation\Renderer.h"
#include "Simulation\RendererManager.h"
#include "Simulation\CameraSensor.h"

namespace VANE
{
	//////////////////////////////////////////////////////////////////////////
	// Forward Declares
	
	namespace Vehicles { class Vehicle; }
	
	//////////////////////////////////////////////////////////////////////////
	// Types

	namespace Types
	{
		extern VaneID ZMQVideo;
	}

	//////////////////////////////////////////////////////////////////////////
	// Commands

	namespace Commands
	{
		extern CommandID kCommandUseZMQVideo;
	}

	//////////////////////////////////////////////////////////////////////////
	// ZMQVideo 

	namespace Controller
	{
		extern const String kZMQVideoName;

		class ZMQVideoFactory
			: public IControllerFactory, IPropertyProvider
		{
		public:
			ZMQVideoFactory();
			~ZMQVideoFactory();

		public: //[ControllerFactory methods]

			virtual ControllerID CreateControllerInterface();
			virtual ControllerType GetControllerType() const { return kZMQVideoName; }
			virtual ControllerID GetControllerTypeId() const { return Types::ZMQVideo; }
			virtual void SerializeControllers(TiXmlElement *) const {}
			virtual void DeserializeControllers(const TiXmlElement *) {}
			virtual void DestroyControllerInterface(ControllerID id);
			virtual ControllerPtr GetController(ControllerID id);

		public: //[IPropertyProvider methods]
			virtual void OnPropertyChanged(VaneID objId, PropertyIndex index);
			virtual ObjectPropertySet GetProperties(VaneID objID);
			virtual VaneIdVector GetIdsOfType(const VaneID dataType) const;
		private:
			void RegisterZMQVideoProperties();

		private:
			ControllerID m_currentID;

			//Internal class to handle command management
			class ZMQVideoCommandGroup : public CommandGroup
			{
			public:
				ZMQVideoCommandGroup(ZMQVideoFactory& factory);
				CommandResult HandleCommand( CommandID commandID, const CommandParamList& parameterList);
				//CommandParamList GetParameterChoices( CommandID commandID );

			protected:
				ZMQVideoFactory& m_factory;
			};

			ControllerPtrMap m_ownedControllers;

			ZMQVideoCommandGroup m_commands;
		};

		/************************************************************************/
		/*  ZMQVideo                                                  */
		/************************************************************************/
		
		class ZMQVideo : public ControllerInterface
		{
			static const String kTypeZMQVideo;

			friend class ZMQVideoFactory;
		public:
			~ZMQVideo();

			static ControllerType GetControllerType() { return kTypeZMQVideo; }

		public: // [ControllerInterface methods]
			virtual void Update(TimeValue dt);
			virtual ControlValue GetInput(ControlInputIndex index) const;
			virtual ControlValue GetInput(const ControlName& name) const;
			virtual std::vector<ControlValue> GetInputs() const;
			virtual void ClearInputs();
			virtual std::vector<ControlInput> GetControls() const;
			virtual void SetControlMode(ControlMode mode);
			virtual void AddControllable( VaneID controllableID );
			virtual inline ControllerType GetType() const {return GetControllerType();}
			virtual inline ControllerID GetControllerID() const {return m_id;}
			virtual inline ControllerID GetBaseControllerID() const { return m_baseID; }
			virtual void OnAttachedToObject( VaneID objectID );
			
		protected:

			ZMQVideo();
			void CalculateControlValues( TimeValue dt );
			
		protected:
		
			VaneID m_controllableID;

			/// This controller's typed ID
			ControllerID m_baseID;
			ControllerID m_id;

			std::vector<ControlValue> m_inputValues;
			std::vector<ControlInput> m_inputs;	

			TimeValue m_elapsedTime;
			TimeValue m_lastCalculation;

			String ipaddr;
			double m_desired_speed;
			double m_desired_yaw;
			double m_actual_speed;
			double m_actual_yaw;
			double m_throttle;
			double m_steering;

			int frame;
			bool running;
		};
	}
}
#endif