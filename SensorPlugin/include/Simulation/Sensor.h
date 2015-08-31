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
#ifndef _QS_VANE_SENSOR_
#define _QS_VANE_SENSOR_

#include "Core/VaneIDs.h"
#include "Core/DataTypeManager.h"
#include "Core/Units_MKS.h"
#include "Core/Property.h"
#include "Core/Timer.h"

#include "SimulationCommon.h"

namespace VANE
{
	//The types that this system defines
	namespace Types
	{				
		extern VaneSimExport VaneID SensorManager;
		extern VaneSimExport VaneID Sensor;
	}

	namespace Commands
	{
		extern VaneSimExport const char* kSensorCategory;

		extern VaneSimExport CommandID kCommandDestroySensor;
		extern VaneSimExport CommandID kCommandDetachSelectedSensor;
		extern VaneSimExport CommandID kCommandAttachSelectedSensorByLocation;
		extern VaneSimExport CommandID kCommandDetachSensor;
		extern VaneSimExport CommandID kCommandAttachSensorByLocation;
	}
	
	namespace World
	{
		class Object;
	}

	const String kSensorFileNodeObject = "sensor";

	typedef String SensorType;
	class Sensor;
	
	///The asset params associated with sensors.
	/// Some are type specific, and some of the base
	/// ones are unused.
	class SensorStaticAssetParams
		: public StaticAssetParams
	{
	public:
		///The type of world object to create for use with this sensor
		String m_worldObjectAssetName;

		///The type of sensor we represent
		/// (also determines which parameters are actually used)
		SensorType m_sensorType;

		///The sample rate for the sensor
		float64 m_sampleRate;
		float32 m_defaultFidelity;
	};
	
	class VaneSimExport ISensorFactory
	{
	public:
		virtual Sensor* CreateSensor( SensorStaticAssetParams& params, DynamicAssetParams& dynamicParams ) = 0;
		virtual StaticAssetParamsPtr ParseXmlParams( const TiXmlElement* pXmlElement ) = 0;
	
		virtual ~ISensorFactory() {}
	};

	/// Interface for visualizing sensor data.
	class VaneSimExport ISensorVisualizer
	{
	public:
		/// Get the type of sensor this visualizer is used for.
		virtual SensorType GetType() const = 0;
		virtual ~ISensorVisualizer() { }
	};

	/// Interface for a sensor visualization manager.
	class ISensorVisualizationManager
	{
	public:
		/// Create a new sensor visualizer.
		/// @param[in] sensorType The type of sensor to create a visualizer for.
		/// @param[in] name Name of the visualizer.
		/// @param[in] x X location of the visualizer.
		/// @param[in] y Y location of the visualizer.
		virtual ISensorVisualizer* CreateSensorVisualizer( const SensorType& sensorType, const String& sensorAsset, const String& name, bool isReplay, int32 x = 0, int32 y = 0 ) = 0;
		
		///Destroy a sensor visualizer
		virtual void DestroySensorVisualizer( ISensorVisualizer* pVisualizer ) = 0;

	};
	
	///Generic abstract interface for something that can supply power.
	class IPowerSource
	{
	public:
		///@return the actual power that can be used by this unit.
		virtual Units::Power SupplyPower( Units::Power power, Units::Voltage voltage ) = 0;
	};

	///Data for a radio frequency ID ( RFID ) emitter. These are registered with the
	///sensor manager for the RFID sensor.
	struct RFIDEmitter
	{
		//maximum distance this emitter can transmit
		float64 m_distance;
		//Unique ID for this emitter, which is normally the VaneID it is associated with.
		VaneID m_id;
	};

	///Our sensor ID type
	typedef VaneID SensorID;
	///Invalid sensor ID
	const SensorID kInvalidSensorID = 0x00;

	///Sensor - base class for all sensors that will be used in VANE
	///Certain methods can be overridden to allow differing types of sensors that
	///will still mostly share the same format
	///We will need to define some sensor types, such as:
	///kPosition, kPositionAndOrientation, kVelocity, etc.
	class VaneSimExport Sensor
	{
	public:
		friend class SensorManager;
		
		enum PowerState
		{
			kPowerStateOff,     ///< Sensor is off, using no power
			kPowerStateStandby, ///< Sensor is in standby mode, using standby current
			kPowerStateOn,      ///< Sensor is in the On mode
			kNumPowerStates
		};
		
		enum PowerSource
		{
			kConnectedToExternalPower, ///< Sensor is connected to an external (vehicle) power source
			kInternalBattery,          ///< Sensor uses its own internal battery
			kNoPowerSource,            ///< Sensor is not hooked up to any power source
			kNotSimulated              ///< Sensor is not simulating power usage
		};
		
		struct PowerUsageProfile
		{
			Units::Voltage SupplyVoltage;
			//How much power is consumed when unit is in off, standby, or Active modes.
			Units::Power Consumption[ kNumPowerStates ];
		};

		//The default sample rate for all sensors
		static const float64 kDefaultSampleRate;

