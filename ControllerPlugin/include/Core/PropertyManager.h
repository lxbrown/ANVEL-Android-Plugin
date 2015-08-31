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
#ifndef VanePropertyManager_h__
#define VanePropertyManager_h__

#include "Singleton.h"
#include "CoreCommon.h"
#include "Property.h"
#include "XmlInterface.h"

namespace VANE
{
	//////////////////////////////////////////////////////////////////////////
	// Types
	
	namespace Types
	{
		extern VaneCoreExport VaneID PropertyManager;
		extern VaneCoreExport VaneID Property;
	}
	
	//////////////////////////////////////////////////////////////////////////
	// Resources

	namespace Resources
	{
		extern VaneCoreExport const char* kPropertiesFileNodeFileReference;
		extern VaneCoreExport const char* kPropertiesFileNodeRoot;
		extern VaneCoreExport const char* kPropertiesFileNodeObject;
		extern VaneCoreExport const char* kPropertiesFileNodeProperty;
	}
	
	//////////////////////////////////////////////////////////////////////////
	// Property Descriptions
	
	enum PropertyFlags
	{
		kPropReadOnly           = 0x01,   ///This description is valid
		kPropNonSerializable    = 0x02,   ///This is a read only property
		kPropHidden             = 0x04,   ///This property should be saved or loaded
		kPropAdvanced           = 0x08,   ///This property should not be displayed
		kPropDeveloper          = 0x10,   ///This property is for advanced view only
		kPropValid              = 0x8000  ///This property is valid (used only internally)
	};
	
	///Description of a property, including whether it is readonly.  This is used
	///to describe the property to the end user, so that they may modify it,
	class VaneCoreExport PropertyDescription
	{
	public:
		PropertyDescription() 
		: m_flags(0)
		{	}
	
		///The name of the property
		String m_name;
		///The longer description of the property
		String m_desc;
		///Flags relating to various info about this property
		uint32 m_flags;
	};
	
	//////////////////////////////////////////////////////////////////////////
	// Property group
	
	///A Description of a group of properties that are related to the same data type.
	class VaneCoreExport PropertyGroupDescription
	{
	public:
		std::vector<PropertyDescription> m_descriptions; ///A group of descriptions.
		VaneID m_dataType; ///the data type we are associated with
	};
	
	//////////////////////////////////////////////////////////////////////////
	// PropertyManagerListener
	
	///A listener that gets notifications of newly created property providers and when they
	///are destroyed.
	class VaneCoreExport PropertyManagerListener
	{
	public:
		virtual void OnPropertyProviderDestroyed(VaneID id)=0;
	};
	
	typedef std::pair<VaneID, IPropertyProvider*> PropertyProviderPair;
	typedef std::vector<PropertyProviderPair> PropertyProviderInfoList;

	//////////////////////////////////////////////////////////////////////////
	//
	//  PropertyManager
	//
	//////////////////////////////////////////////////////////////////////////
	
