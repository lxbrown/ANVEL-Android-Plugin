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
#ifndef VANE_Property_h__
#define VANE_Property_h__

#include "Variant.h"
#include "Commands.h"
#include "VaneTypes.h"
#include "CoreCommon.h"
#include "VaneIDs.h"
#include "VaneTime.h"

namespace VANE
{
	namespace Resources
	{
		extern VaneCoreExport const char* kPropertiesFileNodeProperty;
	}

	///Not used yet, but will be nicer than a bool in the property registration stuff
	enum PropertyState
	{
		kReadWrite =0,
		kReadOnly  =1
	};
	

	/************************************************************************/
	/* Property                                                             */
	/************************************************************************/


	///A very lightweight version of Variant meant to minimize space.
	///It is basically the reference version of variant, ie, it does not
	///own any of its own data, it only has a reference and a type.
	///This makes it much smaller than a variant, and allows changes
	///to actual data to take place through this class, but if the owner
	///of the parameter is released, the properties become invalid.
	///
	///Note that you must correctly provide the type of the variable,
	///ie make sure you use uint64 in the actual class, instead of uint32 if
	///you specify a uint64 parameter, otherwise it will fail badly.
	class VaneCoreExport Property
	{
	public:

		///Default constructor to generic an invalid, but constructed property
		Property();

		Property(bool& value);
		Property(uint32& value);
		Property(int32& value);
		Property(float32& value);
		Property(uint64& value);
		Property(int64& value);
		Property(float64& value);
#if !PLAIN_VANEID
		Property(VaneID& value);
#endif
		Property(Vector4& value);
		Property(Vector3& value);
		Property(Vector2& value);
		Property(Quaternion& value);
		Property(Color& value);
		Property(String& value);
		Property(Enum& value);
		Property(Flag& value);
		Property(Radian& value);
		Property(TimeUtil::DateTime& value);
		Property(Coordinate& value);
		Property(VariantVector& value);
		Property(NameValuePairList& value);

		VariantType::Value GetType() const { return m_type; }

		//bool
		bool          GetBoolValue() const;
		void          SetBoolValue(bool value);

		//uint32
		uint32        GetUInt32Value() const;
		void          SetUInt32Value(uint32 value);

		//signed int32
		int32         GetInt32Value() const;
		void          SetInt32Value(int32 value);

		//uint64
		uint64        GetUInt64Value() const;
		void          SetUInt64Value(uint64 value);

		//signed int64
		int64         GetInt64Value() const;
		void          SetInt64Value(int64 value);

		//float64
		float64       GetFloat64Value() const;
		void          SetFloat64Value(float64 value);

		//float32
		float32       GetFloat32Value() const;
		void          SetFloat32Value(float32 value);

		//VaneID
		VaneID        GetVaneIDValue() const;
		void          SetVaneIDValue(const VaneID& value);

		//vector4
		Vector4       GetVector4Value() const;
		void          SetVector4Value(const Vector4& value);

		//vector3
		Vector3       GetVector3Value() const;
		void          SetVector3Value(const Vector3& value);

		//vector3
		Vector2       GetVector2Value() const;
		void          SetVector2Value(const Vector2& value);

		//quaternion
		Quaternion    GetQuaternionValue() const;
		void          SetQuaternionValue(const Quaternion& value);

		//color
		Color         GetColorValue() const;
		void          SetColorValue(const Color& value);

		//string
		String        GetStringValue() const;
		void          SetStringValue(const String& value);

		//enum
		Enum          GetEnumValue() const;
		void          SetEnumValue(const Enum& value);

		//flag
		Flag          GetFlagValue() const;
		void          SetFlagValue(const Flag& value);

		//radian
		Radian        GetRadianValue() const;
		void          SetRadianValue(const Radian& value);
		
		//datetime
		TimeUtil::DateTime GetDateTimeValue() const;
		void          SetDateTimeValue(const TimeUtil::DateTime& value);
		
		Coordinate    GetCoordinateValue() const;
		void          SetCoordinateValue( const Coordinate& value );
		
		VariantVector   GetVariantListValue() const;
		void          SetVariantListValue( const VariantVector& value );
		
