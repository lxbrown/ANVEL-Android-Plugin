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
/*
-----------------------------------------------------------------------------
This source file is part of OGRE
	(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2009 Torus Knot Software Ltd

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
-----------------------------------------------------------------------------
*/
#ifndef VANE_Core_StringConverter_h__
#define VANE_Core_StringConverter_h__

// This file is based on material originally from:
// the Ogre3d rendering system
// Copyright (c) 2000-2009 Torus Knot Software Ltd
// Distributed under the MIT License.
// For the latest info, see http://www.ogre3d.org/

#include <iostream>
#include <sstream>

#include "VaneIDs.h"
#include "VaneTypes.h"
#include "StringUtil.h"
#include "VaneTime.h"
#include "Variant.h"

namespace VANE
{
	class Vector2;
	class Vector3;
	class Vector4;
	class Matrix3;
	class Matrix4;
	class Quaternion;
	class Color;
	struct Coordinate;
	struct SpecificProperty;
	struct VaneIdSet;
	typedef VaneID EnumID;
	class Enum;
	class Flag;

	/// Converts variable types to/from string format.  
	/// Based off of Ogre::StringConverter
	class VaneCoreExport StringConverter
	{
	public:
		/// Convert a float32 to a String.
		/// @param[in] val float32 to convert.
		/// @param[in] precision How many decimal places to include.
		/// @param[in] strWidth Minimum width of the string.
		/// @param[in] fill Character to fill string with to meet minimum strWidth requirement.
		/// @param[in] flags Formatting flags for the string.
		static String ToString( float32 val, uint16 precision = 6,
			uint16 strWidth = 0, char fill = ' ',
			std::ios::fmtflags flags = std::ios::fmtflags(0) );

		/// Convert a float64 to a String.
		/// @param[in] val float64 to convert.
		/// @param[in] precision How many decimal places to include.
		/// @param[in] strWidth Minimum width of the string.
		/// @param[in] fill Character to fill string with to meet minimum strWidth requirement.
		/// @param[in] flags Formatting flags for the string.
		static String ToString( float64 val, uint16 precision = 6,
			uint16 strWidth = 0, char fill = ' ',
			std::ios::fmtflags flags = std::ios::fmtflags(0) );

		/// Convert an int8 to a String.
		/// @param[in] val int8 to convert.
		/// @param[in] strWidth Minimum width of the string.
		/// @param[in] fill Character to fill string with to meet minimum strWidth requirement.
		/// @param[in] flags Formatting flags for the string.
		static String ToString( int8 val, uint16 strWidth = 0, 
			char fill = ' ', std::ios::fmtflags flags = std::ios::fmtflags(0) );

		/// Convert an int16 to a String.
		/// @param[in] val int16 to convert.
		/// @param[in] strWidth Minimum width of the string.
		/// @param[in] fill Character to fill string with to meet minimum strWidth requirement.
		/// @param[in] flags Formatting flags for the string.
		static String ToString( int16 val, uint16 strWidth = 0, 
			char fill = ' ', std::ios::fmtflags flags = std::ios::fmtflags(0) );

		/// Convert an int32 to a String.
		/// @param[in] val int32 to convert.
		/// @param[in] strWidth Minimum width of the string.
		/// @param[in] fill Character to fill string with to meet minimum strWidth requirement.
		/// @param[in] flags Formatting flags for the string.
		static String ToString( int32 val, uint16 strWidth = 0, 
			char fill = ' ', std::ios::fmtflags flags = std::ios::fmtflags(0) );

		/// Convert a int64 to a String.
		/// @param[in] val int64 to convert.
		/// @param[in] strWidth Minimum width of the string.
		/// @param[in] fill Character to fill string with to meet minimum strWidth requirement.
		/// @param[in] flags Formatting flags for the string.
		static String ToString( int64 val, uint16 strWidth = 0, 
			char fill = ' ', std::ios::fmtflags flags = std::ios::fmtflags(0) );

		/// Convert an uint8 to a String.
		/// @param[in] val uint8 to convert.
		/// @param[in] strWidth Minimum width of the string.
		/// @param[in] fill Character to fill string with to meet minimum strWidth requirement.
		/// @param[in] flags Formatting flags for the string.
		static String ToString( uint8 val, uint16 strWidth = 0, 
			char fill = ' ', std::ios::fmtflags flags = std::ios::fmtflags(0) );

		/// Convert an uint16 to a String.
		/// @param[in] val uint16 to convert.
		/// @param[in] strWidth Minimum width of the string.
		/// @param[in] fill Character to fill string with to meet minimum strWidth requirement.
		/// @param[in] flags Formatting flags for the string.
		static String ToString( uint16 val, uint16 strWidth = 0, 
			char fill = ' ', std::ios::fmtflags flags = std::ios::fmtflags(0) );

