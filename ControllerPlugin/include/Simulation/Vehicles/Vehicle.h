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
#ifndef VANE_Vehicle_h__
#define VANE_Vehicle_h__

#include "Core/ControllerInterface.h"
#include "Core/Core.h"
#include "Core/AxisAlignedBoundingBox.h"
#include "Core/Property.h"
#include "Core/Vector3_MKS.h"

#include "Simulation/SimulationCommon.h"
#include "Simulation/Sensor.h"
#include "Simulation/Physics/Servo.h"
#include "Simulation/ISoundSystem.h"
#include "Simulation/Vehicles/VehicleDataProvider.h"
#include "Simulation/Vehicles/VehicleTypes.h"
#include "Simulation/Vehicles/Motor.h"
#include "Simulation/Vehicles/MotorSpeedController.h"
#include "Simulation/Vehicles/Battery.h"
#include "Simulation/Vehicles/ElectricMotor.h"
#include "Simulation/World/WorldTypes.h"
#include "Simulation/RendererManager.h"
#include "Core/Vector2_MKS.h"

namespace VANE
{
	class Servo;
	class INavigationSystem;
	namespace World
	{
		class Object;
	}

	namespace Vehicles
	{
		//forward declares
		class Object;
		class Control;
		class MotorController;
		class SteeringController;
		class TractionObject;

		class Manager;

		// Support for "customAttributes" in vehicle.xml file:
		// Class to support Case-Insensitive String Compare
		class CaseInsensitiveCompare
		{
		public: 
			bool operator() (const String& lhs, const String& rhs ) const
			{
				return StringUtil::LessCaseless(lhs, rhs);
			}
		};
		
		///Vehicle Sound management. Allows user's to specify simple parametric 
		///states for sounds. This can only be created by the parent Vehicle.
		class SoundProfile
		{	
			friend class Vehicle;
			
		public:
			~SoundProfile();
			
			///Set the vehicle ID this sound is associated with
			inline void SetVehicleID( World::WorldObjectID objID ) { m_vehicleObjID = objID; }
			
			///Set the Motor RPM threshold for the idle sound. Any RPM value lower than this
			///will use the Idle sound instead of the Running loop
			inline void SetIdleThreshold( float32 threshold ) { m_idleThreshold = threshold;}
			
			///Set the sound to use for the Idle loop
			void SetIdleLoop( const String& idleLoopFile );
			
			///Set the sound to use for when the motor is running, which will be scaled
			///by the throttle input
			void SetRunningLoop( const String& runningLoopFile );

			///Update this sound profile, based on the current vehicle state
			///@param[in] running If the vehicle is running
			///@param[in] rpm The motor speed in RPM ( the highest, if multiple motors are used )
			///@param[in] throttle The highest throttle value 
			///@param[in] speed The forward speed of the vehicle
			void Update( bool running, float64 rpm, float64 throttle, float64 speed );
			
		private:	
			///Create a sound profile, which can only be done by its parent vehicle
			SoundProfile( );
			
			World::WorldObjectID m_vehicleObjID;
			Sound::SoundID m_idleLoop;
			Sound::SoundID m_runningLoop;

			//Motor RPM threshold. Any RPM lower than this will use the Idle sound
			//instead of the Running Loop sound.
			float32 m_idleThreshold;
		};

		// Map to store arbitrary name/value pairs read from
		// vehicle.xml	(not case sensitive)
		typedef std::pair<String, String> CustomAttribute;
		typedef std::map<String, String, CaseInsensitiveCompare> CustomAttributeMap;
		
		//////////////////////////////////////////////////////////////////////////
		//
		//  Vehicle
		//
		//////////////////////////////////////////////////////////////////////////

