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
#ifndef QS_VANE_CONTROLLER_INTERFACE
#define QS_VANE_CONTROLLER_INTERFACE

#include "Core.h"
#include "VaneIDs.h"
#include "XmlInclude.h"

//Controller interface
#include <vector>
#include <string>

#if VANE_COMPILER == VANE_COMPILER_MSVC
//Disable warning about exporting an stl container in the dll class.
//IControllerFactory::m_ownedControllers is the guilty member
#	pragma warning( push )
#	pragma warning( disable: 4251 ) 
#endif

namespace VANE
{
	//forward declares
	namespace World { class Object; }
	class VehicleDataProvider;

	///namespace for all Controller related classes and routines
	namespace Controller
	{
		//////////////////////////////////////////////////////////////////////////
		// Typedefs 
		
		///ControlInputIndex is the index type for our control inputs
		///We can use an index to directly access controls instead of having to do a string
		///lookup
		typedef uint32 ControlInputIndex; 

		/// Control input values. 
		typedef float64 ControlValue;	

		/// The name of our controls
		typedef String ControlName;
		
		///ID for a controller
		typedef VaneID ControllerID;

		///Vector of controller IDs
		typedef std::vector<ControllerID> VehicleIDVector;

		///Using a controller type as a string now.
		///This allows easy extension...
		typedef String ControllerType;

		//////////////////////////////////////////////////////////////////////////
		// Enums 
		
		///The types of controls we are allowed
		enum ControlType
		{
			kControlTypeButton=0, ///< An on/off value (true/false) such as vehicle lights on
			kControlTypeAxis,     ///< A value that represents a range of values, such as 0-1, or -10 - 10.  Steering could be -1 to 1
			kNumControlTypes      ///< Count of control types
		};

		///Different modes for a controller interface to act with
		enum ControlMode
		{
			kControlModePassive = 0, ///< Standard controller interface mode, where the vehicle queries the controller interface
			kControlModeActive  = 1  ///< Mode where the controller interface updates its controllables directly
		};

		//[UNUSED] - Need to add this capability in.
		///The update method used by the controllable object
		enum ControllerUpdateType
		{
			kControllerUpdatePassive, ///< Controller is queried by vehicle
			kControllerUpdateActive   ///< Controller sets state on vehicle
		};

		//[UNUSED] - Need to add this capability in.
		enum ControllerMethod
		{
			kControllerInput, ///< Update only the input to the vehicle
			kControllerStatus ///< Update actual positions, orientations, etc
		};

		enum MotorControlAlgorithm
		{
			kOpenLoop = 0,  /// Input values to "Steering" and "throttle" values 
							/// passed to underlying motor controller object
							/// are treated as having only relative meanings
							/// by the controller (more speed/less speed)
			kClosedLoop     /// Input Values to Motor Controller are taken
							/// to represent target values for vehicle
							/// speed and yaw rate, controller will adjust
							/// actual motor powers to achieve targets
		};

		//////////////////////////////////////////////////////////////////////////
		// Constants           
		
		///An invald controller ID
		const uint32 kInvalidController = 0xFFFFFFFF;

		///Default control value
		const ControlValue kDefaultControlValue = 0.0f;

		//////////////////////////////////////////////////////////////////////////
		//
		//  ControlInput
		//
		//////////////////////////////////////////////////////////////////////////
		
		///A Control input is a control type with a string name
		///The vehicle defines which inputs it accepts, and their type
		struct ControlInput
		{
			ControlInput() 
			{
				;
			}

			ControlInput(const ControlName& name, ControlType type) 
			: m_type(type), m_name(name)
			{

			}

			///Our type of control
			ControlType m_type;
			///the name of our control
			ControlName m_name;
		};

		//////////////////////////////////////////////////////////////////////////
		//
		//  IControllable
		//
		//////////////////////////////////////////////////////////////////////////
		
		///Interface to allow a Controller to actively control a vehicle
		class VaneCoreExport IControllable
		{
		public:
			///Get a list of controls this controllable object supports
			///@return 
			virtual std::vector<ControlInput> GetControls() const = 0;
			
			///Set the input value for the given control name
			///@param[in] name The control name to update
			///@param[in] value The current value for this control 
			virtual void SetInput(const ControlName& name, ControlValue value) = 0;
			
			///Get the WorldObject for this IControllable
			///@return the WorldObject for this IControllable
			virtual World::Object* GetWorldObject() const = 0;

			///Return an id by which we can determine type and
			/// instance of the controlled object, which is necessary
			/// for re-grabbing an IControllable object across networks
			/// or between sim runs, etc.
			virtual VaneID GetInstanceId() const = 0;
		};

		///Vector of controllables
		typedef std::vector<IControllable*> ControllableList;

		//Note! should we have some controller interface that can only act as an active controller?
		//it would make the interface much simpler as it would only take controls
		//we could query what types of controls the vehicle accepts first and then
		//use whatever ones overlap

		//////////////////////////////////////////////////////////////////////////
		//
		//  ControllerInterface
		//
		//////////////////////////////////////////////////////////////////////////
		
		///Defines an interface to allow almost anything to control any other
		///system that uses input values which can be represented as name value
		///pairs of String / float64.
		///Normally, the controlled object ( vehicle, AI, etc) requests input 
		///from the controller interface with a string or an index. 
		///If the Controller interface has input for that
		///value, it will return the current value for that input.
		class VaneCoreExport ControllerInterface
		{
		public:
			virtual ~ControllerInterface() {;}
			