		///Create a base sensor with the given ID and params
		///@param[in] specificID ID of the specialized sensor type instance
		///@param[in] sensorParams The sensor specific parameters for this sensor
		///@param[in] dynamicParams The placement specific parameters for this sensor
		Sensor( VaneID specificID, SensorStaticAssetParams& sensorParams, DynamicAssetParams& dynamicParams );
		
		///Virtual destructor that not only closes any open files, 
		///it also removes itself form the global list of sensors in the 
		///sensor mgr
		virtual ~Sensor();

#if 0 // For future use with a post creation step that can create visualizers, etc.
		///Post construction initialization task that will be run after the sensor has been created.
		virtual void Initialize() {} 
#endif 

		///Get our sensor type
		inline SensorType GetSensorType() const { return m_type;}
		
		///Get our sample rate
		///@return sample rate in hz
		inline double GetSampleRate() const { return m_sampleRate;}
		
		///Get the sample size in secs (1/sampleRate)
		///@return sample step in seconds
		inline double GetSampleStep() const { return m_sampleStep;}

		///Get the simulation time from the last sample
		inline float64 GetLastSampleTime() const { return m_lastSampleTime; }
		
		///Get our sensor name
		inline String GetName() const { return m_name;}

		//Update our sensor
		virtual void Update(TimeValue dt) = 0;
		
		///Update the power state of this sensor
		virtual void SetPowerState( PowerState newPowerState ) { m_powerState = newPowerState; }
		
		///Retrieve this sensor's current power state
		virtual PowerState GetPowerState() const { return m_powerState; }
		
		//Non overridable call to get the base sensor ID
		VaneID GetBaseSensorID() const { return m_baseSensorID;}
		
		//Subclasses that have their own ID should override this.
		VaneID GetID() const { return m_specificSensorID; }
		
		virtual bool UpdatePowerConsumption();

		//Get the string name of our sensor type, used for human readable description in file
		static String GetSensorTypeString( SensorType type);

		//Get the world object from this sensor
		World::Object* GetWorldObject() const {return m_pWorldObject;}

		///Set the parent of a sensor based upon its locality to objects.
		///This is used for drag and drop placement.
		void SetParentByLocality();

	protected:
		
		float64 m_sampleRate;
		float64 m_sampleStep; //how long a step s 1/ sampleRate

		///time left until we write another sample
		float64 m_sampleTimeLeft;
		float64 m_elapsedTime;
		float64 m_lastSampleTime;

		//0 - 1 value for fidelity settings.
		//0 = no accuracy, 1.0 = full simulation of actual sensor.
		float32 m_fidelity;
		
		//Our root SensorID, not specialized by sensor type.
		SensorID m_baseSensorID;
		//The specialized sensor ID.
		SensorID m_specificSensorID;

		///The name of our sensor, should be unique
		String m_name;

		///our type of sensor 
		SensorType m_type;
		
		///The current power state of this sensor.
		PowerState m_powerState;
		PowerSource m_powerSourceStatus;
		PowerUsageProfile m_powerUsage;
		
		//The optional power source from which to draw power
		//if we are simulating power consumption.
		IPowerSource* m_pPowerSource;

		///The world object that represents our sensor asset.
		World::Object* m_pWorldObject;	
		
		bool m_enabled;
	};

	typedef SharedPtr<Sensor> SensorPtr;
	
	///A collection of sensors
	typedef std::vector<SensorPtr> SensorList; 
	typedef std::vector<SensorID> SensorIDList;
	