		/// Convert an unsigned integer to a String.
		/// @param[in] val Unsigned integer to convert.
		/// @param[in] strWidth Minimum width of the string.
		/// @param[in] fill Character to fill string with to meet minimum strWidth requirement.
		/// @param[in] flags Formatting flags for the string.
		static String ToString( uint32 val, uint16 strWidth = 0, 
			char fill = ' ', std::ios::fmtflags flags = std::ios::fmtflags(0) );

		/// Convert a uint64 to a String.
		/// @param[in] val uint64 to convert.
		/// @param[in] strWidth Minimum width of the string.
		/// @param[in] fill Character to fill string with to meet minimum strWidth requirement.
		/// @param[in] flags Formatting flags for the string.
		static String ToString( uint64 val, uint16 strWidth = 0, 
			char fill = ' ', std::ios::fmtflags flags = std::ios::fmtflags(0) );


		/// Convert an long to a String.
		/// @param[in] val Long to convert.
		/// @param[in] strWidth Minimum width of the string.
		/// @param[in] fill Character to fill string with to meet minimum strWidth requirement.
		/// @param[in] flags Formatting flags for the string.
		static String ToString( long val, uint16 strWidth = 0, 
			char fill = ' ', std::ios::fmtflags flags = std::ios::fmtflags(0) );

		/// Convert an unsigned long to a String.
		/// @param[in] val Long to convert.
		/// @param[in] strWidth Minimum width of the string.
		/// @param[in] fill Character to fill string with to meet minimum strWidth requirement.
		/// @param[in] flags Formatting flags for the string.
		static String ToString( unsigned long val, uint16 strWidth = 0, 
			char fill = ' ', std::ios::fmtflags flags = std::ios::fmtflags(0) );

#if !PLAIN_VANEID
		/// Convert a VaneID to a String.
		/// @param[in] val VaneID to convert.
		/// @param[in] strWidth Minimum width of the string.
		/// @param[in] fill Character to fill string with to meet minimum strWidth requirement.
		/// @param[in] flags Formatting flags for the string.
		static String ToString( VaneID val, uint16 strWidth = 0, 
			char fill = ' ', std::ios::fmtflags flags = std::ios::fmtflags(0) );
#endif


		/// Convert a boolean to a String
		/// @param[in] val Boolean to convert.
		/// @param[in] yesNo Whether to display values as yes/no instead of true/false.
		static String ToString( bool val, bool yesNo = false );

		/// Convert a Vector2 to a String.
		/// @param[in] val Vector2 to convert.
		static String ToString( const Vector2& val );

		/// Convert a Vector3 to a String.
		/// @param[in] val Vector3 to convert.
		static String ToString( const Vector3& val );
		
		/// Convert a Vector3 to a String.
		/// @param[in] val Vector3 to convert.
		/// @param[in] precision decimal places to print.		
		static String ToString(const Vector3& val, uint16 precision);

		/// Convert a Vector4 to a String.
		/// @param[in] val Vector4 to convert.
		static String ToString( const Vector4& val );

		/// Convert a Matrix3 to a String.
		/// @param[in] val Matrix3 to convert.
		static String ToString( const Matrix3& val );

		/// Convert a Matrix4 to a String.
		/// @param[in] val Matrix4 to convert.
		static String ToString( const Matrix4& val );

		/// Convert a Quaternion to a String.
		/// @param[in] val Quaternion to convert.
		static String ToString( const Quaternion& val );

		/// Convert a Color to a String.
		/// @param[in] val Color to convert.
		static String ToString( const Color& val );

		/// Convert a StringVector to a String.
		/// @param[in] val StringVector to convert.
		static String ToString( const StringVector& val );

		/// Convert vector of doubles to comma delimited string
		///@param[in] valArray  Vector of floats
		static String ToString(const std::vector<float64>& valArray);

		/// Convert a SpecificProperty to a String.
		/// @param[in] val SpecificProperty to convert.
		static String ToString( const SpecificProperty& val, bool findReadableName = false, bool lookForNameProp = true );

		///Convert a DateTime structure to a readable string
		///@param[in] dateTime The date/time value to convert
		static String ToString( const TimeUtil::DateTime& dateTime );
		
		///Convert a Coordinate to a string representation.
		///This will first describe the notation, and then the
		///data.  UTM NORTHING EASTING ELEVATION, or LATLONG LATITUDE LONGITUDE ELEVATION
		static String ToString( const Coordinate& coordinate );

		///Convert an enum to a string, with values like:
		/// <EnumTypeName>.<EnumValueName>
		static String ToString( const Enum& enumVal );