		NameValuePairList   GetNameValueListValue() const;
		void          SetNameValueListValue( const NameValuePairList& value );

		//This will fail unless the data types are the same.
		void          SetFromVariant(const Variant& variantData);

		///Write this out as a string value, no matter what
		///the internal type
		String        ToString() const;

		///Get a variant (which will own a copy of the data)
		///From what we currently have in this Property
		Variant       GetVariantValue() const;

	private:
		VariantType::Value  m_type; //the data type of this property
		void*               m_data; //actual data for this type
	};

	///A group of properties that are related to the same Object Instance.
	typedef std::vector<Property> PropertyGroupInstance;

	//A struct for tying a set of property values to their object, with flags
	struct PropertyGroupInstanceAndObject
	{
		VaneID objectId;
		//A vector of pairs that represents a Property and it's associated flags.
		//We use a variant instead of a property because a Property will be updated
		// and it's intended that this value represent a 'snapshot'
		std::vector<std::pair<Variant, uint32> > propertiesAndFlags;
	};

	//A vector of property value sets associated with an object
	typedef std::vector<PropertyGroupInstanceAndObject> PropertyGroupsVector;

	///A group of properties for an object, along with any ids pertaining to
	/// it's isA's and hasA's, so interested clients can look up those as well.
	struct ObjectPropertySet
	{
		VaneIdVector isAIds;
		VaneIdVector hasAIds;
		PropertyGroupInstance properties;
	};

	///A predefined property with Invalid values.
	extern VaneCoreExport Property kInvalidProperty;

	///Simple array index for the properties.
	typedef VANE::uint32 PropertyIndex;
	
	///Our invalid property index
	const PropertyIndex kInvalidPropertyIndex = 0xFFFFFFFF;

	///Container for a property belonging to a specific object.
	struct SpecificProperty
	{
		SpecificProperty(const VaneID& f, const PropertyIndex& s)
			: objectId(f)
			, propertyIndex( s )
		{	}

		SpecificProperty()
			: objectId( kInvalidVaneID )
			, propertyIndex( kInvalidPropertyIndex )
		{   }

		VaneID objectId;
		PropertyIndex propertyIndex;
	};

	///A class that provides properties for one or more registered data types.
	///The property provider will register with the property manager and let it 
	///know which data types it can generate properties for.  The property provider should
	///have access to any type it will provide properties for, and will generally be the manager
	///or factory class for a given type.  The property provider should also be able to 
	///lookup a specific instance of a type based on the instanceID portion of the VaneID.
	class VaneCoreExport IPropertyProvider
	{
	public:
		///Notification that a property value has changed
		///@param[in] objID the Unique ID of the object that has had a property change
		///@param[in] index The Property Index that has been changed
		virtual void OnPropertyChanged(VaneID objId, PropertyIndex index) { VANE_UNUSED(objId ); VANE_UNUSED( index ); };
		
		/// Get a single property value for the object with the given unique id
		///@param[in] objID The Unique ID of the object to get properties from
		///@param[in] index The Property Index that has been changed
		///@return A Property for this field
		virtual Property GetProperty(VaneID objID, PropertyIndex index);
		
		///Get the complete set of properties for this object.
		///@param[in] objID The Unique ID of the object to get properties from
		///@param[in] index The Property Index that has been changed
		///@return A Property for this field
		virtual ObjectPropertySet GetProperties(VaneID objID)=0;

		///Given a data type (NOT a data type index), return a vector
		///containing every VaneID with matching type that this provider
		///is responsible for.
		///@param[in] dataType The data type of VaneIDs we are looking for
		///@return A vector of VaneIDs with their data type matching the input
		virtual VaneIdVector GetIdsOfType(const VaneID dataType) const=0;
		
		///Set this to false if we should disallow querying of types, used
		///for skipping registered types, such as those that are part of VTI, etc.
		virtual bool AreTypesPublic() { return true; }

		virtual void ImposeProperties(VaneID objId, const VariantVector& newProps, bool serializableOnly = true);

		virtual ~IPropertyProvider() {}
	};
}

#endif // Property_h__
