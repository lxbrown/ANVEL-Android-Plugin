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
#ifndef ControllerManager_h__
#define ControllerManager_h__

#include "Core/Core.h"
#include "Core/ControllerInterface.h"

#include "Simulation/SimulationCommon.h"
#include "Simulation/Vehicles/VehicleManagerListener.h"
#include "Simulation/Vehicles/Vehicle.h"

namespace VANE
{
	namespace Types
	{
		extern VaneSimExport VaneID ControllerMgr;
		extern VaneSimExport VaneID ControllerInterface;
	}
	
	namespace Controller
	{
		class SimpleANSController;
		class SimpleAIController;
		class PointAIController;
		
		typedef std::pair<VANE::Vehicles::VehicleID, ControllerPtr> VehicleControllerMapEntry;
		typedef std::map<VANE::Vehicles::VehicleID, ControllerPtr> VehicleControllerMap;

		typedef std::map<VaneID, IControllerFactory*> ControllerFactoryMap;
		
		//////////////////////////////////////////////////////////////////////////
		// ControllerManagerListener
		class IControllerManagerListener
		{
		public:
			virtual void OnControllerManagerUpdated()=0;
		};
		
		//////////////////////////////////////////////////////////////////////////
		//
		//  Controller Manager
		//
		//////////////////////////////////////////////////////////////////////////

		///Manager for our controller interfaces.  
		///Controllers are registered with the manager, and are also associated
		///with vehicles.
		///To create a new controller, you should call CreateControllerOfType with the
		///name of the type you wish to create, ie "HumanController".
		class VaneSimExport Manager : 
			public VANE::Singleton<VANE::Controller::Manager>,
			public Vehicles::IVehicleManagerListener,
			public IPropertyProvider,
			public IXmlInterface
		{
		public:
			///Create a manager
			Manager();
			~Manager();
			static Manager* GetSingletonPtr ();
			static Manager& GetSingleton ();

			///Update the controllers with the given time delta
			void Update(TimeValue dt);

			/// Set the controller interface for a given vehicle id
			///@param id The vehicle ID of the vehicle we will be controlling
			///@param controller The new controller interface we will be owning (TAKES OWNERSHIP!)
			void SetVehicleController(VANE::Vehicles::VehicleID id, const ControllerPtr& controller);

			/// Get the controller for a certain vehicle
			///@param id
			///@return The controller interface for this vehicle, or NULL if none exist
			ControllerPtr GetVehicleController(Vehicles::VehicleID id);

			///Get a controller by its controller ID
			ControllerPtr GetController(ControllerID id);

			///Get a vector of controller IDs
			std::vector<ControllerID> GetControllerIDs();

			///This should be called in the constructor of all controllers, or at least
			///before it is first used.  It returns the ID that this controller should use.
			void RegisterController(const ControllerPtr& pController);
			
			//Unregister a previously registered controller. 
			//This removes it from any vehicles that use this controller as well.
			//Note that the factory may still contain a reference to the controller,
			//keeping it alive until DestroyController is called.
			void UnregisterController( ControllerID id );

			///Get the next Controller ID to use. Should only be called
			///by Factories creating new controllers.
			ControllerID GetNextControllerID() { return m_currentControllerID++; }

			///Add a listener that will receive updates when the controller
			///manager has a controller created, deleted, or assigned to a vehicle
			void AddListener( IControllerManagerListener* listener );
			
			///Remove a listener
			void RemoveListener( IControllerManagerListener* listener );

		public: //Factory related functions
		
			///Main method to create a Controller. 
			///@param[in] typeName The name of the controller type to create
			ControllerID CreateControllerOfType(const String& typeName);

			//Someday soon, upgrade property inheritance features and use that here.
			///Notify the manager that a factory is available
			void RegisterControllerFactory(IControllerFactory* pFac);
			///Notify the manager that a factory is becoming unavailable;
			/// This causes the controller to be removed from any managed lists.
			void UnregisterControllerFactory(IControllerFactory* pFac);
			
			///Set a type to be used for a new vehicles default controls.
			///@param[in] typeName The controller type name that will be used 
			///to create a new controller for a newly created vehicle
			void SetNewVehicleDefaultControllerType( const String& typeName );

			///So that clients can do factory specific stuff without making it a
			///singleton.
			IControllerFactory* GetControllerFactoryOfType(const String& typeName);

			//This ONLY destroys the controller in the factory, and not in any 
			//internal references
			void DestroyController(ControllerID controllerId);

			// Set default tracking speed to be about 80% of actual top speed
			static const Real m_kRelativePathFollowingSpeed;

		public: //[IVehicleManagerListener ]
			virtual void OnNewVehicle(Vehicles::VehicleID id);
			virtual void OnVehicleDestroyed(Vehicles::VehicleID id);
			
		public: //[IXmlInterface]
			virtual bool ReadXmlFromNode( const TiXmlNode* pElement );
			virtual void WriteXmlToNode( TiXmlNode* pParentElement );
			
		public: //[IPropertyProvider methods]
			virtual ObjectPropertySet GetProperties(VaneID objID);
			virtual VaneIdVector GetIdsOfType(const VaneID dataType) const;

		private:
			PropertyGroupInstance GetControllerMgrProperties();
			void RegisterControllerMgrProperties();
			PropertyGroupInstance GetSimpleAnsCtlrProperties(SimpleANSController& controller);
			void RegisterSimpleAnsCtlrProperties();
			PropertyGroupInstance GetSimpleAICtlrProperties(SimpleAIController& controller);
			void RegisterSimpleAICtlrProperties();
			PropertyGroupInstance GetPointAICtrlProperties(PointAIController& controller);
			void RegisterPointAICtlrProperties();

			void ClearControllers();

		private:
			
			std::vector<ControllerPtr> m_registeredControllers;

			ControllerFactoryMap m_registeredFactories;
		
			///Our vehicle controllers
			VehicleControllerMap m_vehicleControllers;
			
			//The parties interested in receiving updates from
			//the controller manager.
			std::vector<IControllerManagerListener*> m_listeners;

			ControllerID m_currentControllerID;
			
			String m_newVehicleDefaultControllerType;
		};
	}
}
#endif // ControllerManager_h__
