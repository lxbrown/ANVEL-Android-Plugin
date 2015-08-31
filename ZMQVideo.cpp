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

#include "Core/Logger.h"
#include "Core/StringConverter.h"
#include "ZMQVideo.h"
#include "Simulation/Controller/ControllerManager.h"
#include "Simulation/Vehicles/Vehicle.h"
#include "Simulation/Vehicles/VehicleManager.h"




namespace VANE
{
	static const DataTypeGUID kZMQVideoGUID = { 0x4cc98610, 0xa5c8, 0x4ab7, { 0xa1, 0x38, 0xef, 0x34, 0xb1, 0xaf, 0x65, 0xb8 } };

	namespace Types
	{
		VaneID ZMQVideo = kInvalidVaneID;
	}

	namespace Commands
	{
		CommandID kCommandUseZMQVideo = kInvalidCommand;
	}

	namespace Controller
	{
		const String kZMQVideoName = "ZMQVideo";
	}
}

using namespace VANE;
using namespace VANE::Controller;
using namespace jpge;

const String ZMQVideo::kTypeZMQVideo = kZMQVideoName;
const String kInputThrottle = "Throttle";
const String kInputSteering = "Steering";

//////////////////////////////////////////////////////////////////////////
//
// ZMQVideoFactory
//
//////////////////////////////////////////////////////////////////////////

ZMQVideoFactory::ZMQVideoFactory()
	: m_commands( *this )
{
	DataTypeDescription desc = DataTypeDescription(kZMQVideoName, "Some description", "ZMQVideoFactory");
	Types::ZMQVideo = DataTypeManager::GetSingleton().RegisterDataType(desc, kZMQVideoGUID);
	
	m_currentID = MakeID( Types::ZMQVideo, 0 );

	Controller::Manager::GetSingleton().RegisterControllerFactory( this );

	RegisterZMQVideoProperties();
}

//////////////////////////////////////////////////////////////////////////

ZMQVideoFactory::~ZMQVideoFactory()
{
	Controller::Manager::GetSingleton().UnregisterControllerFactory( this );

	LogMessage( StringConverter::ToString( m_ownedControllers.size() ) ); 

	ControllerPtrMap::iterator it = m_ownedControllers.begin();
	ControllerPtrMap::iterator end = m_ownedControllers.end();

	while ( it != end )
	{
		Controller::Manager::GetSingleton().UnregisterController( it->second->GetBaseControllerID() );
		Controller::Manager::GetSingleton().UnregisterController( it->second->GetControllerID() );
		++it;
	}

}

//////////////////////////////////////////////////////////////////////////

ControllerID ZMQVideoFactory::CreateControllerInterface()
{
	ZMQVideo* pNewZMQVideo = new ZMQVideo();

	Controller::Manager& controllerMgr = Controller::Manager::GetSingleton();
	pNewZMQVideo->m_baseID   = controllerMgr.GetNextControllerID();
	pNewZMQVideo->m_id       = m_currentID++;

	ControllerPtr pController = ControllerPtr(pNewZMQVideo);
	m_ownedControllers.insert(std::make_pair(pController->GetControllerID(), pController));
	controllerMgr.RegisterController(pController);

	return pNewZMQVideo->GetControllerID();
}

//////////////////////////////////////////////////////////////////////////

void ZMQVideoFactory::DestroyControllerInterface(ControllerID id)
{
	ControllerPtrMap::iterator it = m_ownedControllers.find(id);
	if(it != m_ownedControllers.end())
		m_ownedControllers.erase(it);
}

/************************************************************************/

void ZMQVideoFactory::OnPropertyChanged( VaneID objId, PropertyIndex index )
{	
	ControllerPtrMap::iterator it = m_ownedControllers.find(objId);
	if(it == m_ownedControllers.end())
		return;

	ControllerPtr& pCtrlr = it->second;
	ZMQVideo& simCtrlr = static_cast<ZMQVideo&>( *pCtrlr.Get() );

	//Update based on what was changed
	switch (index)
	{
	case 0:
		//Change in output file name
		simCtrlr.ipaddr = simCtrlr.ipaddr;
		LogMessage("Set IP " + simCtrlr.ipaddr);
		break;
	case 1:
		//Make sure the desired speed is between 0 and 2 m/s
		simCtrlr.m_desired_speed = Math::Clamp(simCtrlr.m_desired_speed, -1.0, 2.0);
		break;
	case 2:
		//Make sure the desired turn rate is between -1.15 and 1.15 rad/s
		simCtrlr.m_desired_yaw = Math::Clamp(simCtrlr.m_desired_yaw, -1.15, 1.15);
		break;
	default:
		break;
	
	}
}

