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

#include "zmq.hpp"
#include "jpge.h"
#include <string>
#include <iostream>
#ifndef _WIN32
#include <unistd.h>
#else
#undef min
#undef max
#include <windows.h>

#define sleep(n)    Sleep(n)
#endif

#include <WinSock.h>
#include<WinSock2.h>


#include "SampleSensor.h"

#include "Core/PropertyManager.h"
#include "Simulation/World/WorldManager.h"
#include "Simulation/Sensor.h"
#include "Simulation\CameraSensor.h"

namespace VANE
{
	//////////////////////////////////////////////////////////////////////////
	// GUIDs

	const DataTypeGUID kSampleSensorGUID   = { 0xfd24ef34, 0xc4d2, 0x4364, { 0x83, 0x93, 0x53, 0xd7, 0x6f, 0x2d, 0xa7, 0xb5 } };
		
	//////////////////////////////////////////////////////////////////////////
	// Types

	namespace Types
	{
		SENSOR_API VaneID SampleSensor = 0;
	}

	//////////////////////////////////////////////////////////////////////////

	const SENSOR_API SensorType kSensorTypeSampleSensor = "SampleSensor";

	//////////////////////////////////////////////////////////////////////////
	
	
	zmq::socket_t socket_;
	zmq::context_t context_;
	
	//Get the IP address of the computer
	bool getMyIP(String& myIP)
	{
		char szBuffer[1024];

		#ifdef WIN32
		WSADATA wsaData;
		WORD wVersionRequested = MAKEWORD(2, 0);
		if(::WSAStartup(wVersionRequested, &wsaData) != 0)
			return false;
		#endif


		if(gethostname(szBuffer, sizeof(szBuffer)) == SOCKET_ERROR)
		{
		  #ifdef WIN32
		  WSACleanup();
		  #endif
		  return false;
		}

		struct hostent *host = gethostbyname(szBuffer);
		if(host == NULL)
		{
		  #ifdef WIN32
		  WSACleanup();
		  #endif
		  return false;
		}

		for (int i = 0; host->h_addr_list[i] != 0; ++i) {
			struct in_addr addr;
			memcpy(&addr, host->h_addr_list[i], sizeof(struct in_addr));

			//We want the local IP address
			String temp = inet_ntoa(addr);
			String compare = "192.168.1.";
			if(temp.compare(0, compare.length(), compare) == 0) {
				myIP = inet_ntoa(addr);
				#ifdef WIN32
				WSACleanup();
				#endif
				return true;
			}
		}


		#ifdef WIN32
		WSACleanup();
		#endif
		return false;
	}

	SampleSensor::SampleSensor( VaneID specificId, SensorStaticAssetParams& params, DynamicAssetParams& dynamicParams )
		: Sensor(specificId, params, dynamicParams)
	{
		
		//Get the current IP address
		String ip;
		if(!getMyIP(ip)) {
			LogMessage("Failed to get IP", kLogMsgError);
			running = false;
		}
		else {
			LogMessage(ip);

			ipaddr = ip;
			running = true;

			//Bind to the computer's IP adress
			socket_.init(context_, ZMQ_PAIR);
			socket_.bind("tcp://" + ip + ":9000");

			LogMessage("Connected", kLogMsgSpecial);
		}

		frame = 0;
		sendRate = 15;
		quality_factor = 85;

		
		//cast to our specific type of asset params, and grab data 
		const SampleSensorStaticAssetParams& sampleParams = static_cast<const SampleSensorStaticAssetParams&>( params );
		m_sampleIntData = sampleParams.m_intData;
	}
	
	//////////////////////////////////////////////////////////////////////////

	SampleSensor::~SampleSensor()
	{
		//Any sensor resources should be shut down here
	}

	//////////////////////////////////////////////////////////////////////////


