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
#ifndef VANE_Vehicles_VehicleManager_h__
#define VANE_Vehicles_VehicleManager_h__

#include "Core/Core.h"
#include "Core/Commands.h"
#include "Core/MetaTagManager.h"
#include "Core/Property.h"
#include "Core/XmlUtils.h"
#include "Core/Resources.h"
#include "Core/DataTypeManager.h"

#include "Simulation/SimulationCommon.h"

#include "VehicleManagerListener.h"
#include "Vehicle.h"
#include "VehicleObject.h"
#include "VehicleOptions.h"

namespace VANE
{
	//The types that this system defines
	namespace Types
	{				
		extern VaneSimExport VaneID Vehicle;
		extern VaneSimExport VaneID VehicleManager;	
		extern VaneSimExport VaneID ODEMotor;
	}

	namespace Resources
	{
		extern VaneSimExport FileTypeId kFileTypeVehicle;
	}

	namespace Commands
	{
		extern VaneSimExport const char* kVehicleCategory;

		extern VaneSimExport CommandID kCommandLoadVehicle;
		extern VaneSimExport CommandID kCommandReloadVehicle;
		extern VaneSimExport CommandID kCommandSetVehiclePosition;
		extern VaneSimExport CommandID kCommandResetVehicle;
		extern VaneSimExport CommandID kCommandRemoveVehicle;
		extern VaneSimExport CommandID kCommandRemoveAllVehicles;
		extern VaneSimExport CommandID kCommandHaltVehicle;
		extern VaneSimExport CommandID kCommandSetVehicleOnGround;
		extern VaneSimExport CommandID kCommandGetVehicleIDs;
		extern VaneSimExport CommandID kCommandGetVehicleState;
		extern VaneSimExport CommandID kCommandUseVehicleView;
		extern VaneSimExport CommandID kCommandFollowPath;
		extern VaneSimExport CommandID kCommandSeekPoint;
		extern VaneSimExport CommandID kCommandMakeHumanControlled;
		extern VaneSimExport CommandID kCommandSetVehicleController;
		extern VaneSimExport CommandID kCommandAttachCamToVehicle;
		extern VaneSimExport CommandID kCommandGetPointCloudData;
	}

	namespace Vehicles
	{
		class MotorControllerFactory;
		class MotorController;
		class DifferentialMotorControllerFactory;
		class DifferentialMotorController;
		class CustomMotorController;
		class ODEStrut;
		class ODEMotor;

		///Vehicle Shared Pointer
		typedef VANE::SharedPtr<VANE::Vehicles::Vehicle> VehiclePtr;

		// A "Weak" pointer is one where ownership is not applied
		typedef Vehicles::MotorSpeedControllerFactory* SpeedControllerMgrWeakPtr;

		///Pair of Vehicle Pointer and VehicleOptions, so that we can 
		///always know what options were around for this vehicles creation
		///so that we can reset it to that position
		typedef std::pair<VehiclePtr, VehicleOptions> VehiclePairing;
		
		///A container for holding vehicles
		typedef std::deque<VehiclePairing> VehicleCollection;

		typedef VANE::Vehicles::VehicleCollection::const_iterator VehicleCollectionConstIterator;

		typedef std::map<VaneID, MotorControllerFactory*> MotorControllerFactoryIDMap;

		/************************************************************************/
		/* Assets and Asset Support                                             */
		/************************************************************************/

		struct VehicleStaticAssetParams
			: public StaticAssetParams
		{
			String m_vehicleFilename;
			String m_physicsFilename;
			String m_controllerType;
			bool m_placeOnGround;

			VehicleStaticAssetParams() : m_placeOnGround(true) {;}
		};

		/************************************************************************/
		/* Vehicle Manager                                                      */
		/************************************************************************/

