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
#ifndef VANE_World_Manager_h__
#define VANE_World_Manager_h__

#include "Core/VaneException.h"
#include "Core/Commands.h"
#include "Core/XmlInclude.h"
#include "Core/XmlUtils.h"
#include "Core/Property.h"
#include "Core/CommandManager.h"
#include "Core/DataTypeManager.h"
#include "Core/EnumDataType.h"

#include "Simulation/Paths.h"
#include "Simulation/SelectionManager.h"
#include "Simulation/Physics/PhysicsTypes.h"
#include "Simulation/ParticleSystemTracker.h"

#include "WorldObjectRepFactory.h"
#include "WorldObjectRepresentation.h"
#include "WorldObject.h"

#include "WorldManagerListener.h"

typedef VANE::SharedPtr<const TiXmlElement> TiXmlElementPtr;

//@TODO - Store objects in a better way than a string, objectPtr map. Freelist with ObjectIDs as index might be nice

namespace VANE
{
	//The types that this system defines
	namespace Types
	{				
		extern VaneSimExport VaneID WorldManager;
	}

	namespace Commands
	{
		extern VaneSimExport const char* kWorldScope;

		extern VaneSimExport CommandID kCommandLoadObject;
		extern VaneSimExport CommandID kCommandAddObjDefFile;
		extern VaneSimExport CommandID kCommandClearObjects;
		extern VaneSimExport CommandID kCommandSetObjPosition;
		extern VaneSimExport CommandID kCommandMoveObjRelative;
		extern VaneSimExport CommandID kCommandCreateFolderObj;
		extern VaneSimExport CommandID kCommandMoveToNewFolderObj;
		extern VaneSimExport CommandID kCommandSelObjectsMoved;
		extern VaneSimExport CommandID kCommandSelObjsToggleVis;
		extern VaneSimExport CommandID kCommandAllObjsVisOn;
		
		extern VaneSimExport CommandID kCommandRotateObject90;
		extern VaneSimExport CommandID kCommandRotateObject180;
		extern VaneSimExport CommandID kCommandRotateObject270;
		extern VaneSimExport CommandID kCommandSetObjectOnGround;
		extern VaneSimExport CommandID kCommandGetPath;
		extern VaneSimExport CommandID kCommandGetPathProperties;
		extern VaneSimExport CommandID kCommandRenamePath;

		extern VaneSimExport CommandID kCommandSetSelectedObjectsOnGround;
		extern VaneSimExport CommandID kCommandSetSelectedObjectsOnTerrain;
		extern VaneSimExport CommandID kCommandSetSelectedObjectsOnObject;

		extern VaneSimExport CommandID kCommandOrientSelectedOnGround;

		extern VaneSimExport CommandID kCommandReplaceObject;

		//Selection Commands
		extern VaneSimExport CommandID kCommandWorldSelectionRemove;
		extern VaneSimExport CommandID kCommandWorldSelectionAdd;
	}

	///A namespace including all World Object classes.  
	namespace World
	{
		extern VaneSimExport const String kPlainObjectName;
		extern VaneSimExport const char*  kXmlAssetRepresentationsChild;

		class ParticleSystemObject;

		typedef std::pair<Physics::RayCollisionResult, World::Object*> ObjectCollisionResult;

		///System wide Initialization   
		void VaneSimExport Initialize();

		///System wide shutdown
		void VaneSimExport Shutdown();

		///The current format of the Object Definition
		///This is a TiXmlElement* for now, but we may make a better
		///format for this in the future, so that we are not restricted to
		///holding on to XmlDocs all over the place.
		typedef const TiXmlElement* ObjectDefinitionType;
		const ObjectDefinitionType kInvalidObjDef = NULL;

		const char kParticleObjectTag[15] = "particleObject";

		/************************************************************************/
		/* Asset Support                                                        */
		/************************************************************************/