	void SampleSensor::Update(TimeValue dt)
	{
		//check to see if it is time to write an update to this sensor
		m_sampleTimeLeft -= dt;

		//if we still have time left, skip writing an update
		if (m_sampleTimeLeft > 0.0) 
			return;

		//Sending the image is dependent on the frame rate and if the user has closed the connection
		if((frame % (int) (100 / sendRate) == 0) && running) {
			// Get Video Data and Send as ZMQ Message
			std::vector<SensorPtr> sensors = SensorManager::GetSingleton().GetAllSensors();
			for (uint32 i = 0; i < sensors.size(); ++i)
			{
				if (sensors[i]->GetSensorType() != "CameraSensor")
					continue;

				CameraSensor* pCam = static_cast<CameraSensor*>(sensors[i].Get());
		
				const std::vector<LensData>& lens = pCam->GetLensData();
				if (lens.size() == 0)
					continue;

				const LensData& thisLens = lens[0];
				const LensParams & lensParams = pCam->GetLensParams()[0];

				if (thisLens.m_renderRequest.m_pOutputBuffer != NULL) {
					//Pull the dimensions of the camera
					int size, sizeX, sizeY;
					sizeX = lensParams.m_resolutionX;
					sizeY = lensParams.m_resolutionY;
					size = sizeX * sizeY * 3;
				
					//Make a buffer for the compressed jpeg
					void *buf = malloc(size);

					//Set the Quality Factor
					jpge::params params;
					params.m_quality = quality_factor;

					//Compress the image to improve transfer speed
					if(compress_image_to_jpeg_file_in_memory(buf, size, sizeX, sizeY, 3, thisLens.m_renderRequest.m_pOutputBuffer, params)) {
						//Put the compressed data into a ZMQ message and send it over the socket
						zmq::message_t image (size);
						memcpy((void *) image.data(), buf, size);
						socket_.send (image);
					}
					else {
						LogMessage("Failed to compress image", kLogMsgError);
					}
				}
			}
		}
		frame++;
		
		
		m_sampleTimeLeft += m_sampleStep;
	}

	//////////////////////////////////////////////////////////////////////////
	//
	// SampleSensorFactory
	//
	//////////////////////////////////////////////////////////////////////////

	SampleSensorFactory::SampleSensorFactory() 
		: m_sensorIDCount(0)
	{
		DataTypeManager& dataTypeMgr = DataTypeManager::GetSingleton();

		DataTypeDescription typeDesc( "ZMQSensor", "ZMQ Sensor", "ZMQ Sensor Plugin" );
		typeDesc.typeIsA.push_back( Types::Sensor );

		Types::SampleSensor = dataTypeMgr.RegisterDataType( typeDesc, kSampleSensorGUID );

		RegisterProperties();

		SensorManager::GetSingleton().RegisterSensorFactory( kSensorTypeSampleSensor, this );
	}
	
	//////////////////////////////////////////////////////////////////////////

	SampleSensorFactory::~SampleSensorFactory()
	{
		SensorManager::GetSingleton().UnregisterSensorFactory( this );
	}

	//////////////////////////////////////////////////////////////////////////

	SensorID SampleSensorFactory::GetNextSensorID()
	{
		VaneID sensorID = MakeID( Types::SampleSensor, m_sensorIDCount );
		m_sensorIDCount++;
		return sensorID;
	}

	//////////////////////////////////////////////////////////////////////////
	
	PropertyGroupInstance SampleSensorFactory::GetSensorProperties( SampleSensor& sensor )
	{
		PropertyGroupInstance properties;
		properties.push_back( Property( sensor.sendRate ) );
		properties.push_back( Property( sensor.quality_factor ) );

		return properties;
	}

	//////////////////////////////////////////////////////////////////////////

	VaneIdVector SampleSensorFactory::GetIdsOfType( const VaneID /*dataType*/ ) const
	{
		//Since vehicles and other objects own sensors, and not this factory itself,
		//we are not responsible for keeping track of the IDs we create.
		VaneIdVector output;
		return output;
	}