	///A Singleton to manager all current sensors
	///Sensor manager only has references to sensors since it does not
	///own the sensors itself.  Vehicles, environments, and other entities own
	///sensors.  However the sensor manager knows about each sensor since it can
	///only be created through the CreateSensor interface in the manager
	///This allows Sensor owners to possess the sensors, while also allowing
	///the sensor manager to access ALL sensors present in the world
	///When a sensor is destroyed, it should un register itself from the
	///sensor manager
	class VaneSimExport SensorManager 
		  : public Singleton<SensorManager>
		  , public IPropertyProvider
		  , public IXmlInterface
		  , public IAssetHandler
	{
	private:
		SensorManager();
		~SensorManager();

	public:
		static void Initialize();
		static void Shutdown();

		//for use with dlls, we need to override template behavior, otherwise
		//linking errors
		static SensorManager* GetSingletonPtr();
		static SensorManager& GetSingleton();

		void Update(TimeValue dt);
		
		void RegisterSensorFactory( const SensorType& sensorType, ISensorFactory* pFactory );
		void UnregisterSensorFactory( ISensorFactory* pFactory );
		
		///Turn sensor visualization on or off
		///With visualization on, sensors will be rendered on screen
		void SetSensorVisualizationEnabled(bool enabled);
		
		///Have the SensorManager take over ownership of the default sensor factory.
		void SetDefaultSensorFactory( ISensorFactory* pSensorFactory );

		///Get access to all sensors
		const std::vector<SensorPtr>& GetAllSensors();

		///Remove a sensor from the manager
		void RemoveSensor( Sensor* sensor );
		void RemoveSensor( SensorID sensorID );
		
		//Removes all sensors
		void ClearSensors();
		
		///Add a sensor* to our internal registration. 
		///Should only be called by Sensor base class.
		///@param sensor A sensor to add
		void AddSensor( Sensor* pSensor );

		SensorPtr GetSensor( SensorID id );
		
		/// Build an id set for sensors using their IDs, names, and an invalid ID.
		/// @return The vane ID set for the current sensors.
		VaneIdSet GetSensorIdSet(VaneID sensorType = kInvalidVaneID) const;

		std::vector<SensorID> GetSensorIDs();
		StringVector GetSensorNames();

		void SetVisualizationManager( ISensorVisualizationManager* pVisualizationManager ) { m_pVisualizationManager = pVisualizationManager; }
		ISensorVisualizationManager* GetVisualizationManager() { return m_pVisualizationManager; }

		///Get the global sensor simulation fidelity scale. This can be used to 
		///scale simulation accuracy vs performance depending on the current simulation conditions.
		float32 GetGlobalSensorFidelity() const { return m_globalFidelityScale; }

		///Register something as an RFID Emitter
		void AddRFIDEmitter( VaneID emitterID, float64 range );
		void RemoveRFIDEmitter( VaneID emitterID );
		std::vector<RFIDEmitter> GetRFIDEmitters( ) const { return m_rfidEmitters; }

	public: //[IPropertyProvider methods]
		virtual ObjectPropertySet GetProperties(VaneID objID);
		virtual VaneIdVector GetIdsOfType(const VaneID dataType) const;

	public: // [IXmlInterface]
		virtual bool ReadXmlFromNode( const TiXmlNode* pElement );
		virtual void WriteXmlToNode( TiXmlNode* pParentElement );

	public: //[IAssetHandler methods]
		VaneID CreateAssetInstance(const StaticAssetParamsPtr& pStaticParams, VaneID dataType, const DynamicAssetParamsPtr& pDynParams);
		void DestroyAssetInstance(VaneID objId);
		StaticAssetParamsPtr ParseXmlToBinary(const TiXmlElement* pXmlParams, VaneID dataType);
		VaneIdVector GetHandledDataType() const;
		VaneID GetMainWorldObjectId(VaneID id);
		VaneID GetInstanceUsingMainWorldObject(VaneID id);

	private:
		PropertyGroupInstance GetSensorManagerProperties();
		void                  RegisterSensorManagerProperties();
		PropertyGroupInstance GetSensorProperties( Sensor& sensor );
		void				  RegisterSensorProperties();

	private:
		
		//Update our command enabled/disabled states
		void UpdateCommandState( bool hasSensors );
		
		SensorID GetNextSensorID();
		
	private:
		
		///List of all sensors in the world.  
		SensorList m_sensors;

		///How long the last update took to process in ms (used for profiling / display )
		TimeValue m_updateTime;
		
		VaneID m_sensorIDCount;

		ISensorVisualizationManager* m_pVisualizationManager;
		
		typedef std::map< SensorType, ISensorFactory*> SensorFactoryMap;
		//The registered sensor factories
		SensorFactoryMap m_factories;
		SharedPtr<ISensorFactory> m_defaultFactory;

		//The currently valid RFID Emitters
		std::vector<RFIDEmitter> m_rfidEmitters;

		Timer m_sensorTimer;

		//A quick way to manipulate the fidelity calculation on all sensors at once 
		//to adjust for performance.
		float32 m_globalFidelityScale;
		
		///whether or not ALL sensor processing is enabled.
		bool m_simulationEnabled; 

		///Our specialized CommandGroup for working with Mgr functions
		///this will not need a public interface so it is declared as an internal
		///class.  This handles all of the command management for the Manager
		class SensorCommandGroup
			: public CommandGroup
			, public CommandSelectionListener
		{
		public:
			///Create a sensor command group with a given sensor mgr
			///NOTE: Even though the Manager is a singleton, we still want
			///to verify that it actually has been created, and we want to have
			///the option to use it without requiring it to be a Singleton
			///@param[in] mgr The manager to use
			SensorCommandGroup(SensorManager& mgr);

			///Destructor
			~SensorCommandGroup();

			///Handle commands specific to the Sensor manager 
			///@param[in] commandID The ID of the command to handle
			///@param[in] parameterList The parameters to use for this command
			///@return A commandresult describing the success or failure of this command
			CommandResult HandleCommand( CommandID commandID, const CommandParamList& parameterList);

			/// Get a list of possible values for each parameter for the specified command.
			/// NOTE: Not all commands or parameters may have choices and can either be left blank
			/// or assigned a value of Variant::INVALID.
			/// @param[in] commandID The command to get possible parameter choices for.
			CommandParamList GetParameterChoices( CommandID commandID );

			void OnCommandSelectionUpdate();

		protected:
			//our sensor manager reference
			SensorManager& m_mgr;

			///A helper that gets the selected IDs, and returns a vector
			/// comprised of only sensor ids. Is uses the VaneIDs that are SensorIDs,
			/// as well as checking any WorldObjectIDs to see if they belong to a Sensor
			/// and returning that Sensor's ID as well.
			///@return A vector of SensorIDs.
			VaneIdVector GetSelectedSensors();
		};

		SensorCommandGroup m_commandGroup;
	};
}

#endif