		///The params for world objects; contains XML for physics and representations,
		/// as well as default position/orientation/scale.
		struct WorldObjectStaticAssetParams : public StaticAssetParams
		{
			//To set these for a specific object, create the object, get its properties,
			//and set those.
			Quaternion m_initialOrientation;
			Vector3    m_initialPosition;
			Vector3    m_initialScale;
			
			String m_typeName;

			//These are difficult to structify without another system for dynamic params
			//for representations/physics. So we're just dealing with them as XML for now.
			TiXmlElementPtr m_pPhysicsDefinition;
			TiXmlElementPtr m_pRepresentationDefinition;
			
			ZBehavior m_zBehavior;
			ObjectCategory::Category m_objectCategory;

			bool m_isSelectable;
			bool m_isScalable;
			bool m_isTerrain; //if this should be treated as terrain
			bool m_isVirtual; //if this object is a simulation abstraction, such as a waypoint.
		};

		///The params for world objects; contains XML for physics and representations,
		/// as well as default position/orientation/scale.
		struct ParticleObjectStaticAssetParams : public WorldObjectStaticAssetParams
		{
			//Affectors? Particle physics? etc.
		};

		//////////////////////////////////////////////////////////////////////////
		//
		//  WorldManager
		//
		//////////////////////////////////////////////////////////////////////////
		