		///Convert a flag to a string, with values like:
		/// <EnumTypeName>.<FlagBitvec>
		static String ToString( const Flag& flagVal );
		
		///Convert a VaneIdSet to a String
		static String ToString( const VaneIdSet& idSet );
		
		///Convert a VaneIdVector to a string
		static String ToString( const VaneIdVector& idVector );

		///Convert a VariantVector to a string
		static String ToString( const VariantVector& varVector );

		///Convert a NameValueList to a string
		static String ToString( const NameValuePairList& nameValPairList );

		/// Convert value to Hex String
		/// @param[in] val Unsigned integer to convert.
		///@return Value as Hexadecimal String
		template <typename T> static String ToHexString(  T val)
		{
			char fill = ' ';
			uint16 strWidth = 0;
			std::stringstream sstr;
			sstr.width(strWidth);
			sstr.fill(fill);

			// see http://www.cplusplus.com/reference/iostream/ios_base/fmtflags/ 
			// for discussion of formatting flags
			sstr.setf (std::ios_base::hex ,std:: ios_base::basefield);
			sstr.setf (std::ios_base::showbase);
			sstr << val;
			return sstr.str();
		}

		/// Convert a pointer to a hex string
		/// @param[in] val Pointer to convert.
		///@return Value as Hexadecimal String
		template <typename T> static String ToHexString( T* ptrVal )
		{
			// Probably a better way to do this....
			uint32 ptrSize = sizeof(void*);
			if(ptrSize == 4) 
			{
				// 32 bit pointer
				uint32 ptr32 = (uint32) (ptrVal);
				return ToHexString(ptr32);
			}
			else if (ptrSize == 8) 
			{
				// 64 bit pointer
				uint64 ptr64 = (uint64) (ptrVal);
				return ToHexString(ptr64);
			}
			return String();
		};


		/****************************************************
		 *  Parsing methods
		 ****************************************************/
		/// Parse a float32 from a String
		/// @param[in] String to parse.
		/// @return The parsed value or 0.0 if the parsing failed.
		static float32 ParseFloat32( const String& val );

		/// Parse a float64 from a String
		/// @param[in] String to parse.
		/// @return The parsed value or 0.0 if the parsing failed.
		static float64 ParseFloat64( const String& val );

		/// Parse an int8 from a String
		/// @param[in] String to parse.
		/// @return The parsed value or 0 if the parsing failed.
		static int8 ParseInt8( const String& val );

		/// Parse an int16 from a String
		/// @param[in] String to parse.
		/// @return The parsed value or 0 if the parsing failed.
		static int16 ParseInt16( const String& val );

		/// Parse an integer from a String
		/// @param[in] String to parse.
		/// @return The parsed value or 0 if the parsing failed.
		static int ParseInt( const String& val );

		/// Parse an int32 from a String
		/// @param[in] String to parse.
		/// @return The parsed value or 0 if the parsing failed.
		static int32 ParseInt32( const String& val );

		/// Parse an int64 from a String
		/// @param[in] String to parse.
		/// @return The parsed value or 0 if the parsing failed.
		static int64 ParseInt64( const String& val );

		/// Parse an uint8 from a String
		/// @param[in] String to parse.
		/// @return The parsed value or 0 if the parsing failed.
		static uint8 ParseUnsignedInt8( const String& val );

		/// Parse an uint16 from a String
		/// @param[in] String to parse.
		/// @return The parsed value or 0 if the parsing failed.
		static uint16 ParseUnsignedInt16( const String& val );

		/// Parse an unsigned int from a String
		/// @param[in] String to parse.
		/// @return The parsed value or 0 if the parsing failed.
		static unsigned int ParseUnsignedInt( const String& val );

		/// Parse an uint32 from a String
		/// @param[in] String to parse.
		/// @return The parsed value or 0 if the parsing failed.
		static uint32 ParseUnsignedInt32( const String& val );

		/// Parse an uint32 from a Hexidecimal String
		///@param[in] Hexidecimal String to parse.
		///@return The parsed value or 0 if the parsing failed.
		static uint32 ParseUnsignedInt32FromHex( const String& hexVal );

		/// Parse an uint64 from a String
		/// @param[in] String to parse.
		/// @return The parsed value or 0 if the parsing failed.
		static uint64 ParseUnsignedInt64( const String& val );

		/// Parse a Real from a String
		/// @param[in] String to parse.
		/// @return The parsed value or 0.0 if the parsing failed.
		static Real ParseReal( const String& val );

		/// Parse a long from a String
		/// @param[in] String to parse.
		/// @return The parsed value or 0 if the parsing failed.
		static long ParseLong( const String& val );