/************************************************************************/

ObjectPropertySet ZMQVideoFactory::GetProperties( VaneID objID )
{
	ObjectPropertySet result;
			
	ControllerPtrMap::iterator it = m_ownedControllers.find(objID);
	if(it == m_ownedControllers.end())
		return result;

	ControllerPtr& pCtrlr = it->second;
	ZMQVideo& simCtrlr = static_cast<ZMQVideo&>( *pCtrlr.Get() );

	result.properties.push_back(Property(simCtrlr.ipaddr));
	result.properties.push_back(Property(simCtrlr.m_desired_speed));
	result.properties.push_back(Property(simCtrlr.m_desired_yaw));

	return result;
}

VaneIdVector ZMQVideoFactory::GetIdsOfType( const VaneID dataType ) const
{
	if ( dataType != Types::ZMQVideo )
		return VaneIdVector();
				
	return VaneIdVector();
}

/************************************************************************/

void ZMQVideoFactory::RegisterZMQVideoProperties() 
{
	PropertyManager& propMgr = PropertyManager::GetSingleton();
	propMgr.RegisterPropertyProvider(Types::ZMQVideo, this);

	//Add properties: (Must be in same order they were pushed onto array in "GetProperties"
	propMgr.RegisterProperty(Types::ZMQVideo, "IP Address", "Current IP of this device", true);
	propMgr.RegisterProperty(Types::ZMQVideo, "Desired Speed", "Robot Desired Forward Speed Command", true);
	propMgr.RegisterProperty(Types::ZMQVideo, "Desired Yaw Rate", "Robot Desired Yaw Rate Command", true);
}

/************************************************************************/

ControllerPtr ZMQVideoFactory::GetController(ControllerID id)
{
	ControllerPtrMap::iterator it = m_ownedControllers.find(id);
	if(it != m_ownedControllers.end())
		return it->second;

	//Returns NULL
	return ControllerPtr();
}

//////////////////////////////////////////////////////////////////////////
//
// ZMQVideo 
//
//////////////////////////////////////////////////////////////////////////
//zmq::socket_t socket_;
zmq::socket_t socket1_;
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

ZMQVideo::ZMQVideo()
: m_controllableID( kInvalidVaneID )
, m_elapsedTime( 0 )
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
		socket1_.init(context_, ZMQ_PAIR);
		socket1_.bind("tcp://" + ip + ":5555");

		LogMessage("Connected", kLogMsgSpecial);
	}
	m_desired_speed = 0;
	m_desired_yaw = 0;
	m_throttle = 0;
	m_steering = 0;

	//Add our throttle and steering inputs
	m_inputs.push_back(ControlInput(kInputThrottle, kControlTypeAxis));
	m_inputs.push_back(ControlInput(kInputSteering, kControlTypeAxis));

	m_inputValues.resize(2, ControlValue(0));

	frame = 0;
}

//////////////////////////////////////////////////////////////////////////

ZMQVideo::~ZMQVideo()
{
	socket1_.close();
	context_.close();
}

//////////////////////////////////////////////////////////////////////////