		///base class for our implementations that can widely vary, based on Physics or 
		///other specialized needs.
		class VaneSimExport Vehicle : 
			public Controller::IControllable,
			public VehicleDataProvider,
			public IXmlInterface
		{
			friend class VANE::Vehicles::Manager;

		protected:
			///Protected constructor, since we only want subclasses to be able to construct a vehicle
			///@param[in] name The individual name of this vehicle, must be unique
			///@param[in] type The type of vehicle this is.
			Vehicle(const String& name, const String& type);
			
			void SetVehicleID(VehicleID id);

			/// Update Vehicle Motion State, called by ODEVehicle::Update()
			///@return new motion state
			virtual VehicleMotionState UpdateVehicleMotionState(TimeValue dt) = 0;
		public:

			/// values returned by "WhichSide()":
			enum RelativePosition
			{
				kLeft = 0, 
				kRight = 1,
				kCenter = 2    
			};

			///Different states for the bounding box.
			enum BoundingBoxContents
			{
				kBodyOnly,           ///< The bounding box contains the body of the vehicle only
				kBodyPlusDriveSystem ///< The bounding box contains the body and also the drive system (treads/wheels)
			};
			
			///virtual destructor since we'll be handling these with ptrs to
			///vehicles mainly
			virtual ~Vehicle() ;

			/// Compute "Beam" and WheelBase for Steering Calculations
			///@return void
			virtual void Initialize();
			
			/// React to "Reset Vehicle" in GUI
			///
			virtual void OnVehicleReset();
			
			///Get the ID of this vehicle
			VehicleID GetVehicleID() const {return m_vehicleID;}

			/// Set Vehicle-Modified flag to force rebuild
			/// of UI properties tree (SimTreeView)
			void SetModified() const;

			///Get our individual vehicles name
			String GetName() const { return m_name; }

			///Get the type of vehicle this is, eg Andros Mini II, Talon...
			String GetType() const {return m_type; }

			///Enable or disable input for this vehicle
			inline void SetInputEnabled(bool enabled) {m_inputEnabled = enabled;}
			
			///Get whether input is currently enabled or disabled for this vehicle
			inline bool GetInputEnabled() { return m_inputEnabled;}

			///Set the file name of the vehicle definition file that was used to load this vehicle
			void SetFileName(String filename) {m_fileName = filename;}
			///Get the file name of the vehicle definition file that was used to load this vehicle
			String GetFileName() {return m_fileName;}

			/// Get Vehicle "Beam" (Distances between centers of front (or rear) wheels)
			///@return Vehicle Beam
			Units::Length GetTrack() const {return m_track;};

			/// Get Vehicle Wheelbase
			///@return WheelBase
			Units::Length GetWheelBase() const {return m_wheelBase;};

			///Set the position of the entire vehicle (from center)
			virtual void SetPosition(const Vector3& position) =0;

			///Get the position of the center of the vehicle
			virtual Vector3 GetPosition() const =0;

			/// Get Vehicle Roll
			///@return Vehicle Roll
			Radian GetRoll() const {return Radian(m_roll);}

			/// Get Vehicle Yaw
			///@return Vehicle Yaw
			Radian GetYaw() const {return Radian(m_yaw);}

			/// Get Vehicle Pitch
			///@return Vehicle Pitch
			Radian GetPitch() const {return Radian(m_pitch);}

			///Set the orientation of the vehicle
			virtual void SetOrientation(const Quaternion& orientation) =0;
			
			///Get the orientation of vehicle
			virtual Quaternion GetOrientation() const =0;
			
			///Get the transform to get the vehicle into our correct coordinate system.
			virtual Quaternion GetCoordinateTransform() const =0;

			///Set the transform that is used to rotate this vehicle to the correct coordinate system
			virtual void       SetCoordinateTransform(const Quaternion& quat) =0;

			/// Determine which side of vehicle wheel or other object resides on
			///@param[in] original coordinates of object
			///@return 0 if object is on left side of vehicle,
			///        1 for right,
			///        2 for Centerline
			virtual RelativePosition GetRelativePosition(Vector3 origPos) const = 0;

			///Get the angular velocity of the vehicle
			virtual Vector3 GetAngularVelocity() const = 0;

			/// Get Vehicle Yaw Rate
			///@return Yaw Rate
			virtual Units::RadialSpeed GetYawRate() const {return m_yawRate;}

			/// Get Vehicle Roll Rate
			///@return Roll Rate
			virtual Units::RadialSpeed GetRollRate() const {return m_yawRate;}

			// Method defined in VehicleDataProvider
			virtual float64 GetYawRateAsDouble() const {return m_yawRate;}

			/// Get Current Forward speed (signed)
			///@return Forward Speed
			Units::Speed GetForwardSpeed() const {return m_forwardSpeed;}

			///Get "Nominal" wheel speed, based on rated speed of motor & Gearing
			///@return Nominal Wheel Speed
			Units::RadialSpeed GetNominalWheelSpeed( ) const;

			///Get estimated maximum speed speed, meaning speed of
			/// vehicle when motor is running at rated speed and 
			/// there is no wheel slip.
			///@return Estimated Maximum Speed
			virtual Units::Speed GetMaximumForwardSpeed( ) const = 0;

			/// Get Vehicle Forward Momentum.  This is a signed quantity,
			/// and ignored rotational momentum of wheels.
			///@return Vehicle forward Momentum
			Units::ScalarMomentum GetForwardMomentum() const;

			/// Get Angular Momentum about arbitrary axis
			///@return Angular Momentum (scalar)
			virtual Units::AngularMomentum GetAngularMomentum(const Vector3& axis) const = 0;

			/// Get Vehicle Velocity Vector (named to avoid conflict with "ODEVehicle::GetVelocity())
			///@return Vehicle Velocity
			virtual Vector3_MKS_types::Velocity GetVelocityVector() const = 0;

			/// Get Linear Acceleration
			///@returnLinear Acceleration
			virtual Vector3_MKS_types::Acceleration GetAccelerationVCS() const = 0;

			/// Get Current Velocity in Vehicle Coordinates
			///@return Current velocity in VCS
			virtual Vector3_MKS_types::Velocity GetVelocityVCS() const = 0;

			virtual Vector3 GetLinearAcceleration() const = 0;
			virtual Vector3 GetAngularAcceleration() const = 0;

			///Get the total distance traveled, in meters
			float64 GetTotalForwardTraveledDistance() const { return m_forwardTravelDistance; }

			/// Get Current Velocity in Vehicle Coordinates at specified VCS position
			///@param[in] VCS Coordinates
			///@return Current velocity in VCS
			virtual Vector3_MKS_types::Velocity GetVelocityVCSAtPoint(const Vector3& vcsPos) const = 0;

			/// Vehicle will be considered "Stationary" in forward direction if its forward
			/// speed is less then "m_forwardSpeedTolerance" (about 0.001 M/S for vehicle
			/// one meter in length).  
			///@return Forward Speed tolerance
			const Units::Speed& GetForwardSpeedTolerance() const { return m_forwardSpeedTolerance;};

			/// Set Forward Speed tolerance (used by VTI code)
			///@param[in]  Forward speed tolerance, smaller values are considered equal to zero
			///@return void
			void SetForwardSpeedTolerance(Units::Speed fwdSpeedTol) { m_forwardSpeedTolerance = fwdSpeedTol;}

			/// Vehicle will be considered Stationary in lateral direction if its lateral
			/// speed is less then "m_lateralSpeedTolerance", which defaults to 0.3 times
			/// the forward speed tolerance (about 0.0003 M/S for vehicle one meter long)
			///@return Lateral Speed tolerance
			const Units::Speed& GetLateralSpeedTolerance() const { return m_lateralSpeedTolerance;};

			/// Set Lateral Speed tolerance (used by VTI code)
			///@param[in]  Lateral speed tolerance, smaller values are considered equal to zero
			///@return void
			void SetLateralSpeedTolerance(Units::Speed lateralSpeedTol) { m_lateralSpeedTolerance = lateralSpeedTol;}

			/// Vehicle will be considered to be "Not Rotating" if its Angular Speed is
			/// less then returned value, computed based on lateral speed tolerance and
			/// wheelbase
			///@return Radial Speed tolerance
			Units::RadialSpeed GetYawRateTolerance() const;
			
			///Get the bounding box for this vehicle
			///@param[in] contents Which type of bounding box you would like.
			///@see BoundingBoxContents
			virtual AxisAlignedBoundingBox	GetBoundingBox(BoundingBoxContents contents = kBodyOnly) = 0;

			///Getting control files (part of controllable?)
			///@return the filename used for control definitions
			virtual String GetControlFile() const=0;

			///Set the control file to the given filename
			///Loads the control definitions from a given filename
			///@param filename The file that defines controls for this vehicle
			virtual void SetControlFile(const String & filename)=0;// { m_controlFileName = filename;}

			///Get a control by name
			///@param[in] name
			///@return 
			virtual Control* GetControlByName( const String& name ) const = 0;

			///Add a control to the vehicle
			virtual void AddControl(Control* controls)=0;

			///Get the navigation system associated with this vehicle.
			virtual INavigationSystem* GetNavigationSystem() const { return m_pNavigationSystem; }

			///Set the navigation system associated with this vehicle
			virtual void SetNavigationSystem( INavigationSystem* pNavSystem ) { m_pNavigationSystem = pNavSystem; }
			
			///Add a sensor to the vehicle
			virtual void AddSensor( SensorID sensorID )=0;
			
			///Get the list of all sensor IDs from Sensors that are attached
			///to this vehicle.
			virtual SensorIDList GetSensors() const =0;

			///Get a servo by name
			///@param[in] name
			///@return 
			virtual Servo* GetServoByName(const String &name) const = 0;
			
			/// Get all servo IDs attached to this vehicle
			///@return 
			virtual ServoIDVector GetServoIDs() const = 0;
			
			///Add a servo to this vehicle.  It can control vehicle object connections.
			virtual void AddServo( ServoID servo ) = 0;

			///Get a motor by name.
			virtual Motor* GetMotorByName(const String &name) const;
			virtual VaneIdVector GetMotorIDs() const { return m_motorIDs; }

			/// Add a motor to the vehicle.
			/// @param[in] pMotor The motor to add.
			virtual void AddMotor( const MotorSharedPtr& pMotor );

			/// Add pointer to Electric Motor Prototype, for display in SimTreeView
			///@param[in]   pMotorPrototype   Pointer to motor prototype
			///@return 
			void AddElectricMotorPrototype(const DCElectricMotorPrototypePtr& pMotorPrototype);

			/// Get Motor Prototype by Name
			///@param[in] typeName
			///@return MotorPrototypeSharedPtr
			MotorPrototypeSharedPtr GetMotorPrototypeByName(   const String& typeName);

			/// Get Vector of Electric Motor Prototypes for display in SimTreeView
			///@return MotorPrototypeVector
			const MotorPrototypeSharedPtrVector& GetMotorPrototypes() const {return m_electricMotorPrototypes;};

			///Get the motors for this vehicle
			///@return MotorSharedPtrContainer&
			const MotorSharedPtrContainer& GetMotors() const;

			/// Get Electric Motor by Vane ID
			///@param[in] Motor ID
			///@return pointer to Motor or NULL
			ElectricMotorWeakPtr  GetElectricMotorByID( VaneID  motorID);

			//Get number of Motors
			//@return number of Motors
			uint32 GetNumMotors() const { return m_motors.size();};

				/// Add shared pointer to Motor Speed Controller Factory
			///@param[in]  pSpeedControllerFactory 
			///@return 
			void AddSpeedControllerFactory( const MotorSpeedControllerFactorySharedPtr& pSpeedControllerFactory );

			/// Get Speed Controller Factory based on Speed Controller type name
			///@param[in] controllerTypeName   Controller Type Name
			///@return                         Pointer to Motor Speed Controller Factory
			MotorSpeedControllerFactorySharedPtr GetSpeedControllerFactoryByName(const String& controllerTypeName) const;

			/// Get Speed Controller Factory from ID
			///@param[in] factoryID      Factory ID
			///@return                   Pointer to Motor Speed Controller Factory
			MotorSpeedControllerFactorySharedPtr GetSpeedControllerFactoryByID(const VaneID factoryID) const;

			/// Get Speed Controller Factory from ID
			///@param[in] factoryID      Factory ID
			///@return                   Pointer to Motor Speed Controller Factory
			MotorSpeedControllerFactorySharedPtr GetSpeedControllerFactoryByTypeID(const VaneID factoryTypeID) const;

			/// Get Vector of Pointers to Speed Controller Factories
			///@return 
			const MotorSpeedControllerFactorySharedPtrVector& GetSpeedControllerFactories() const {return m_motorSpeedControllerFactories;};

			/// Add a motor controller to the vehicle.
			/// @param[in] pMotorController The motor controller to add.
			virtual void AddMotorController( MotorController* pMotorController ) = 0;

			/// Get Vector of Motor Controllers on Vehicle
			///@return Motor Controllers
			virtual const MotorController* GetMotorControllerByType(VaneID motorControllerTypeID) const = 0;

			/// Add a steering controller to the vehicle.
			/// @param[in] pSteeringController The steering controller to add.
			virtual void AddSteeringController( SteeringController* pSteeringController ) = 0;

			///Return true if this vehicle uses skid or differential steering. Most
			///robots will use skid steering, while cars use standard steering.
			virtual bool IsSkidSteered() const = 0;

			/// Set Vehicle Battery System
			///@param[in] Battery System ID
			inline void SetBatterySystem(VaneID batterySysID) { m_batterySystemID = batterySysID;};

			/// Get ID of Vehicle Battery System
			///@return Battery System ID 
			VaneID GetBatterySystemID() const {return m_batterySystemID;};

			/// Get pointer to Vehicle Battery System
			///@return pointer to battery system, or NULL if none exists
			BatterySystem* GetBatterySystem() const;

			/// Get total power consumed by vehicle motors for timestep (KWatts)
			///@return Total Power
			Units::Power GetMotorPower() const;

			///Set visibility for this vehicle
			virtual void SetVisible(bool visible)=0;

			///Calls to update any states necessary before we do the physics sim steps
			virtual void PreSimUpdate() {}
			
			///Calls to do any post simulation step updates.
			virtual void PostSimUpdate( TimeValue simulatedTime ) { VANE_UNUSED( simulatedTime ); }

			///Update the vehicle
			virtual void Update(TimeValue dt)=0;
			
			///Update the tracks and particles for this vehicle
			virtual void UpdateWheelTracksAndParticles( TimeValue dt, bool override=false) = 0;

			virtual void InterruptTracks() = 0;

			///Halt the vehicle entirely and instantly. Removes all momentum, etc.
			virtual void Halt()=0;

			///Get the world object associated with the vehicle
			///There is only one main world object for a vehicle, while the rest are VehicleObjects
			///@see VehicleObject
			virtual World::Object* GetWorldObject() const=0;

			VaneID GetInstanceId() const { return m_vehicleID; }

			///Turn physics on or off for this vehicle
			virtual void SetPhysicsEnabled(bool enabled) =0;

			///Set the external control state for this vehicle. If it is externally controlled, it will
			///not use its own update, but only move from external controllers
			inline void SetExternallyControlled(bool externallyControlled) {m_externallyControlled = externallyControlled;}
			
			///Get whether or not this vehicle is currently externally controlled
			inline bool GetExternallyControlled() {return m_externallyControlled;}

			inline SoundProfile& GetSoundProfile() { return m_soundProfile; }

			///If gravity should affect the vehicle
			virtual void SetGravityEnabled(bool enabled)=0;
			
			///Get whether or not gravity should affect this vehicle
			virtual bool GetGravityEnabled() const=0;
			
			/// Add some mass to the vehicle. This is not for physics, but more
			/// of a generic mass value.
			///@param[in] mass
			virtual void AddMass(double mass) {m_totalMass += mass;}
			
			/// Set the current mass to 0
			virtual void ClearMass() { m_totalMass = 0.0;}
			
			/// Set the total mass value of the vehicle to a specific value
			///@param[in] mass New mass in Kg
			///@return 
			virtual void SetMass(double mass) {m_totalMass = mass;}
			
			///Get the total mass value for this vehicle
			virtual double GetTotalMass() const {return m_totalMass;}

			/// Get Total Vehicle mass, returned as Units::Mass
			Units::Mass GetMass() const {return Units::kKilograms * m_totalMass;}

			/// Get mass of chassis only
			///@return Chassis Mass
			virtual Units::Mass GetBodyMass() const = 0;

			///Set the additional mass beyond the vehicle's initial parameters on the body object
			virtual void SetAdditionalMass(double additionalMass) = 0;
			
			///Get the current additional mass applied to the body object
			virtual double GetAdditionalMass() const = 0;
			
			///Remove all additional mass (payload)
			virtual void RemoveAdditionalMass() = 0;

			///Get the inertia tensor from this vehicle
			virtual Matrix3 GetInertiaTensor() const = 0;

			/// Get Vehicle Center of Mass
			///@return Center of Mass
			virtual Vector3_MKS_types::Position GetCenterOfMass() const = 0;
			
			///Set the air resistance related parameters for this vehicle.
			///@param[in] dragCoefficient 
			///@param[in] surfaceArea Frontal Surface Area in m^2
			void SetDragParameters( float64 dragCoefficient, float64 surfaceArea );

			///Get the drag coefficient of this vehicle
			///@return the drag coefficient of this vehicle
			inline float64 GetDragCoefficient() const { return m_dragCoefficient; }
			
			///Get the frontal surface area. Used for drag calculations
			///@return frontal surface area in m^2
			inline float64 GetFrontalArea() const { return m_frontalArea; }

			///Get a vector of wheels for this vehicle.
			virtual std::vector<VANE::Vehicles::Object*> GetWheels() const = 0;

			///Get a vector of tracks for this vehicle.
			virtual std::vector<VANE::Vehicles::Object*> GetTracks() const = 0;

			///Get a vector of all traction objects that belong to this vehicle.
			virtual std::vector<TractionObject*> GetTractionObjects() const = 0;

			/// Get Wheel by Index
			/// @return pointer to = wheels
			virtual Wheel* GetWheel(uint32 idx) const = 0;

			/// Get number of wheels on vehicle
			/// @return number of wheels
			virtual size_t GetWheelCount() const = 0;

			/// Get Index of Wheel from Pointer
			///@return wheel index
			virtual size_t GetWheelIndex(const Wheel* pWheel) const = 0;

			/// Get track by Index
			/// @return pointer to track
			virtual Track* GetTrack(uint32 idx) const = 0;
			
			///Get the number of tracks attached to this vehicle
			///@return the current number of tracks attached to this vehicle
			virtual size_t  GetTrackCount() const = 0;

			/// Get the current collection of vehicle objects
			///@return The entire set of vehicle objects
			virtual std::vector<VANE::Vehicles::Object*> GetVehicleObjects() const =0;
			
			/// Get the number of current vehicle objects
			///@return The number of current vehicle objects
			virtual unsigned int GetVehicleObjectCount() const=0;
			
			///Get the positions of the vehicle objects
			virtual std::vector<Vector3> GetVehicleObjectPositions() const =0;
			
			///Get the orientations of all of the vehicle objects
			virtual std::vector<Quaternion> GetVehicleObjectOrientations() const =0;

			///Set the positions of the vehicle objects
			///@NOTE: This assumes you will be setting the positions in the exact same order
			///as they are defined. If the order is different, expect the vehicle to come out quite wrong
			virtual void SetVehicleObjectPositions(const std::vector<Vector3>& positions) =0;
			
			///Set the orientations of the vehicle objects
			///@NOTE: This requires the vehicle object orientations are ordered in the same way that they
			///are loaded in (or returned in GetVehicleObjectOrientations). Otherwise the vehicle will come out 
			///incorrect
			virtual void SetVehicleObjectOrientations(const std::vector<Quaternion>& orientations)=0;

			/// Get String representing Vehicle Motion State ("CoastingState", etc.) for debug output
			///@param[in] motionState
			///@return  String representing name of motion state
			static String GetVehicleMotionStateName(VehicleMotionState motionState);

			/// Get Current Vehicle Motion State ("kVehicleDriving", "kVehicleCoasting", etc.)
			///@return VehicleMotionState enum
			VehicleMotionState GetVehicleMotionState() const {return m_vehicleMotionState;};

			/// "ComingToRest" returns true if vehicle is coasting and speed
			/// is less then 1% of peak
			///@return true if coming to stop.
			bool ComingToRest() const;

			/// Get Direction of Motor power, may not = vehicle motion direction
			/// if vehicle changing direction
			///@return VehicleMotionDirection Enum
			VehicleMotionDirection GetVehicleThrustDirection() const {return m_vehicleMotionDirection;};
  
			/// Return true if Vehicle Powered
			///@return
			virtual bool IsVehiclePowered() const = 0;

			///Get the height of the Center of Gravity ( vehicle coord = (0,0,0) )
			///relative to the ground.
			virtual float64 GetCgHeight() const = 0;

			///------------Custom Attributes --------------
			/// "Custom Attributes" is a mechanism for adding arbitrary
			///  client-defined naem/value pairs to vehicle.xml; 
			/// for example:
			///	<customAttributes>
			///		<param name="VaneGCE.SoilType" value="SW" />
			///		<param name="VaneGCE.SoilStrength" value="200" />
			///	</customAttributes>
			/// Quoted values are NOT Case Sensitive.

			///    Add a Name/value pair representing a "custom Attribute",
			/// to the customAttributeMap (called by the xml parser
			/// when reading "customAttribute" records in vehcile.xml)
			///@param[in] key
			///@param[in] value
			///@return
			void AddCustomAttribute(const String& key, const String& value);

			// Check is a particular custom attribute is defined
			// in vehicle.xml
			///@param[in] Attribute Name
			///@return true if attribute defined in vehicle.xml
			bool AttributeExists(const String& attribName);

			///Read a stored Custom attribute out of customAttributeMap. 
			///@param[in] key
			///@param[in] defaultVal  value to return if key not found
			///@return String Value
			String ReadCustomAttributeString(const String& key, const String& defaultVal=String());

			/// Read a stored Custom attribute out of customAttributeMap,
			///   attempt to convert "value" to double before returning
			///@param[in] key
			///@param[in] defaultVal  value to return if key not found
			///@return double value, 0.0 if parsing fails
			float64 ReadCustomAttributeDouble(const String& attribName, const float64 defaultVal = 0.0);

			/// Read a stored Custom attribute out of customAttributeMap,
			///   attempt to convert "value" to int32 before returning
			///@param[in] key
			///@param[in] defaultVal  value to return if key not found
			///@return int32 value, 0 if parsing fails
			int32 ReadCustomAttributeInt32(const String& attribName, const int32 defaultVal = 0);

			/// Read a stored Custom attribute out of customAttributeMap,
			///   attempt to convert "value" to uint32 before returning
			///@param[in] key
			///@param[in] defaultVal  value to return if key not found
			///@return uint32 value, 0 if parsing fails
			uint32 ReadCustomAttributeUInt32(const String& attribName, const uint32 defaultVal= 0);

			bool ReadCustomAttributeBool(const String& attribName, const bool);

			///Get the asset name for our world object preview asset, which
			///can be used for object placement or other non simulation visualization.
			String GetPreviewAssetName() const;

		public: // [ IXmlInterface ]

			virtual bool ReadXmlFromNode(const TiXmlNode* element)=0;
			virtual void WriteXmlToNode(TiXmlNode* pParentElement)=0;
		
		protected:

			const static float32 kTrackUpdateInterval;

		protected:

			///The mass of the combined vehicle components, in kg.
			float64 m_totalMass;

			///The various aspects of the vehicle's orientation
			float64 m_yaw;
			float64 m_pitch;
			float64 m_roll;		
			
			/// Stuff for UI Properties display
			// Total Torque applied to wheels on left side
			Units::ScalarTorque m_leftTorque;

			// Total Torque applied to wheels on Right side
			Units::ScalarTorque m_rightTorque;

			// Total Drawbar Pull on each side of vehicle
			Units::ScalarForce m_leftForce;
			Units::ScalarForce m_rightForce;

			Units::ScalarForce m_leftNormalForce;
			Units::ScalarForce m_rightNormalForce;
			Units::ScalarForce m_totalNormalForce;

			float64 m_rolloverMetric;

			// These are set in ODEVehicle
			Units::RadialSpeed        m_yawRate;
			Units::RadialSpeed        m_rollRate;
			Units::ScalarAcceleration m_lateralAcceleration;
			Units::AngularMomentum    m_angularMomentum;

			Units::Speed              m_forwardSpeed;
			Units::Speed              m_forwardSpeedTolerance;
			Units::Speed              m_lateralSpeedTolerance;

			// Acceleration in Vehicle XY, for Telemetry Support
			Vector2_MKS_types::Acceleration2D  m_accelerationXY;

			/// Computed Accelerations, used by Telemetry Code
			Vector3_MKS_types::Velocity      m_linearVelocityVCS;
			Vector3_MKS_types::Acceleration  m_linearAccelerationVCS;

			Units::Length m_wheelBase;
			Units::Length m_track;
			
			float64 m_frontalArea;     // in m^2
			float64 m_dragCoefficient; 

			float64 m_airResistance; // in newtons
			float64 m_vegetationResistance;

			///total forward driven distance
			float64 m_forwardTravelDistance;

			/// Total torque about Vehicle "Z" due to VTI forces:
			Units::ScalarTorque         m_totalVTITorque;

			/// Sum of VTI Resistance Forces on each side of vehicle (VCS)
			Vector3_MKS_types::Force    m_vtiResistanceForce[2];

			/// Total torque about Vehicle "Z" due to VTI Resistance
			/// Forces (Soil Compaction, Rolling Resistance, etc.)
			Units::ScalarTorque         m_vtiLateralResistanceTorque;

			/// Sum of all VTI forces, by side
			// of vehicle (left/right)(VCS)
			Vector3_MKS_types::Force    m_totalVTIForce[2];

			/// Scale factors applied to resistance forces
			/// prior to applying them to vehicle, usually 1.0 
			/// but may be less if resistance forces reduced
			/// to prevent them from accelerating vehicle
			/// (Defined for all three directions, both sides
			/// of vehicle)
			Vector3                     m_resistanceForceScale[2];

			/// Total Vehicle Momentum
			Vector3_MKS_types::Momentum m_momentumVCS;

			/// Scale factor applied to "Resistance Torques"
			/// (e.g. torques due to lateral forces on front
			/// and rear of vehicle), usually 1.0 but may
			/// be reduced to prevent radial acceleration
			/// due to these forces.
			float64                     m_resistanceTorqueScale;

			/// If m_limitResistanceForce==true (default),
			/// limit Forward Resistance forces (if necessary)
			/// to prevent ve3hicle from accelerating 
			/// in reverse
			///   m_limitResistanceForce[0] : limit forces in X
			///   m_limitResistanceForce[0] : limit forces in Y

			bool                        m_limitResistanceForce[2];

			/// if m_limitResistiveTorque==true (default),
			/// limit resistive torques on stationary vehicle
			bool                        m_limitResistiveTorque;

			//Our vehicle's navigation system, if we have one...
			INavigationSystem* m_pNavigationSystem;

			///Our vehicle's individual name
			String m_name;
			//the definition file for this vehicle
			String m_fileName;
			///The type of vehicle
			String m_type;

			///World object preview of this asset
			String m_previewAsset;
			
			///The sound information for this vehicle.
			SoundProfile m_soundProfile;

			///the unique id of this vehicle
			VehicleID m_vehicleID;
			/// ID of battery system
			VaneID m_batterySystemID;

			VehicleMotionState m_vehicleMotionState;
			VehicleMotionDirection m_vehicleMotionDirection;

			CustomAttributeMap m_customAttributeMap;

			// Vector of shared pointers to Speed Controller Factories
			MotorSpeedControllerFactorySharedPtrVector m_motorSpeedControllerFactories;

			// Vector of shared pointers to Electric motor prototypes,
			MotorPrototypeSharedPtrVector m_electricMotorPrototypes;
			
			// Vector of track renderable IDs
			std::vector<TrackRenderableID> m_trackRenderables;

			MotorSharedPtrContainer m_motors;
			VaneIdVector m_motorIDs;

			// Time since the track renderable positions were last updated
			float32 m_timeSinceTrackUpdate;
			
			///If we currently are using input to move the vehicle
			bool m_inputEnabled;
			///If we are being externally controlled, we don't need any
			///update calculations
			bool m_externallyControlled;

			bool m_enableVehicleParticleEffects;
			bool m_vehicleParticleEffectInitialized;

		};

	}
}

#endif // Vehicle_h__