		///The world object manager is one of the most important classes in VANE
		///It has a reference to all objects in the world and is also responsible
		///for creating the representations of the objects through various factories.
		///Factories must register themselves with the world manager and are then assigned 
		///a type ID that will be used to classify the representation and all parameters relating
		///to this representation type
		///The World Manager also is a global access point to all World objects
		class VaneSimExport Manager : 
			public VANE::Singleton<VANE::World::Manager>,
			public IPropertyProvider,
			public IAssetHandler,
			public CommandSelectionListener,
			public IXmlInterface
		{
		public:
			///Create a manager 
			Manager();
			~Manager();

			void DestroyEverything();

			///Create a world object.  The ONLY way to create a new world object. The world
			///object is owned by the World::Manager so this pointer should not be deleted.
			///To destroy an object, call World::Manager::DestroyObject()
			///@param name The unique name of this world object
			///@param parent The parent of this object (if NULL, will use root object)
			///@param position The initial position of this world object
			///@param orientation The initial orientation of this world object
			///@param queryFlags The selection / type flags for this object
			///@return A new Object pointer
			Object* CreateObject(
				const String& name, 
				Object* parent,
				ObjectCategory::Category objCategory,
				const Vector3 & position = Vector3::Zero(),
				const Quaternion& orientation = Quaternion::GetIdentity(),
				const Vector3 & scale = Vector3::UnitScale(),
				unsigned int queryFlags = Selection::kWorldObjectFlag,
				bool repressNotification = false,
				bool isVirtual = false
				);

			Object* CreateObjectOfType(
				const String& type,
				const String& name,
				ObjectCategory::Category objCategory,
				Object* parent,
				const Vector3 & position = Vector3::Zero(),
				const Quaternion& orientation = Quaternion::GetIdentity(),
				const Vector3 & scale = Vector3::UnitScale()
				);

			void SetDefaultObjectType(const String& defType) { m_defaultObjDef = defType; }

			Object* CreateObjectOfDefaultType(
				const String& name,
				Object* parent,
				const Vector3 & position = Vector3::Zero(),
				const Quaternion& orientation = Quaternion::GetIdentity(),
				const Vector3 & scale = Vector3::UnitScale()
				)
			{
				Object* newObj = CreateObjectOfType(m_defaultObjDef, name, ObjectCategory::Generic, parent, position, orientation, scale);
				return  newObj;
			}

			///Return true if the string given is a create-able type
			bool IsTypeValid(const String& typeName)
			{
				return m_objDefs.find(typeName) != m_objDefs.end();
			}

			///Add all objects with the same object/asset type to the
			/// list of selected objects
			void SelectObjectsMatchingType(VaneID objId);

			/// Create a path object
			///@param[in] name The name of the new path  
			///@param[in] pParent The hierarchical parent of the new path 
			///@param[in] position The position in world space of the new path
			///@param[in] orientation The orientation relative to the world axis 
			///@param[in] queryFlags The selection / type flags for this object
			///@return A pointer to the new path object
			PathPtr CreatePathObject( const VANE::String& name, Object* pParent, const VANE::Vector3 & position = VANE::Vector3::Zero(), const VANE::Quaternion& orientation = VANE::Quaternion::GetIdentity(), unsigned int queryFlags = VANE::Selection::kWorldObjectFlag );

			void AddObject( Object* pObj, Object* pParent );

			///Create our root world objects, which will parent all objects in the scene
			void CreateRootObjects();

			///Notify that an object has moved
			void ObjectMoved( Object* pObj );
			
			///Get the Root world object
			const Object* GetRootObject();
			
			///Get the ID of the root world object
			WorldObjectID GetRootObjectID();

			///Get the next WorldObjectID (used during object creation)
			WorldObjectID GetNextObjectID();

			///Get the next PathID (used during path creation)
			PathID GetNextPathID();

			///Destroys an object and removes it from the world manager.
			///@param obj The object to destroy
			void DestroyObject(Object* obj);

			///Destroys an object and removes it from the world manager.
			///@param id ID of the object to destroy.
			void DestroyObject( WorldObjectID id );

			///Destroys a path and removes it from the world manager.
			///@param path The path to destroy.
			void DestroyPath(PathPtr path);

			///Destroys a path and removes it from the world manager.
			///@param id The ObjectID or PathID of the path to destroy.
			void DestroyPath( VaneID id );

			///Destroy all paths.
			void DestroyAllPaths();

			/// Retrieve a path.
			///@param[in] id The ObjectID or PathID of the path to retrieve.
			///@return A pointer to the Path specified.
			PathPtr GetPath( VaneID id );

			/// Get the path owning a path point
			///TODO: This would be very good to optimize in some way...
			PathPtr GetPathOwningPathPoint(VaneID pathPointId);

			/// Retrieve all paths.
			///@return A vector containing all paths.
			std::vector<PathPtr> GetAllPaths() const;

			/// Get a world object by name
			///@param name The unique name of the object
			///@return NULL if not found, otherwise the object with that name
			Object* GetWorldObject(const String& name);

			/// Get object by WorldObjectID
			///@param id The WorldObjectID of the world object to find
			///@return NULL if not found, otherwise the object
			///@note Since the map uses strings, this may be less efficient than
			///removing by name
			Object* GetWorldObject(WorldObjectID id);

			///Get a collection of every object ID
			///@return Complete set of object IDs registered
			const ObjectIDVector GetObjectIDs();

			///Get a collection of every object name
			///@return complete set of object names
			const StringVector GetObjectNames();

			/// Attach an object onto another object.
			/// @param[in] attachingID Attaching object ID.
			/// @param[in] destID ID for the object being attached to.
			void AttachObjectToObject( WorldObjectID attachingID, WorldObjectID destID );

			/// Get all world objects within a certain a range from a point that are registered to the world manager.
			///@param[in] origin The center of the area to search.
			///@param[in] radius The radius of the area to search.
			///@return A vector of pointers to Object within the specified area.
			std::vector<Object*> GetObjectsInRange(const Vector3& origin, float64 radius);

			void SetWorldObjectOnGround(World::Object* pObj);


			//Serialization

			///Create an Object from an XmlElement
			///@param element the Xml Element we will use to fill out the information 
			///of this world object
			///@returns the new WorldObjectID if it could parse this, kInvalidObjectID if
			///it could not create this object
			WorldObjectID CreateObjectFromXml(const TiXmlElement* element);

			///Serialize an object to an XmlElement inside the given node
			///@param node The parent node of the new element we will create
			///@return true if it could serialize, false if it could not
			bool SerializeObjectToXml(World::Object* pObject, TiXmlNode* pNode);

			///Create an xml file describing the current world state
			void SerializeWorldState();


			//Object Representations

			///This method allows us to externally set the implementation type so that
			///the world object manager does not need to know about the specifc object system 
			///upon compilation
			void RegisterObjectRepFactory(ObjectRepresentationFactory* factory);

			///Remove the object representation factory for this type of object
			///@param type The type of the factory to remove.  Since we can only have one factory
			///per type, this is is a simpler way of removal as opposed to passing in the pointer
			void RemoveObjectRepFactory(ObjectRepresentationType type);

			///Parse an xml object representation node
			///params[in] pObject the world object we are parsing representations for
			///params[in] pNode the node that contains all of the xml definitions for object representations for this object
			void ParseObjectRepresentations(World::Object* pObject, const TiXmlNode* pNode);

			///Add an object representation to this object based on the type of parameters passed in
			///If no factory corresponds to this type, or a failure occurs, NULL is returned
			///@param obj The object to attach the representation to
			///@param params The 
			///@return 
			ObjectRepresentation* AddObjectRepresentation(Object* obj, const ObjectRepresentationParams& params);

			///This function is called by the Selection manager when we are trying to 
			///do a terrain ray query
			Selection::TerrainSelectionResult* TerrainSceneQuery( const Ray& ray);

			Selection::ObjectSelectionResult* ObjectSceneQuery( const Ray& ray);

			ObjectCollisionResult ObjectPhysicsQuery( const Ray& ray, World::Object* pIgnoreObject = NULL, float64 rayLength = Physics::kDefaultRayLength );

			///Get a const iterator so we can loop through the objects externally
			///@return A const iterator to the beginning of the objects
			ObjectMap::const_iterator GetObjectIterator();

			///The const_iterator for the end of the object container structure
			///Used with GetObjectIterator() to provide the end of the loop
			ObjectMap::const_iterator GetObjectEnd();
			
			
			/// Rename an object
			///@param[in] id
			///@param[in] newName
			///@return true if we can rename this object, false otherwise.
			bool RenameObject( WorldObjectID id, const String& newName);
			
			/// Rename a path
			///@param[in] id
			///@param[in] newName
			///@return 
			bool RenamePath( PathID id, const String& newName );

						
			//Object Listeners

			///Add A listener that will receive updates about Object Creation and deletion
			///@param listener The listener to add
			void AddListener(WorldManagerListener* listener);

			///remove listeners
			///@param listener The listener to remove
			void RemoveListener(WorldManagerListener* listener);

			///Remove all listeners
			void ClearListeners();
			
			///Add A listener that will receive updates about Object Creation and deletion
			///@param listener The listener to add
			void AddPathListener(IPathCreationListener* pListener);

			///remove listeners
			///@param listener The listener to remove
			void RemovePathListener(IPathCreationListener* pListener);

			//Updates

			///When an object needs to be updated by some external means, such as an external
			///controller, we can add it to the update list and the manager will then call
			///UpdateFromController on each object in the list.  If the controller is destroyed
			///or the object is destroyed it should be removed from this list
			void AddObjectToUpdateList(Object* obj);

			///Remove an object from the list of objects that get updated automatically by this 
			///manager. It will no longer get UpdateFromController called on it by this manager
			///in Manager::Update
			void RemoveObjectFromUpdateList(Object* obj);

			///Update all world object related tasks, such as calling UpdateFromController on
			///objects in the updateList.  This should be called once per step, not once per render
			void Update(TimeValue dt);

			//Maybe expose these as commands eventually.
			/// Add a point to the path currently being created.
			///@param[in] point Three dimensional point to add to the path.
			void AddPointToCreatedPath(const Vector3& point );

			/// Save the path currently being created.
			void SaveCreatedPath(bool silently = false);

			void SavePath( PathPtr pPath, bool silentCreation = false );

			/// Destroy the path currently being created.
			void DestroyCreatedPath();

			inline EnumID GetPathEnumTypeID() { return m_pathEnumTypeID; }

			/// Figure out if the user is trying to save/destroy
			void PathKeyboardLogic();

			//Object Type Defintions

			///Register an object definition
			///This will be used anytime an object is created by this type
			///This allows us to just load in the definition once, keep it around,
			///and whenever someone needs to create an object, they can do so by
			///specifying the object type name, and the defintion will be looked up from
			///this system
			///@param[in] type The unique type name that will be registered, such as "PalmTree1"
			///@param[in] pObjectDefn The Xml Element where the object definition is stored
			///NOTE: it is expected that this XmlElement will be valid for the duration of the
			/// simulation so that more objects can be created at any time.
			void RegisterObjectDefinition(const String& type, ObjectDefinitionType pObjectDefn);

			///Load and parse an object definition file. This will be kept in memory
			///for the duration of the simulation, and get referenced when creating
			///objects defined in this file.
			///param[in] fileName the filename of the Xml object definition file to parse
			///@param[in] systemDefn Whether the definition is loaded with the system configuration;
			///                      If it is a system-specific definition, it is assumed the simulation can
			///                      change, and definition will still be available.
			void LoadXmlObjectDefinitionFile(const String& fileName, bool systemDefinition = false);

			void LoadXmlWorldObjectDefinitionFile(TiXmlElement* pRootElement, bool systemDefinition);

			///Get all the registered object definition names
			///@return A complete list of all the Object type names that have
			///been registered at this point
			StringVector GetObjectTypeNames();

			///Lookup a definition based on the passed in type name
			///@param[in] type The name of the registered object type you wish
			///to get the definition for
			///@return The definition of this type if it is a type that has
			///already been registered, otherwise kInvalidObjDef
			ObjectDefinitionType GetObjectDefinition(const String& type);

			///Get the default asset preview filename/path
			///@return The path to the default preview file.
			String GetDefaultAssetPreview() { return m_defaultAssetPreview; }

			///Parse an xml object definition and apply its properties to the newly created
			///object. A definition includes physics and rendering properties, and extra info
			///necessary for additional modules, such as sound.
			void ParseXmlObjectDefinition(Object* pObject, const TiXmlElement* pDefinition);

			///Parse the physics section of an object definition
			void ParseObjectPhysics(Object* pObject, const TiXmlElement* pPhys );
			
			/// Build an id set for world objects using their IDs, names, and an invalid ID.
			/// @return The vane ID set for the current world objects.
			VaneIdSet GetObjectIdSet(bool excludeVehicleParts = false) const;
			
			///Returns an id set for our interesing world objects, such as user placed vehicles
			///and objects.
			VaneIdSet GetInterestingObjectIdSet() const;
			
			/// Get and id set for paths
			///@return 
			VaneIdSet GetPathIdSet() const;

			// Particle System Tracker functions

			ParticleSystemTrackerPtr CreateParticleSystemTracker( WorldObjectID objectID );
			
			//Particle object functions

			/// Create a particle system world object
			///@param[in] name The name of the new object
			///@param[in] pParent The hierarchical parent of the new object
			///@param[in] position The position in world space of the new object
			///@param[in] orientation The orientation relative to the world axis
			///@param[in] queryFlags
			///@return A pointer to the new particle system object
			ParticleSystemObject* CreateParticleSystemObject( const VANE::String& name, Object* pParent, const VANE::Vector3 & position = VANE::Vector3::Zero(), const VANE::Quaternion& orientation = VANE::Quaternion::GetIdentity(), unsigned int queryFlags = VANE::Selection::kWorldObjectFlag );

			/// Create a new particle system world object based on a defined type
			///@param[in] type The defined type of the new object
			///@param[in] name The name of the new object  
			///@param[in] pParent The hierarchical parent of the new object 
			///@param[in] position The position in world space of the new object
			///@param[in] orientation The orientation relative to the world axis
			///@return A pointer to the new particle system object
			ParticleSystemObject* CreateParticleSystemObjectOfType( const VANE::String& type, const VANE::String& name, Object* pParent, const VANE::Vector3 & position = VANE::Vector3::Zero(), const VANE::Quaternion& orientation = VANE::Quaternion::GetIdentity());

			/// Parse the XML details of a particle system's representation
			///@param[in] pObject The particle system object to affect
			///@param[in] pNode The XML node to parse
			void ParseParticleSystemObjectRepresentations( Object* pObject, const TiXmlNode* pNode );

			/// Parse the XML details of a particle system's physics attributes
			///@param[in] pObject The particle system object to affect
			///@param[in] pNode The XML node to parse
			void ParseParticleSystemObjectPhysics(Object* pObject, const TiXmlElement* pPhys );

			///Parse an xml object definition and apply its properties to the newly created
			///object. A definition includes physics and rendering properties, and extra info
			///necessary for additional modules, such as sound.
			///@param[in] pObject The particle system object to affect
			///@param[in] pNode The XML node to parse
			void ParseXmlParticleSystemObjectDefinition(Object* pObject, const TiXmlElement* pDefinition);

			///Load and parse a particle system object definition file. This will be 
			///kept in memory for the duration of the simulation, and get referenced 
			///when creating objects defined in this file.
			///param[in] fileName the filename of the Xml object definition file to parse
			///@param[in] systemDefn Whether the definition is loaded with the system configuration;
			///                      If it is a system-specific definition, it is assumed the simulation can
			///                      change, and definition will still be available.
			void LoadXmlParticleSystemObjectDefinitionFile( TiXmlElement* root, bool systemDefinition = false );

			/// Load the sensor configurations from the xml file
			///@param[in] root The root of the xml file
			///@param[in] systemDefinition Whether or not the definitions are loaded with the main init file
			void LoadXmlSensorObjectDefinitionFile( TiXmlElement* pRoot, bool systemDefinition = false );

			/// Load the ai configurations from the xml file
			///@param[in] root The root of the xml file
			///@param[in] systemDefinition Whether or not the definitions are loaded with the main init file
			void LoadXmlAIObjectDefinitionFile(TiXmlElement* pRoot, bool systemDefinition = false );

			/// Check to see if large selection text is active.
			/// @return True if large selection text is active, false if not.
			bool LargeSelectionTextActive( ) const { return m_showLargeSelectionText; }

			/// Set the position of the large selection text.
			/// @param[in] textPosition The position we want the text to display at.
			void SetLargeSelectionTextPosition( const Vector3& textPosition ) { m_largeSelectionTextPosition = textPosition; m_updateLargeSelectionText = true; }

			///Get the set of use surfaces in the current world
			std::vector<SurfaceID> GetUsedSurfaces();

		public: //[IPropertyProvider methods]
			virtual void OnPropertyChanged(VaneID objId, PropertyIndex id);
			virtual ObjectPropertySet GetProperties(VaneID objID);
			virtual VaneIdVector GetIdsOfType(const VaneID dataType) const;

			PropertyIndex GetPositionPropIndex() const { return m_positionPropIndex; }
			PropertyIndex GetOrientationPropIndex() const { return m_orientationPropIndex; }

		public: //[IXmlInterface]
			virtual bool ReadXmlFromNode( const TiXmlNode* pElement );
			virtual void WriteXmlToNode( TiXmlNode* pParentElement );

		public: //[CommandSelectionListener]
			void OnCommandSelectionUpdate();
			bool ValidateNewCommandSelection();

			void SelectChildren(Object& curObject, VaneIdVector& selectedObjects);

		public: //[IAssetHandler] - see DataTypeManager.h for comments
			VaneID CreateAssetInstance(const StaticAssetParamsPtr& pStaticParams, VaneID dataType, const DynamicAssetParamsPtr& pDynParams);
			void DestroyAssetInstance(VaneID objId);
			StaticAssetParamsPtr ParseXmlToBinary(const TiXmlElement* pXmlParams, VaneID dataType);
			VaneIdVector GetHandledDataType() const;
			VaneID GetMainWorldObjectId(VaneID id);
			VaneID GetInstanceUsingMainWorldObject(VaneID id);

			//Different functions for different types
			void ParseWorldObjectXmlToBinary(const TiXmlElement* pXmlParams, WorldObjectStaticAssetParams* pOutput);
			void ParseParticleSystemXmlToBinary(const TiXmlElement* pXmlParams, ParticleObjectStaticAssetParams* pOutput);

		protected:
				
			void UpdateParticleSystemTrackers( TimeValue deltaTime );

		protected:
			friend class DataTypeManager;
				
			///Called to get the next representation type ID available when
			///assigning a typeID to a representation factory
			ObjectRepresentationType GetNextRepresentationTypeID();

			///Notify all of our listeners about an object creation
			void NotifyAllCreation(Object* obj);
			
			///Notify all of our listeners about and object
			void NotifyAllDelete(Object* obj);

			///Notify all of our listeners about an object attachment.
			void NotifyAllAttach(Object* obj);

			///Notify all of our listens about an object moved
			void NotifyAllMoved(Object* obj);

			//Notify all of our listeners about an added asset
			void NotifyAllAssetAdded(const String& name);
		protected:
			
			PropertyGroupInstance GetWorldObjectProperties( Object& pWorldObject );
			void                  RegisterWorldObjectProperties();
			PropertyGroupInstance GetWorldManagerProperties( );
			void                  RegisterWorldManagerProperties();			
			PropertyGroupInstance GetPathProperties( Path& path );
			void                  RegisterPathProperties();
		protected:
			//Internal function used for object renaming. Not useful during normal operation.
			String  GetRegisteredName( WorldObjectID id );
			
		protected:
			
			///The current total count of objects created
			///used when assigning IDs to new objects
			uint32 m_objectIDCount;

			///The current total count of paths created
			///used when assigning IDs to new paths
			uint32 m_pathIDCount;

			///All of our objects
			ObjectMap m_objects;

			///All of our paths
			std::vector<PathPtr> m_paths;

			///The Path to which we are creating/adding points
			PathPtr m_pCreatedPath;

			///An EnumType so that we can maintain an Enum with all current paths for nice
			///  and easy display in the properties tab.
			EnumID m_pathEnumTypeID;

			///The list of objects that need to be updated by their corresponding controller
			std::list<World::Object*> m_updateList;

			///The listeners that will receive notifications about object creations and deletions,
			///so they can either clear their representation or remove their associated data
			std::vector<WorldManagerListener*> m_listeners;
			std::vector<IPathCreationListener*> m_pathListeners;

			//representation data

			///The current count of representation types created
			///Used when creation new representation types and assigning them to factories
			ObjectRepresentationType m_repTypeCount;

			///A map of representation types the their associated factories
			typedef std::map<ObjectRepresentationType, ObjectRepresentationFactory*> ObjectFactoryMap;
			///A map of strings to their associated factories, useful for xml parsing, etc.
			///ogre3d would point to the Ogre3d Factory, etc.
			typedef std::map<String, ObjectRepresentationFactory*> ObjectFactoryNameMap;
			///an entry in the ObjectFactoryName map
			typedef std::pair<String, ObjectRepresentationFactory*> ObjectFactoryNameMapEntry;
			
			///Our factories mapped by representation type
			ObjectFactoryMap m_factories;

			///Our factories mapped by name
			ObjectFactoryNameMap m_factoryNames;
			
			//Object definition file related data
			struct ObjectDefinitionFileInfo
			{
				String m_fileName;
				TiXmlDocumentPtr m_pXmlDoc;
				uint32 m_numObjectDefinitions;
			};

			//list of object definition files (smart pointer to keep them around)
			typedef std::vector<ObjectDefinitionFileInfo> ObjectDefinitionFileList;
			//our object definition files
			ObjectDefinitionFileList m_objDefFiles;

			//mapping of object type names to object definitions
			typedef std::map<String, ObjectDefinitionType> ObjectDefinitionMap;
			ObjectDefinitionMap m_objDefs;

			//The default obj definition to use
			String m_defaultObjDef;

			//The default particle system object definition to use
			String m_defaultPartSysObjDef;

			//The default preview to show for assets/definitions
			//NOTE: This is not in any way related to m_defaultObjDef
			String m_defaultAssetPreview;

			//the object from which all are parented.
			Object* m_pRootObject;

			//object from which all virtual objects are parented
			Object* m_pRootVirtualObject; 
			VaneID  m_rootVirtualObjectID;

			//root path object, which is a child of the root virtual object
			Object* m_pRootPathObject;
			VaneID  m_rootPathObjectID;
			
			//The registered property index for various properties.
			PropertyIndex m_positionPropIndex;
			PropertyIndex m_orientationPropIndex;
			PropertyIndex m_scalePropIndex;
			PropertyIndex m_propertyName;
			PropertyIndex m_pathName; 

			ParticleSystemTrackerList m_particleSystemTrackers;

			Vector3 m_largeSelectionTextPosition;
			bool m_showLargeSelectionText;
			bool m_updateLargeSelectionText;

			///Our specialized CommandGroup for working with VehicleMgr functions
			///this will not need a public interface so it is declared as an internal
			///class.  This handles all of the command management for the VehicleManager
			class WorldCommandGroup : public CommandGroup
			{
			public:
				///Create a vehicle command group with a given vehicle mgr
				///NOTE: Even though Simulation Manager is a singleton, we still want
				///to verify that it actually has been created, and we want to have
				///the option to use it without requiring it to be a Singleton
				///@param[in] mgr The manager to use
				WorldCommandGroup(World::Manager& mgr);

				///Handle commands specific to the Simulation manager 
				///@param[in] commandID The ID of the command to handle
				///@param[in] parameterList The parameters to use for this command
				///@return A commandresult describing the success or failure of this command
				CommandResult HandleCommand( CommandID commandID, const CommandParamList& parameterList);

				/// Get a list of possible values for each parameter for the specified command.
				/// NOTE: Not all commands or parameters may have choices and can either be left blank
				/// or assigned a value of Variant::INVALID.
				/// @param[in] commandID The command to get possible parameter choices for.
				CommandParamList GetParameterChoices( CommandID commandID );
			protected:
				
				/// Take all selected objects and apply a ground placement to them.
				void PlaceSelectedObjectsOnGroundMode( ZBehavior mode );

				//our vehicle manager reference
				World::Manager& m_mgr;
			};

			WorldCommandGroup m_commandGroup;

			class SelectionCommandGroup : public CommandGroup
			{
			public:

				SelectionCommandGroup(World::Manager& mgr);

				CommandResult HandleCommand( CommandID commandID, const CommandParamList& parameterList);

				/// Get a list of possible values for each parameter for the specified command.
				/// NOTE: Not all commands or parameters may have choices and can either be left blank
				/// or assigned a value of Variant::INVALID.
				/// @param[in] commandID The command to get possible parameter choices for.
				CommandParamList GetParameterChoices( CommandID commandID );

			protected:
				//our vehicle manager reference
				World::Manager& m_mgr;

			} m_selectionCommandGroup;
		};
	}
}


#endif // Manager_h__