void ZMQVideo::Update(TimeValue dt)
{
	m_elapsedTime += dt;

	if ( m_controllableID == kInvalidVaneID ) 
		return;
	
	Vehicles::Manager& vehMgr = Vehicles::Manager::GetSingleton();
	Vehicles::Vehicle* pVehicle = vehMgr.GetVehicle( m_controllableID );
	if ( !pVehicle )
		return;
	
	//Sending the image is dependent on the frame rate and if the user has closed the connection
	if((frame % (int) (100 / 15) == 0) && running) {
		//Keep both sockets syncronized
		zmq::message_t dummy(1);
		//snprintf((char *) dummy.data(), 5,"%d", frame);
		socket1_.send(dummy);

		//Recieve a response with the direction the vehicle should move
		zmq::message_t direction;
		socket1_.recv(&direction);
		String command = static_cast<char*>(direction.data());
		command.resize(1);

		//Set desired speed and yaw based on the direction given
		if(!command.compare("s")) { 
			//stay
			m_desired_speed = 0;
			m_desired_yaw = 0;
		}
		else if(!command.compare("f")) { 
			//forward
			m_desired_speed = 1;
			m_desired_yaw = 0;
		}
		else if(!command.compare("b")) { 
			//back
			m_desired_speed = -1;
			m_desired_yaw = 0;
		}
		else if(!command.compare("l")) { 
			//left
			m_desired_speed = 0;
			m_desired_yaw = 1;
		}
		else if(!command.compare("r")) { 
			//right
			m_desired_speed = 0;
			m_desired_yaw = -1;
		}
		else if(!command.compare("q")) { 
			//forward-left
			m_desired_speed = 1;
			m_desired_yaw = 1;
		}
		else if(!command.compare("w")) { 
			//forward-right
			m_desired_speed = 1;
			m_desired_yaw = -1;
		}
		else if(!command.compare("e")) { 
			//back-left
			m_desired_speed = -1;
			m_desired_yaw = -1;
		}
		else if(!command.compare("t")) { 
			//back-right
			m_desired_speed = -1;
			m_desired_yaw = 1;
		}
		else if(!command.compare("c")) {
			//close
			//Close the socket and continue playing without going through the ZMQ loop
			socket1_.close();
			context_.close();
			running = false;
			LogMessage("Connection has been closed. To reconnect please restart ANVEL", kLogMsgWarning);
		}
		else {
			LogMessage("Invalid Direction Received", kLogMsgError);
			LogMessage(command);
		}
	}
	frame++;

	CalculateControlValues(dt);
}

//////////////////////////////////////////////////////////////////////////

ControlValue ZMQVideo::GetInput( ControlInputIndex index ) const 
{
	if (index < m_inputValues.size() )
		return m_inputValues[index];
	return 0;
}

//////////////////////////////////////////////////////////////////////////

ControlValue ZMQVideo::GetInput( const ControlName& name ) const
{
	//Examples:
	if (name == "Throttle") return m_inputValues[0]; 
	if (name == "Steering") return m_inputValues[1];
	
	return 0;
}

//////////////////////////////////////////////////////////////////////////

std::vector<ControlValue> ZMQVideo::GetInputs() const
{
	return m_inputValues;
}

//////////////////////////////////////////////////////////////////////////

void ZMQVideo::ClearInputs()
{
	uint32 inputCount = m_inputValues.size();
	for(uint32 i = 0; i < inputCount; ++i)
		m_inputValues[i] = kDefaultControlValue;
}

//////////////////////////////////////////////////////////////////////////

std::vector<ControlInput> ZMQVideo::GetControls() const
{
	return m_inputs;
}

//////////////////////////////////////////////////////////////////////////

void ZMQVideo::SetControlMode( ControlMode /*mode*/ )
{
	
}

//////////////////////////////////////////////////////////////////////////

void ZMQVideo::AddControllable( VaneID controllableID )
{
	m_controllableID = controllableID;
}

//////////////////////////////////////////////////////////////////////////

void ZMQVideo::OnAttachedToObject( VaneID objectID )
{
	m_controllableID = objectID;
}

//////////////////////////////////////////////////////////////////////////

