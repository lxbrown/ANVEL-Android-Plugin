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


#ifndef RangeSensor_h__
#define RangeSensor_h__

#include "SensorPlugin.h"
#include "Simulation/Sensor.h"

namespace VANE
{
	extern const SENSOR_API SensorType kSensorTypeSampleSensor;

	//////////////////////////////////////////////////////////////////////////
	// Parameters

	///Sample specific configuration parameters
	class SampleSensorStaticAssetParams
		: public SensorStaticAssetParams
	{
	public:
		uint32  m_intData; 	
	};

	//Forward declare for use within the SampleSensor class
	class SampleSensorFactory;

	//////////////////////////////////////////////////////////////////////////
	// Sample Sensor

	///Simple sensor model developed to illustrate Sensor creation and usage
	///within the ANVEL system.
	class SENSOR_API SampleSensor : public Sensor
	{
	friend class SampleSensorFactory;

	public:
		/// Standard virtual destructor ( not needed since we have no resources to release )
		virtual ~SampleSensor();

		/// Update our sensor
		virtual void Update(TimeValue dt);

	protected:
		SampleSensor( VaneID specificId, SensorStaticAssetParams& params, DynamicAssetParams& dynamicParams );

	protected:
		// Sensor specific data goes here
		uint32 m_sampleIntData;
		
		int frame;
		int sendRate;
		int quality_factor;
		bool running;
		String ipaddr;

	};

	//////////////////////////////////////////////////////////////////////////
	// SampleSensorFactory

	///Sensor factory for our sample sensor. This needs to register
	///the factory with the SensorMgr for each type that it handles.
	class SENSOR_API SampleSensorFactory 
		: public ISensorFactory
		, public IPropertyProvider
	{
	public:
		SampleSensorFactory();
		~SampleSensorFactory();

	public: //[ISensorFactory methods]
		virtual Sensor* CreateSensor( SensorStaticAssetParams& params, DynamicAssetParams& dynamicParams );
		virtual StaticAssetParamsPtr ParseXmlParams( const TiXmlElement* pXmlElement );

	public: //[IPropertyProvider methods]
		virtual ObjectPropertySet GetProperties( VaneID objID );
		virtual VaneIdVector GetIdsOfType( const VaneID dataType ) const;
	
	private:

		///Create a range sensor
		Sensor* CreateSampleSensor( SensorStaticAssetParams& params, DynamicAssetParams& dynamicParams );
		///Register our properties
		void RegisterProperties();
		///Get the properties for a specific sample sensor
		PropertyGroupInstance GetSensorProperties( SampleSensor& sensor );
		//TODO: ADD our nifty id creator thing here...
		SensorID GetNextSensorID();

	private:

		VaneID m_sensorIDCount;
	};





}

#endif