	//////////////////////////////////////////////////////////////////////////

	void SampleSensorFactory::RegisterProperties()
	{
		PropertyManager& propMgr = PropertyManager::GetSingleton();

		//Sample properties
		propMgr.RegisterPropertyProvider( Types::SampleSensor, this);
		propMgr.RegisterProperty(Types::SampleSensor, "Frame Rate", "Frame rate to be sent", false);
		propMgr.RegisterProperty(Types::SampleSensor, "Quality Factor", "Image compression quality factor", false);
	}

	//////////////////////////////////////////////////////////////////////////

	ObjectPropertySet SampleSensorFactory::GetProperties( VaneID objID )
	{
		uint32 dataType = GetDataType(objID);

		ObjectPropertySet output;

		Sensor* pBaseSensor = SensorManager::GetSingleton().GetSensor( objID ).GetPointer();

		if ( !pBaseSensor )
		{
			VANEError();
			return output;
		}

		//Verify that the ID is of the correct type
		if ( dataType == Types::SampleSensor )
		{
			SampleSensor* pSensor = static_cast<SampleSensor*>(pBaseSensor);
			if ( pSensor != NULL )
			{
				//We also need to manually include our base sensor ID for the 
				//property system, adding it to the "isA" list.
				output.properties = GetSensorProperties( *pSensor );
				output.isAIds.push_back( pSensor->GetBaseSensorID() );
			}
		}

		return output;
	}

	//////////////////////////////////////////////////////////////////////////

	StaticAssetParamsPtr SampleSensorFactory::ParseXmlParams( const TiXmlElement* pXmlParams )
	{
		SensorStaticAssetParams* pSensorParams = NULL;

		String sensorType = XmlUtils::GetStringAttribute(pXmlParams, "sensorType");

		if( sensorType == kSensorTypeSampleSensor )
		{
			SampleSensorStaticAssetParams* pParams = new SampleSensorStaticAssetParams();
			pSensorParams = pParams;

			//Do sensor specific XML parsing here
			pParams->m_intData = XmlUtils::GetUnsignedIntAttribute( pXmlParams, "intData", 0 );
		}
		else
		{
			//Unhandled sensor type! All cases should be handled.
			VANEError();
			return StaticAssetParamsPtr();
		}

		//the following is necessary for all sensors
		pSensorParams->m_worldObjectAssetName = XmlUtils::GetStringAttribute(pXmlParams, "objectAsset");
		pSensorParams->m_sensorType = sensorType;
		pSensorParams->m_sampleRate = XmlUtils::GetDoubleAttribute(pXmlParams, "sampleRate");
		pSensorParams->m_assetName = XmlUtils::GetStringAttribute(pXmlParams, "name");
		pSensorParams->m_assetPreview = "";
		pSensorParams->m_category = "Sensors";
		pSensorParams->m_isPlaceable = true;

		return StaticAssetParamsPtr( pSensorParams );
	}

	//////////////////////////////////////////////////////////////////////////

	Sensor* SampleSensorFactory::CreateSampleSensor( SensorStaticAssetParams& params, DynamicAssetParams& dynamicParams )
	{
		SampleSensor* pSensor = new SampleSensor( GetNextSensorID(), params, dynamicParams );
		return pSensor;
	}

	//////////////////////////////////////////////////////////////////////////

	Sensor* SampleSensorFactory::CreateSensor( SensorStaticAssetParams& params, DynamicAssetParams& dynamicParams )
	{
		Sensor* pSensor = NULL;
		if( params.m_sensorType == kSensorTypeSampleSensor )
		{
			pSensor = CreateSampleSensor( params, dynamicParams );
		}
		else
		{
			LogMessage("Unhandled Sensor type attempting creation from SampleSensorFactory.", kLogMsgError );
		}

		return pSensor;
	}
	
	//////////////////////////////////////////////////////////////////////////
	
}