		///Our manager of all things vehicle
		class VaneSimExport Manager : 
			public Singleton<VANE::Vehicles::Manager>,
			public IPropertyProvider,
			public IXmlInterface,
			public IAssetHandler
		{
		private:
			Manager();
			~Manager();

		public:
			///Initialize the vehicle manager
			static void Initialize();

			///Shutdown the vehicle manager
			static void Shutdown();

			//for use with dlls, we need to override template behavior, otherwise
			//linking errors
			static Manager* GetSingletonPtr();
			static Manager& GetSingleton();

			static const uint32 kMaxVehicles = 512;

			///Get the next ObjectID (used during object creation)
			VehicleID GetNextVehicleID();

			//Get a pointer to the first vehicle
			Vehicle* GetVehicle();

			///get a vehicle by its vehicleID
			Vehicle* GetVehicle(VehicleID id);
			
			///Get vehicle by name
			Vehicle* GetVehicle(const String&  name);

			///Get a listing of all vehicle IDs
			///@return vector of all the local vehicle IDs
			std::vector<VehicleID> GetVehicleIDs();

			/// Get the vehicle names we are using
			///@return Vector of vehicle names local to this instance of Vane
			StringVector GetVehicleNames();

			/// Build an id set for vehicles using their IDs, names, and an invalid ID.
			/// @return The vane ID set for the current vehicles.
			VaneIdSet GetVehicleIdSet(bool includeMetaTags = false) const;

			///Get a data provider for the current vehicle
			VehicleDataProvider* GetVehicleDataProvider();
			VehicleDataProvider* GetVehicleDataProvider(VehicleID id);

			///Halt the given vehicle
			void HaltVehicle(VehicleID vehicle);

			///Set this vehicle on the ground.
			void PlaceVehicleOnGround(VehicleID vehicle);

			///Reset the location of a vehicle without reloading the vehicle
			///@param vehicle - the local vehicle ID
			void ResetVehicleLocation(VehicleID vehicle);

			///Destroy a vehicle from the world
			///This VehicleID will no longer be valid 
			///@param[in] the ID of the vehicle that will be destroyed
			void RemoveVehicle(VehicleID vehicle);

			///Reset the vehicle to its original locations
			///@return the new vehicle after it has been reloaded
			Vehicle* ResetVehicle(VehicleID id);
			Vehicle* ResetVehicle(const VANE::Vector3 & position, const VANE::Quaternion & orientation);

			///notify all listeners (type IVehicleManagerListener)
			/// that vehicle has been modified
			///@param[in] Vehicle ID
			void VehicleModified(VehicleID id);

			///Reloads a vehicle from the file it was initially loaded from.
			///Essentially combines removing and loading again.
			VehicleID ReloadVehicle(VehicleID id);

			///Create a vehicle and add it to the container
			Vehicle* CreateVehicle(const VehicleOptions& options);


			/// Add a new vehicle to the manager
			///@param options The vehicle parameters
			VehicleID AddNewVehicle(const VehicleOptions & options);

			///Add a vehicle that will not get controller by VANE but will still get 
			///displayed (for externally controlled vehicles running in other instances
			///or simulations
			VehicleID AddExternalVehicle(const VehicleOptions &options);
			
			///Get the vehicle for this ID, external vehicles only
			Vehicle* GetExternalVehicle(VehicleID id);

			///Get the Ids of all external vehicles
			std::vector<VehicleID> GetExternalVehicleIDs() const;

			///Update all external vehicles 
			///This may not be necessary
			void UpdateExternalVehicles(TimeValue dt);

			///Update a specific external vehicle with a new position
			///Called by the external source, not our  own instance
			void UpdateExternalVehicle(VehicleID id, const VANE::Vector3 & position, float direction, TimeValue time);

			///Remove an externally controller vehicle from our simulation
			void RemoveExternalVehicle(VehicleID id);

			///remove all vehicles
			void ClearVehicles();

			//Update before the physics updates takes place. This resets states
			//for wheels, etc.
			void PreSimUpdate();
			
			///Update the vehicle manager and all of its vehicles, called during
			///inner physics loop
			void Update(TimeValue dt);
			
			///Do all non physics updates after physics has simulated a step,
			///such as tracks and renderable data
			void PostSimUpdate( TimeValue dt );

			void ResetTimer() {m_update = true;}

			void PlaceVehicleOnGround(VehicleID id, float64 direction, float64 offset);
			void PlaceVehicleOnGround(Vehicle* vehicle, float64 offset);
			
			///Get point cloud data from the sensors on this vehicle.
			///This will only include points that could be considered obstacles
			///and only with a range near the vehicle. 
			///@todo clean this up for more useful behavior.
			std::vector<Vector3> GetFilteredPointCloudData( VehicleID vehicle );

			///Get a const iterator so we can loop through the vehicles externally
			///@return A const iterator to the beginning of the vehicles
			VehicleCollectionConstIterator GetVehicleIteratorBegin();

			///The const_iterator for the end of the object container structure
			///Used with GetVehicleIteratorEnd() to provide the end of the loop
			VehicleCollectionConstIterator GetVehicleIteratorEnd();

			void InterruptTracks(const VaneIdVector& vehicleIds);

			//------------Visualization --------------

			///If vehicles should be displayed
			void SetVehicleVisualizationEnabled(bool enabled); 
			///Are vehicles being displayed?
			bool IsVehicleVisualizationEnabled() { return m_visualization;}

			//------------Listeners ------------------------
			///A a listener
			void AddListener(IVehicleManagerListener* listener);
			///Remove a listener
			void RemoveListener(IVehicleManagerListener* listener);
			///Clear all listeners
			void ClearListeners();
			

			//-------------Motor Controllers--------------
			void RegisterMotorControllerFactory( MotorControllerFactory* pFactory );
			MotorController* CreateMotorController( const TiXmlElement* pElement, Vehicle* pVehicle );
			MotorControllerFactory* GetMotorControllerFactory( String factoryType );

			VaneID GetNextOdeMotorID() { return m_odeMotorID++; }
			///Get a pointer to a motor by its id
			Motor* GetMotor( VaneID motorID );

			void RegisterMotor( Motor& motor );
			void UnregisterMotor( MotorID motor );


		public:
			const MotorControllerFactoryIDMap& GetMotorControllerFactories() const { return m_motorControllerFactoryIDMap;};

		public: //[IXmlSerializable]
			virtual void  WriteXmlToNode(TiXmlNode* pParent);
			virtual bool  ReadXmlFromNode(const TiXmlNode* pStateElement);

		public: //[IPropertyProvider methods]
			virtual void OnPropertyChanged(VaneID objId, PropertyIndex id);
			virtual ObjectPropertySet GetProperties(VaneID objID);
			virtual VaneIdVector GetIdsOfType(const VaneID dataType) const;

		public: //[IAssetHandler]
			virtual VaneID CreateAssetInstance(const StaticAssetParamsPtr& pStaticParams, VaneID dataType, const DynamicAssetParamsPtr& pDynParams);
			virtual void DestroyAssetInstance(VaneID objId);
			virtual StaticAssetParamsPtr ParseXmlToBinary(const TiXmlElement* pXmlParams, VaneID dataType);
			virtual VaneIdVector GetHandledDataType() const;
			virtual VaneID GetMainWorldObjectId(VaneID id);
			virtual VaneID GetInstanceUsingMainWorldObject(VaneID id);
			virtual void FinalizeAssetInstance( VaneID instanceId, const StaticAssetParamsPtr& pStaticParams, const DynamicAssetParamsPtr& pDynParams );



			/// Because of dependencies, assets must wait to be initialized. This function
			/// should be called by the hosts loading function to initialize assets. Otherwise
			/// the ResourceManager will not have loaded all the resource directories from the
			/// VaneInit file and no assets will be found.
			void InitAssets();


		protected:
			static String CreatePreviewAsset( const Resources::RegisteredFile& vehicleFile);

		protected:

			PropertyGroupInstance GetVehicleManagerProperties();
			void                  RegisterVehicleManagerProperties();
			PropertyGroupInstance GetVehicleProperties(Vehicle& vehicle);
			void                  RegisterVehicleProperties();

			void				  RegisterElectricMotorProperties();
			PropertyGroupInstance GetElectricMotorProperties( Vehicles::ElectricMotor& odeMotor);

			void				  RegisterODEMotorProperties();
			PropertyGroupInstance GetODEMotorProperties( Vehicles::ODEMotor& odeMotor);

			void					RegisterElectricMotorPrototypeProperties();
			PropertyGroupInstance  GetElectricMotorPrototypeProperties( Vehicles::DCElectricMotorPrototype& motorPrototype);


			void UnregisterMotorSpeedControllerTypeMgr( SpeedControllerMgrWeakPtr  );


			void                  RegisterPIDSpeedControllerPrototypeProperties();
			PropertyGroupInstance GetPIDSpeedControllerPrototypeProperties(Vehicles::PIDSpeedControllerFactory& speedControllerMgr);
			void                  RegisterPIDSpeedControllerInstanceProperties();
			PropertyGroupInstance GetPIDSpeedControllerInstanceProperties(Vehicles::PIDSpeedController& speedControllerMgr);

			void                  RegisterOpenLoopControllerPrototypeProperties();
			PropertyGroupInstance GetOpenLoopControllerPrototypeProperties(Vehicles::OpenLoopSpeedControllerFactory& speedControllerMgr);
			void                  RegisterOpenLoopControllerInstanceProperties();
			PropertyGroupInstance GetOpenLoopControllerInstanceProperties(Vehicles::MotorSpeedController& speedControllerMgr);

			void RegisterDifferentialMotorControllerProperties();
			PropertyGroupInstance GetDifferentialMotorControllerProperties( Vehicles::DifferentialMotorController& differentialMotorController);

			void RegisterCustomMotorControllerProperties();
			PropertyGroupInstance GetCustomMotorControllerProperties( Vehicles::CustomMotorController& RMPMotorController);

			PropertyGroupInstance GetBatterySystemProperties( Vehicles::BatterySystem& batterySystem);
			void				  RegisterBatterySystemProperties();

			PropertyGroupInstance GetDifferentialMotorControllerFactoryProperties( Vehicles::DifferentialMotorControllerFactory& motorControllerFactory);
			void RegisterDifferentialMotorControllerFactory();

			void RegisterODEStrutTemplateProperties();
			PropertyGroupInstance GetODEStrutTemplateProperties( Vehicles::ODEStrut& rStrut);
			void RegisterODEStrutInstanceProperties();
			PropertyGroupInstance GetODEStrutInstanceProperties( Vehicles::ODEStrut& rStrut);

			Vehicle* CreateVehicleInternal(const VehicleOptions& options);
			Vehicle* CreateVehicleInternal(const String& fileName, const VANE::Vector3 &position = VANE::Vector3(0,0,0), const VANE::Quaternion& orientation = VANE::Quaternion::GetIdentity(), bool placeOnGround = true, const String& instanceName = "");

			void LoadVehiclePhysicsState(const String& physicsFilename, Vehicle* target);
			//Utility to set the command enabled/disabled state all at once...
			void SetVehicleCommandsEnabled( bool enabled );

			
//@remove
//temporary			
uint32 m_numPointCloudPoints;			
			
		protected:

			//For RTI stuff to see if the list has been updated recently
			double	m_lastUpdate;

			VaneID  m_vehicleIDCount;
			VaneID  m_odeMotorID;

			///our vehicle container for OWNED vehicles
			VehicleCollection m_vehicles;

			///our vehicle collection for externally created vehicles
			///we cannot control them, only update them
			VehicleCollection m_externalVehicles;

			///Our listeners
			std::vector<IVehicleManagerListener*> m_listeners;

			typedef std::map<String, MotorControllerFactory*> MotorControllerFactoryMap;
			typedef std::pair<String, MotorControllerFactory*> MotorControllerFactoryMapEntry;
			MotorControllerFactoryMap m_motorControllerFactories;

			MotorControllerFactoryIDMap m_motorControllerFactoryIDMap;

			typedef std::map< MotorID, Motor*> MotorMap;
			MotorMap m_registeredMotors;
			
			bool m_update;
			///If vehicles should be displayed currently
			bool m_visualization;

			///Our specialized CommandGroup for working with VehicleMgr functions
			///this will not need a public interface so it is declared as an internal
			///class.  This handles all of the command management for the VehicleManager
			class VehicleCommandGroup : public CommandGroup
			{
			public:
				///Create a vehicle command group with a given vehicle mgr
				///NOTE: Even though VehicleManager is a singleton, we still want
				///to verify that it actually has been created, and we want to have
				///the option to use it without requiring it to be a Singleton
				///@param[in] mgr The manager to use
				VehicleCommandGroup(Vehicles::Manager& mgr);

				///Handle commands specific to the vehicle manager and the vehicle system
				///@param[in] commandID The ID of the command to handle
				///@param[in] parameterList The parameters to use for this command
				///@return A CommandResult describing the success or failure of this command
				CommandResult HandleCommand( CommandID commandID, const CommandParamList& parameterList);
			
				/// Get a list of possible values for each parameter for the specified command.
				/// NOTE: Not all commands or parameters may have choices and can either be left blank
				/// or assigned a value of Variant::INVALID.
				/// @param[in] commandID The command to get possible parameter choices for.
				CommandParamList GetParameterChoices( CommandID commandID );

			protected:
				//our vehicle manager reference
				Vehicles::Manager& m_mgr;
			};

			//
			VehicleCommandGroup m_commandGroup;
		};
	}
}


#endif // VehicleManager_h__