			///Update our Controller
			///@param[in] dt Delta time since last update
			///@return 
			virtual void Update(TimeValue dt) = 0;
			
			///Callback notification that a Controller has been attached to a vehicle
			virtual void OnAttachedToObject( VaneID objectID ) { VANE_UNUSED( objectID ); }
			
			///Get the input with the given index
			///@param[in] index The control index for this controller
			///@return the value for the control with the given index
			virtual ControlValue GetInput(ControlInputIndex index) const =0;

			///Get an input value with the given name
			///@param[in] name The name of the control to retrieve
			///@return the value for the control with the given name
			virtual ControlValue GetInput(const ControlName& name) const =0;

			///Get the complete set of input values for this controller
			///@Note: If you need names and values, call GetControls()
			///@return the complete set of input values for this controller
			virtual std::vector<ControlValue> GetInputs() const =0;

			///Clear the entire set of input values for this controller
			virtual void ClearInputs() = 0;

			///Get the complete set of inputs ( with names and values) for this controller
			///@Note: If you need just the values, and not the names, call GetInputs()
			///@return the complete set of inputs ( with names and values) for this controller
			virtual std::vector<ControlInput> GetControls() const = 0;

			///Set the mode for this Controller
			///@see ControlMode
			///@param[in] mode the mode for this controller to use
			virtual void SetControlMode(ControlMode mode) { VANE_UNUSED(mode); }

			///Add a controllable entity to this controller. This is used when
			///a controller should inject its values into the controllable, instead
			///of the normal means of operation where the object queries a ControllerInterface
			///for particular values.
			///@param[in] controllable The object that will be controlled directly
			virtual void AddControllable( VaneID controllableID ){ VANE_UNUSED(controllableID); };

			///TODO: Remove this method as it is obsolete now that Vehicles are exported
			///in the Simulation dll.
			///Check whether or not this controller needs a vehicle data provider
			///@return true if the controller requires a vehicle data provider
			virtual bool NeedsVehicleDataProvider() const {return false;}
			
			///TODO: Remove this method and replace it with something involing Vehicles
			///@param[in] pDataProvider the provider of vehicle data used for this controller
			///@return 
			virtual void SetVehicleDataProvider(VehicleDataProvider* pDataProvider) { VANE_UNUSED(pDataProvider); } 

			///Get the type of Controller this is.
			virtual ControllerType GetType() const =0;

			///Get the ID for this controller
			virtual ControllerID GetControllerID() const=0;
			
			//Get the base Controller ID for this controller
			virtual ControllerID GetBaseControllerID() const=0;

			// Assume that underlying motor controller uses "Open Loop" algorithm,
			// meaning that "throttle" and "steering" values passed to it have only
			// relative meanings (more speed/less speed, not specific speed)
			virtual MotorControlAlgorithm GetMotorControllerAlgorithm() const {return kOpenLoop;};

			// Set Algoritm (Open Loop/Closed Loop) used by underlying motor controller
			virtual void SetMotorControllerAlgorithm(MotorControlAlgorithm controllerType)  {VANE_UNREFERENCED_PARAM(controllerType);};

		};

		///SharedPtr to a controller.
		typedef SharedPtr<ControllerInterface> ControllerPtr;

		///A map for fast storage of controller ptrs
		typedef std::map<ControllerID, ControllerPtr> ControllerPtrMap;

		//////////////////////////////////////////////////////////////////////////
		//
		//  IControllerFactory - Factory for creating controllers    
		//
		//////////////////////////////////////////////////////////////////////////
		
		/// Factory for creating and tracking a ControllerInterface. It is expected
		/// that any created controller interfaces are deleted by the factory itself
		/// at some reasonable point.
		class VaneCoreExport IControllerFactory
		{
		public:
			/// Create a Controller and get its ID
			///@return The id of the created controller (kInvalidVaneID on failure)
			virtual ControllerID CreateControllerInterface() = 0;
			/// Get the type of the factory and it's created controllers
			virtual ControllerType GetControllerType() const = 0;
			/// Get the typeId of the factory and it's created controllers
			virtual ControllerID GetControllerTypeId() const = 0;
			/// Destroy the specified controller
			///@param[in] id The id of the controller to be destroyed
			virtual void DestroyControllerInterface(ControllerID id);

			/// This function can be expected to be called often;
			/// implement it as close to O(1) as possible.
			///@param[in] id The id of the controller being recovered.
			///@return A SharedPtr to the controller; note that this
			///        forces the factory itself to keep it as a SharedPtr
			///        lest the returned ptr ends in the controller's destruction.
			virtual ControllerPtr GetController(ControllerID id);

			///Write sufficient properties to fully recreate the controllers
			/// managed by this factory
			///@paran[in] pParent The xml element to attach all properties to.
			virtual void SerializeControllers(TiXmlElement* pParent) const = 0;
			///Read properties (as written by the complementing function) and
			/// fully recreate all controllers.
			virtual void DeserializeControllers(const TiXmlElement* pParent) = 0;

			virtual ~IControllerFactory() {}

		protected:
			ControllerPtrMap m_ownedControllers;
		};

	} //Namespace Controller
} //Namespace VANE

#if VANE_COMPILER == VANE_COMPILER_MSVC
#pragma warning( pop )
#endif

#endif