void ZMQVideo::CalculateControlValues( TimeValue dt )
{	
	// Apply limits and deadzones to desired speed and yaw
	//m_desired_speed = Math::Clamp(m_desired_speed, -1.0, 2.0);
	//m_desired_yaw = Math::Clamp(m_desired_yaw, -1.15, 1.15);
	if (Math::Abs(m_desired_speed)<0.4)
		m_desired_speed = 0.0;
	if (Math::Abs(m_desired_yaw)<0.45)
		m_desired_yaw = 0.0;

	//Calculate m_throttle and m_steering based on calibrations
	//Positive turn rate, positive forward speed
	if (m_desired_yaw>=0.45 && m_desired_speed>=0.4) {
		m_throttle = 0.6325*m_desired_speed+0.1024*m_desired_yaw-0.0094;
		m_steering = -0.0046*m_desired_speed-0.1800*m_desired_yaw+0.0035;
	}
	//Negative turn rate, positive forward speed
	else if (m_desired_yaw<=-0.45 && m_desired_speed>=0.4) {
		m_throttle = 0.6335*m_desired_speed-0.0917*m_desired_yaw-0.0115;
		m_steering = 0.0050*m_desired_speed-0.1805*m_desired_yaw-0.0043;
	}
	//Positive turn rate, negative forward speed
	else if (m_desired_yaw>=0.45 && m_desired_speed<=-0.4) {
		m_throttle = -1*(0.6335*-m_desired_speed+0.1024*m_desired_yaw-0.0094);
		m_steering = 0.0046*-m_desired_speed-0.1800*m_desired_yaw+0.0035;
	}
	//Negative turn rate, negative forward speed
	else if (m_desired_yaw<=-0.45 && m_desired_speed<=-0.4) {
		m_throttle = -1*(0.6335*-m_desired_speed-0.0917*m_desired_yaw-0.0115);
		m_steering = 0.0050*-m_desired_speed-0.1805*m_desired_yaw-0.0043;
	}
	//Nonzero turn rate, zero forward speed
	else if (Math::Abs(m_desired_yaw)>=0.45 && m_desired_speed<0.4 && m_desired_speed >-0.4) {
		m_throttle = 0;
		m_steering = -0.1179*m_desired_yaw;
	}
	//Zero turn rate, forward speed
	else {
		m_throttle = 0.6129*m_desired_speed-0.0001;
		m_steering = 0;
	}
	//TODO: Apply turn rate limit at high speeds or speed limit at high turn rate

	m_inputValues[0] = m_throttle;
	m_inputValues[1] = m_steering;
}

ZMQVideoFactory::ZMQVideoCommandGroup::ZMQVideoCommandGroup( ZMQVideoFactory& factory )
	: CommandGroup( "ZMQVideo" )
	, m_factory( factory )
{
	CommandManager& cmdMgr = CommandManager::GetSingleton();
	Commands::kCommandUseZMQVideo = cmdMgr.RegisterCommand( "UseZMQVideo", this, kCmdHidden );
			
	{	
		CommandDescription desc(Commands::kCommandUseZMQVideo, "Some Description.");
		desc.m_parameters.push_back(ParameterDescription(VariantType::kVaneID, "Vehicle", "Vehicle ID."));
		AddCommand(desc);
	}

	cmdMgr.RegisterObjectAction( Commands::kCommandUseZMQVideo, Types::Vehicle, 0 );

}

CommandResult ZMQVideoFactory::ZMQVideoCommandGroup::HandleCommand( CommandID commandID, const CommandParamList& parameterList )
{
	if (commandID == Commands::kCommandUseZMQVideo)
	{
		Controller::ControllerID controllerID = Controller::Manager::GetSingleton().CreateControllerOfType( "ZMQVideo" );
		Controller::ControllerPtr pController = Controller::Manager::GetSingleton().GetController( controllerID );
		if ( !pController.IsNull() )
		{
			ZMQVideo* pZMQVideo = (ZMQVideo*) pController.Get();
			
			VaneID vehicleID = parameterList[0].GetVaneIDValue();

			Controller::Manager::GetSingleton().SetVehicleController( vehicleID, pController );
					
			Vehicles::Vehicle* pVehicle = Vehicles::Manager::GetSingleton().GetVehicle( parameterList[0].GetVaneIDValue() );
			if ( pVehicle )
			{
				pVehicle->SetExternallyControlled( false );
				pVehicle->SetInputEnabled( true );

				return CommandResult(Success);
			}

			return CommandResult(kCommandFail, kParameterOutsideRange, "No valid vehicle was found with that ID.");
		}
	}

	return CommandResult(kInvalidCommand);
}