	///Class that manages all properties and owns the descriptions of each.
	/// 
	/// An IPropertyProvider will use this class to register properties associated with specific
	/// data types. That is accomplished by registering itself as a PropertyProvider for the 
	/// specified type by calling RegisterPropertyProvider.  Then, properties will be added individually
	/// to the manager that are associated with the given type by calling RegiterProperty with a quick
	/// description.  
	///
	/// The PropertyManager will then be able to quickly forward on requests to get properties, or
	/// to provide the descriptions of properties to any interested caller that knows the 
	/// unique VaneID for a particular object. 
	///
	///@see IPropertyProvider 
	///@see VaneID
	class VaneCoreExport PropertyManager : 
		public Singleton<PropertyManager>,
		public IXmlInterface
	{
	public:
		///Construct the property manager
		PropertyManager();

		///Callback from whenever a property is updated.
		///@param[in] id The objectID of whatever was just updated
		///@param[in] propertyIndex The property index that was just updated
		void OnPropertyUpdated( VaneID id, PropertyIndex propertyIndex) const;

		/// Add a listener that will receive updates about property additions and deletes.
		/// TODO: After the update this has become less relevant. Let's see if we still need this.
		///@param[in] pListener
		void AddListener(PropertyManagerListener* pListener);
		
		///Remove a listener 
		///@param[in] pListener The listener that will no longer receive events
		void RemoveListener(PropertyManagerListener* pListener);
		
		///Register a property provider for 
		///@param[in] dataType The data type ID code that we are registering a property provider for
		///@param[in] pPropertyProvider The provider that will handle properties for the given data type
		void RegisterPropertyProvider(VaneID dataType, IPropertyProvider* pPropertyProvider);
		
		/// Remove the property provider for the given data type
		///@param[in] dataType The data type to remove the provider for
		void UnregisterPropertyProvider(VaneID dataType);
		
		/// Remove all property handling for a particular PropertyProvider
		///@param[in] pProvider
		void UnregisterPropertyProvider(IPropertyProvider* pProvider);
		
		/// Get a complete listing of register pairs and handlers for all register 
		/// data types.
		///@return 
		PropertyProviderInfoList GetPropertyProviderInfo() const;		

		/// Get a list of all DataTypes for which there is a registered handler.
		///@return An array of VaneIDs for which their dataTypes all have handlers.
		VaneIdVector GetHandledDataTypes() const;

		/// Get IDs with matching Type component.
		///@param[in] dataType The type of Ids to retrieve
		///@return A list of VaneIDs with specified Type components
		VaneIdVector GetIdsOfType(const VaneID dataType) const;
		
		/// Get the properties for this object
		///@param[in] objId The unique objectID of some type that may or may not provide properties
		///@return An array of properties from this object, or an empty array if properties could not be found
		ObjectPropertySet GetProperties(VaneID objId) const;
		
		/// Get the collection of property names defined for the given object id.
		///@param[in] objId The unique ID of an object or type that will be used to look up property names 
		///@return Array of strings that make up the set of property names for this object ID, or an empty array if none are found
		StringVector     GetPropertyNames(VaneID objId) const;
		
		///Get a Property Index by its registered name
		///@param[in] objID The type or object ID to lookup
		///@param[in] propertyName The name of the property being looked up
		///@return The index of the property, or kInvalidPropertyIndex if this property was not found for this object
		PropertyIndex GetPropertyIndexByName( VaneID objID, const VANE::String& propertyName ) const;
		
		/// Get the number of properties defined for the objectID.
		///@param[in] objID The ID of an object or data type
		///@return The number of properties defined, or 0, if none were found
		uint32 GetNumProperties(VaneID objID) const;
		
		/// Register a property definition in the system.  This is the way to actually declare a property.
		///@param[in] dataType The id of the data type that this property is getting registered for.
		///@param[in] name The name of this property.
		///@param[in] desc The description of this property.
		///@param[in] flags The various flags this description uses @see PropertyFlags for details on the flag values
		///@return The property index that this new property corresponds to
		PropertyIndex RegisterProperty( VaneID dataType, const String& name, const String& desc, uint32 flags = 0 );
		
		/// Get the description of a property group for a particular data type
		///@param[in] dataType The ID of a preregistered data type.
		///@return A description of the properties for the specified data type, or an empty description if no info was found
		const PropertyGroupDescription& GetPropertyGroupDescription( VaneID dataType) const;
		
		/// Get the description of an individual property
		///@param[in] dataType The data type to get a description for
		///@param[in] propertyIndex The property index
		///@return The description for a specific property
		const PropertyDescription& GetPropertyDescription(VaneID dataType, PropertyIndex propertyIndex) const;

		///Write properties to an Xml Element
		///@param[in] objID The object to turn to Xml
		///@param[in] elementName the string name to use for the child element appended to the passed in parent element.
		///@param[in] pElement The Xml Element to populate
		///@return The element that was added, or NULL if none were added.
		TiXmlElement* AddPropertiesToElement( VaneID objID, TiXmlElement* pElement, const String& elementName, bool includeID ) const;

		///Populate properties on an object based upon an Xml Element.  The Element should
		///include properly named properties for this object type.
		void GetPropertiesFromElement( VaneID objID, const TiXmlElement* pElement ) const;

		///Gets every single property for every single available object of every
		/// single provider. Big func, shouldn't be called often.
		///@return A vector of PropertyGroupInstanceAndObject, which is just a struct
		/// combining an objectId with its own associated PropertyGroupInstance.
		PropertyGroupsVector GetFullPropertySnapshot() const;

		///Gets every single property for a specific object.
		///@return A vector of properties and their associated flags.
		std::vector<std::pair<Variant, uint32> > GetObjectPropertySnapshot(VaneID id) const;
		
		///Create a description of the registered properties
		void CreatePropertyDocumentation( const String& fileName ) const;

		///Attempt to use an object's properties to find a name.
		/// If a name is not available, use it's type name and
		/// instance ID to form a name.
		///@param[in] An ID of a registered type, preferably with a registered
		///           property called "name" (case-insensitive)
		///@return A string with the contents of the first encountered "name"
		///        property, or the type name with the instance ID appended,
		///        or finally just the full ID itself, whichever is first available.
		String GetObjectName(VaneID object) const;

	public: //[Vane::Singleton methods]
		static PropertyManager* GetSingletonPtr();
		static PropertyManager& GetSingleton();

	public: //[IXmlInterface]
		bool ReadXmlFromNode( const TiXmlNode* pElement );
		void WriteXmlToNode ( TiXmlNode* pParentElement );
		
	private:

		//Internal method to resize our property arrays with a bit more control than just the normal std::vector expansion
		void ResizePropArrays(uint32 newMinSize);
		
	private:
		typedef  std::vector<IPropertyProvider*> PropertyProviderVector;
		typedef  std::vector<uint16> PropertyDescriptionIndexVector;
		typedef  std::vector<PropertyGroupDescription> PropertyDescriptionVector;
		
		//Our property providers, stored in an array that has one entry per data type, regardless of whether or not
		//any providers have been registered for each data type. This allows a direct lookup based on the ID portion
		//of the VaneID. The PropertyProviderVector and the PropertyDescriptionIndexVector are both using small POD types
		//(pointers and uint16s) so having a large amount of them should not be much of an issue.  
		//The unused data types have a NULL entry.
		PropertyProviderVector m_propProviders;
		
		//A lookup table that allows us to have a concise array for the descriptions to save space, while
		//also allowing very quick lookups based on the datatype portion of the ID. You use the type index (from GetTypeIndexFromID)
		//to get an index into this lookup table, and then use the value found at that position in the table to get the equivalent
		//item in the larger PropertyDescriptionVector.  Unused types will have a zero value, which should correspond to an invalid
		//property description, that will nicely handle invalid properties.
		PropertyDescriptionIndexVector m_propProviderIndexTable;
		
		//The dense array of PropertyDescriptions.  Unlike the other arrays, this only contains descriptions of PropertyGroups that
		//have been registered.
		PropertyDescriptionVector m_propGroupDescriptions;
		PropertyDescription m_invalidPropDesc;
		
		//the listeners that will get notifications of property provider creation / destruction
		vector<PropertyManagerListener*> m_listeners;
	};
}

#endif // PropertyManager_h__