		/// Parse an unsigned long from a String
		/// @param[in] String to parse.
		/// @return The parsed value or 0 if the parsing failed.
		static uint64 ParseUnsignedLong( const String& val );

#if !PLAIN_VANEID
		/// Parse a VaneID from a String
		/// @param[in] String to parse.
		/// @return The parsed value or kInvalidVaneID if the parsing failed.
		static VaneID ParseVaneID( const String& val );
#endif

		/// Parse a bool from a String
		/// @param[in] String to parse.
		/// @return True if the start of the String matches "true", "yes" or "1", false otherwise.
		static bool ParseBool( const String& val );

		/// Parse a Vector2 from a String
		/// @param[in] String to parse.
		/// @return Parsed vector or Vector2::GetZero() if parsing failed.
		/// @note String format should be "x y"
		static Vector2 ParseVector2( const String& val );

		/// Parse a Vector3 from a String
		/// @param[in] String to parse.
		/// @return Parsed vector or Vector3::ZERO if parsing failed.
		/// @note String format should be "x y z"
		static Vector3 ParseVector3( const String& val );

		/// Parse a Vector4 from a String
		/// @param[in] String to parse.
		/// @return Parsed vector or Vector4::ZERO if parsing failed.
		/// @note String format should be "x y z"
		static Vector4 ParseVector4( const String& val );

		/// Parse a Quaternion from a String
		/// @param[in] String to parse.
		/// @return Parsed quaternion or Quaternion::GetIdentity() if parsing failed.
		/// @note String format should be "w x y z"
		static Quaternion ParseQuaternion( const String& val );

		/// Parse a Matrix3 from a String
		/// @param[in] String to parse.
		/// @return Parsed matrix or Matrix3::IDENTITY if parsing failed.
		/// @note String format should be "00 01 02 10 11 12 20 21 22"
		static Matrix3 ParseMatrix3( const String& val );

		/// Parse a Matrix4 from a String
		/// @param[in] String to parse.
		/// @return Parsed matrix or Matrix4::IDENTITY if parsing failed.
		/// @note String format should be "00 01 02 03 10 11 12 13 20 21 22 23 30 31 32 33"
		static Matrix4 ParseMatrix4( const String& val );

		/// Parse a Matrix3 from a String, uses MATLAB format
		/// @param[in] String to parse.
		/// @return Parsed matrix, missing values will be filled in from Identity matrix
		/// @note String format should be "[00, 01, 02; 10, 11, 12; 20, 21, 22]"
		static Matrix3 ParseRotationMatrix( const String& val );

		/// Parse a vector of floats from comma-delimited string
		/// @param[in] valarray   String to parse.
		/// @param[in] vectorSize Number of elements in vector (optional)
		/// @vector of float64's
		static std::vector<float64> ParseFloat64Vector(const String& valarray, size_t vectorSize = 0);

		/// Parse a Color from a String
		/// @param[in] String to parse.
		/// @return Parsed Color or Color::Black if parsing failed.
		/// @note String format should be "r g b a" or "r g b"
		static Color ParseColor( const String& val );

		/// Parse a StringVector from a String
		/// @param[in] String to parse.
		/// @return Parsed StringVector.
		/// @note Strings are delimited by a space.
		static StringVector ParseStringVector( const String& val );

		/// Parse a SpecificProperty from a String
		///@param[in] val String to parse.
		///@return Parsed SpecificProperty
		///@note String format should be <objId>[<propIndex>],
		///      <objTypeName>[<objInstanceIndex>].<propName>
		///      OR <objTypeName>.<objInstanceName>.<propName>
		static SpecificProperty ParseSpecificProperty( const String& val );
		
		///Parse a date time from a string
		///@return DateTime from the string value. All 0 if parsing failed.
		static TimeUtil::DateTime ParseDateTime( const String& val );
		
		///Parse a coordinate from a string
		///@return Coordinate that is valid and in the type specified if parsing
		///succeeded.
		static Coordinate ParseCoordinate( const String& val );

		///Parse a enum from a string
		///@return Enum that is valid if successful
		static Enum ParseEnum( const String& val );

		///Parse a flag from a string
		///@return Flag that is valid if successful
		static Flag ParseFlag( const String& val );
		
		///Parse an ID Set
		///@return VaneIDSet that is valid if successful
		static VaneIdSet ParseIdSet( const String& val );
		
		///Parse a VaneIDVector Set
		///@return VaneIDVector that is valid if successful
		static VaneIdVector ParseIdVector( const String& val );

		static VariantVector ParseVariantVector( const String& val );

		static NameValuePairList ParseNameValuePairList( const String& val );
	};
}

#endif // StringConverter_h__
