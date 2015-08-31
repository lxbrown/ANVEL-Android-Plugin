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
#ifndef VaneCommands_h__
#define VaneCommands_h__

#include "Core/Variant.h"
#include "Core/VaneTypes.h"
#include "Core/Platform.h"


#if VANE_COMPILER == VANE_COMPILER_MSVC
//For MSVC, the CommandGroup needs a reference to its manager normally, so we put this
//here as a matter of convenience for now.  Remove this once CommandGroup has been 
//reconfigured to not need that.
#pragma warning( push )
#pragma warning( disable: 4355 ) 
#pragma warning( disable: 4251 ) 
#endif

#if VANE_CONFIG_DEBUG
#	define VANE_INCLUDE_COMMAND_DEBUG 1
#else
#	define VANE_INCLUDE_COMMAND_DEBUG 0
#endif 

class TiXmlElement;

namespace VANE
{
	/************************************************************************/
	/* Typedefs                                                             */
	/************************************************************************/

	typedef String				Category;
	typedef String				CommandName;
	typedef uint32				CommandID;
	typedef uint32              CommandResultID;

	///Parameter list defined in string format
	typedef Variant             CommandParameter;
	typedef VariantType::Value  CommandParameterType;
	typedef VariantVector       CommandParamList;

	typedef VariantVector       ParameterList;


	/************************************************************************/
	/* Constants                                                            */
	/************************************************************************/

	const CommandID kInvalidCommandID = 0;
	const CommandResultID kInvalidResultID = 0;
	const VaneID kParamTypeInfoNone = kInvalidVaneID;

	/************************************************************************/
	/* Enumerations                                                         */
	/************************************************************************/

	/// The possible results that can be returned from a command
	enum CommandResultValue
	{
		kCommandSuccess, ///< The Command Succeeded
		kCommandQueued,  ///< The Command has been Queued for processing
		kCommandFail,    ///< The Command has failed
	};

	///The error types that can be returned
	enum CommandErrorType
	{
		kNoError =0,            ///< No Error
		kInvalidCommand,        ///< Command does not exist
		kInvalidParameterType,  ///< Invalid parameter type specified
		kParameterOutsideRange, ///< Invalid parameter value passed in
		kCommandError,
		//...
	};
	
	///Used to Create Commands with CommandResult( Success ) notation
	enum CommandSuccessTag
	{
		Success=0
	};

	///Used to Create Commands with CommandResult( Queued ) notation
	enum CommandQueuedTag
	{
		Queued=0
	};


	/************************************************************************/
	/* CommandResult                                                        */
	/************************************************************************/

	///Provides result values and success/error codes for a command that has
	///been processed by Anvel
	struct CommandResult
	{
		//Create a success result
		CommandResult( CommandSuccessTag, CommandResultID resultID = kInvalidResultID )
			: m_result(kCommandSuccess)
			, m_error(kNoError)
			, m_queuedResultID( resultID )
			, m_msg("Command Success" )
		{	}
		
		///Create a Queued Result, meaning that the command has been queued, and 
		///has been assigned the given resultID if it requires a result. This
		///result can be used by a CommandResult listener to retrieve the actual result when
		///it has been fully processed..
		///@param[in] The queued tag to specify that the command has been queued
		///@param[in] resultID The assigned resultID if one will be available
		CommandResult( CommandQueuedTag, CommandResultID resultID = kInvalidResultID  )
			: m_result( kCommandQueued )
			, m_error( kNoError )
			, m_queuedResultID( resultID )
		{	}

		///Default Constructor which creates an invalid command result
		CommandResult() 
			: m_result(kCommandFail)
			, m_error(kInvalidCommand)
			, m_queuedResultID( kInvalidResultID ) 
			, m_msg("Command Failure. Provide more details!")
		{	}

		///Mostly full constructor
		///@param[in] type The error type, or kNoError
		///@param[in] msg The message to relay with this command result
		///@param[in] resultID A result ID if this command is queued.
		CommandResult( CommandErrorType type, const String& msg = "", CommandResultID resultID = kInvalidResultID )
		: m_result(kCommandFail)
		, m_error(type)
		, m_queuedResultID( resultID )
		{
			switch (type)
			{
			case kInvalidCommand:
				m_msg= "Invalid Command. " + msg;
				break;
			case kInvalidParameterType:
				m_msg = "Invalid Parameter Type.  " + msg;
				break;
			case kParameterOutsideRange:
				m_msg = "Invalid Parameter value. " + msg;
				break;
			case kCommandError:
				m_msg = "Error: " + msg;
				break;
			case kNoError:
				m_result = kCommandSuccess;
				break;
			}
		}

		///Full constructor 
		///@param[in] result
		///@param[in] type The error type, or kNoError
		///@param[in] msg The message to relay with this command result
		///@param[in] resultID A result ID if this command is queued.
		CommandResult(CommandResultValue result, CommandErrorType type, const String& msg, CommandResultID resultID = kInvalidResultID )
			: m_result(result)
			, m_error(type)
			, m_queuedResultID( resultID )
			, m_msg(msg)
		{	}

		//The result of this command, available immediately.
		CommandResultValue m_result;
		
		///If any errors occured with this command at this point.
		CommandErrorType   m_error;
		
		///If the Command has been queued, this ID will be used to 
		///pass the result to the correct listeners
		CommandResultID    m_queuedResultID;
		
		///Any descriptive data to go along with this result.
		String       m_msg;
		
		///Result values passed back from our Command
		ParameterList m_resultValues;

		///Utility methods to quickly check if the result succeeded, failed, or was queued.
		bool Succeeded() const { return (m_result == kCommandSuccess); }
		bool Queued()    const { return (m_result == kCommandQueued);  }
		bool Failed()    const { return (m_result == kCommandFail);    }
	};
	
	/************************************************************************/

	///A listener for command results
	///receives notification when a command has been processed
	class ICommandResultListener
	{
	public:
		virtual void CommandProcessed(const CommandResult& result)=0;
	};

	/************************************************************************/
	/* ParameterDescription                                                 */
	/************************************************************************/

	///Simple structure to describe the parameters for a command
	struct VaneCoreExport ParameterDescription
	{
		///Create a ParameterDescription
		///@param[in] type The data type of this parameter
		///@param[in] name The name of this parameter
		///@param[in] description A description of this parameter
		///@param[in] defaultValue The default value ( optional ) of this parameter 
		///@param[in] typeInfo Additional type information, beyond the generic type, such a FileName instead of a plain string
		///@param[in] optional If this parameter is optional or not
		ParameterDescription(CommandParameterType type, const String& name, const String& description, Variant defaultValue = Variant::INVALID, VaneID typeInfo = kParamTypeInfoNone, bool optional = false );

		/// Get the name of this parameter
		///@return The name of this parameter
		const String& GetName() const;

		/// Get the description of this parameter
		///@return the description of this parameter
		const String& GetDescription() const;

		/// Get the type of this parameter
		///@return The type of this parameter
		CommandParameterType GetType() const;

		/// Get the specialized type info about this parameter
		///@return The specialized type info about this parameter
		VaneID GetTypeInfo() const;
		
		/// Get whether or not this is optional
		///@return true if this is an optional param
		bool IsOptional() const;
		
		///Write out a description for this parameter
		String  ToString() const;

		//////////////////////////////////////////////////////////////////////////
		// Data

		//@todo - make these private

		//the default value
		Variant m_default;
		
		///the name of this parameter
		String	m_name;
		
		///the description (optional) of this parameter
		String  m_description;

		///the data type of this parameter
		CommandParameterType m_type;

		///Additional specialized info about this parameter. For example, the main type could be an integer value
		///but the specialized type info could denote that this is actually a vehicleID, or a portNumber, or timestamp value...
		VaneID m_typeInfo;

		///if this is an optional parameter
		bool    m_optional;
	};

	typedef std::vector<ParameterDescription> ParameterDescriptionList;
	typedef ParameterDescriptionList ResultDescriptionList;

	/************************************************************************/
	/* CommandDescription                                                   */
	/************************************************************************/

	///A simple structure describing a command
	///It contains some descriptive strings and a collection
	///of individual parameter descriptions
	struct VaneCoreExport CommandDescription
	{
		///Short constructor using just ID and description
		///@param[in] id The Unique CommandID for this CommandDescription. This is returned from CommandManager.RegisterCommand()
		///@param[in] description The description of this command
		CommandDescription( CommandID id, const String& description );

		///Create a CommandDescription using the full constructor
		///@param[in] id The Unique CommandID for this CommandDescription. This is returned from CommandManager.RegisterCommand()
		///@param[in] category The category this command belongs to. Basically an organizational group to put related
		///           commands together, whether or not they come from the same source. Plugins should be able to add
		///           additonal commands to the vehicle menu, for example
		///@param[in] name The unique name for this command
		///@param[in] description The description of what this command does.
		CommandDescription( CommandID id, const String& category, const String& name, const String& description );

		//default constructor just does default init
		CommandDescription(); 

		///Verify that the given list of parameters apply to this function
		///@todo - add in capability for optional arguments
		///@return true if there are the correct number of parameters
		///and they are of the expected type, false otherwise
		bool VerifyParameters(const CommandParamList& params) const;

		///Create a list of parameters for this command based on an input string.
		///This is used for command line processing.  Since the command description
		///knows what type of parameters to check, it will be able to parse in
		///each type as a string and validate that it is valid for the expected
		///defintion.
		///@note
		const CommandParamList ParseFromString(const String& parameterString) const;

		///Create a list of parameters for this command based on an input string.
		///This is used for command line processing.  Since the command description
		///knows what type of parameters to check, it will be able to parse in
		///each type as a string and validate that it is valid for the expected
		///definition.
		///@return a list of validated parameters for this type, otherwise an empty
		///list of parameters
		const CommandParamList ParseFromXml(const TiXmlElement* pCmdElement) const;
		
		///Write out the set of parameters as a large string
		///@param[in] paramValues
		///@return The string for the parameters 
		String SerializeToString( const CommandParamList& paramValues ) const;

		///Write a command to Xml, based on a given set of parameters.
		///This can be used to generate network commands in Xml form, to save commands
		///to a script that can be loaded later, etc.
		///@param[in] pCmdElement The Xml element to fill out
		///@param[in] paramValues The parameter values to use when filling out the command in Xml.
		TiXmlElement* WriteToXml(const CommandParamList& paramValues ) const;

		///Write out a description for this command
		///@return a string description of this command
		String ToString() const;

		///Write out a description for this command in HTML format
		///@return a string description of this command in HTML format
		///@param[in] flags optional flags to report with the html description
		String ToHTMLString( uint32 flags = 0) const;

		///Lookup a parameters type (or if it exists) by the string name
		///@param[in] The name of the parameter
		///@return The type of parameter it is, or VariantType::kInvalid if it does not exist
		VariantType::Value GetParameterType(const String& paramName) const;

		///Lookup a parameters index (or if it exists) by the string name
		///@param[in] The name of the parameter
		///@return The index if found, otherwise -1
		int32 GetParameterIndex(const String& paramName) const;

		//////////////////////////////////////////////////////////////////////////
		// Data

		///An invalid CommandDescription
		static const CommandDescription Invalid;

		///Our Unique CommandID
		CommandID m_id;

		///the scope the command belongs to
		String	  m_category;

		///the name of this command
		String    m_name;

		///the description of this command
		String    m_description;

		///Description of all of our parameters
		ParameterDescriptionList m_parameters;
		
		//Description of the results of this command
		ParameterDescriptionList m_results;
		
		///If we are currently enabled or not.
		bool      m_enabled;

	};
	
	/************************************************************************/
	/* XmlParameterList                                                     */
	/************************************************************************/
	/// Class Encapsulates a Command Parameter List,
	/// implement functionality to write it to Xml /
	/// Read from Xml
	class VaneCoreExport  XmlParameterList
	{
	public:

		/// 
		///@param[in] Parameter Descriptions
		XmlParameterList( const ParameterDescriptionList& parameters = ParameterDescriptionList() )
			: m_parameterDescriptions( parameters )
		{	}
	
		/// Read Parameter List from Xml
		/// (ParameterDescriptionList must be initialized first!)
		///@param[in] pCmdElement   Parameter List in Xml Form
		///@return ParameterList
		const ParameterList ReadFromXml(  const TiXmlElement* pCmdElement ) const;

		/// Set Parameter Values
		///@param[in] parameterValues
		///@return
		void SetParameterValues( const ParameterList& parameterValues);
 
		/// Write to Xml
		///@param[in] pParamListElement   Optional top-level element, one will be created
		///                               if NULL
		///@return  Encapsulated parameter list in Xml form
		TiXmlElement* WriteToXml(  TiXmlElement* pParamListElement = NULL) const;

		/// Add a parameter to Parameter list
		///@param[in] parameterDescription
		///@param[in] val  value
		///@return number of parameters in XmlParameterList
		uint32 AddParameter( const ParameterDescription& parameterDescription, const Variant& val = Variant::INVALID);

		/// Get Parameter Values
		///@return  Parameter Values
		const ParameterList& GetParameterValues() const { return m_parameterValues; }
	private:
		/// Get Parameter Index from Name
		///@param[in] paramName
		///@return index, or -1 if paramName not found
		int32 GetParameterIndex(  const String& paramName ) const;

		ParameterList            m_parameterValues;

		ParameterDescriptionList m_parameterDescriptions;
	};


	/************************************************************************/
	/* CommandData                                                          */
	/************************************************************************/

	///A structure representing the data necessary to execute a command.
	///Includes the description and a complete set of parameters.
	struct CommandData
	{
		CommandDescription m_desc;
		CommandParamList m_paramChoices;
		CommandParamList m_defaults;
		uint32 m_flags;
		
		CommandData() : m_flags( 0 ) {}
	};
	
	typedef std::vector<CommandID> CommandIDVector;

	/************************************************************************/
	/* ICommandHandler                                                      */
	/************************************************************************/

	///Interface for all modules that can accept VANE Commands
	///Commands allow all plugins and processes to talk to one another
	///
	class VaneCoreExport ICommandHandler
	{
	public:
		/// Get a list of possible values for each parameter for the specified command.
		/// NOTE: Not all commands or parameters may have choices and can either be left blank
		/// or assigned a value of Variant::INVALID.
		/// @param[in] commandID The command to get possible parameter choices for.
		virtual CommandParamList GetParameterChoices( CommandID commandID ) { VANE_UNUSED(commandID); return CommandParamList(); }
		
		/// Get a list of updated default values for the given parameters
		/// NOTE: Not all commands or parameters may have choices and can either be left blank
		/// or assigned a value of Variant::INVALID.
		/// @param[in] commandID The command to get the default values for.
		virtual CommandParamList GetParameterDefaults( CommandID commandID ) { VANE_UNUSED(commandID); return CommandParamList(); }

		///Handle a command / Execute a command
		///@param commandID		  The ID of the command being executed
		///@param parameterList	  The parameters that go with this parameter
		virtual CommandResult HandleCommand( CommandID commandID, const CommandParamList& parameterList)=0;
		
#if VANE_USE_STRING_PARAMS
		///Handle a command / Execute a command with a string param list
		///@param commandID		  The ID of the command being executed
		///@param parameterList	  The parameters that go with this parameter in string form 
		///example: "1.0 'somename' 'stuff'"
		virtual CommandResult HandleCommand( CommandID commandID, const String& parameterList)=0;
#endif
		///Query for a description of the command. 
		///@returns A descriptive structure of information about the command
		virtual const CommandDescription& GetCommandDescription( CommandID commandID ) = 0;

		///Query this command handler for a list of its supported commands
		//@returns A vector of all the commands this handler supports
		virtual std::vector<CommandName> GetSupportedCommands()=0;

		///Get the scope this command handler belongs to.
		///Commands can be preceded by the scope name to avoid overlap
		///example: Simulation, Vehicles, Physics. Simulation.AdvanceTime, Simulation.Exit, etc.
		virtual Category GetCategory() const=0;

		///Validate conditional actions for a specific instance.
		///@param[in] instanceId The id of the instance that action might apply to; already of correct type
		///@param[in] commandId The id of the command that makes up the action
		///@return True if the action should be included for the instance's action list, false if not
		virtual bool ValidateConditionalActionForInstance(VaneID instanceId, CommandID commandId)
			{ VANE_UNUSED(instanceId); VANE_UNUSED(commandId); return true; }
	};

	/************************************************************************/
	/* ICommandRegistrar                                                    */
	/************************************************************************/

	///Interface for a class that can register commands
	///
	class ICommandRegistrar
	{
	public:
		///Register a command
		///@param[in] commandName The name of the command to register
		///@param[in] handler	  The handler to use for this command
		///@param[in] flags		  Flags for the command.
		virtual CommandID RegisterCommand( const String& commandName, ICommandHandler* handler, uint32 flags = 0 )=0;
	
		///Unregister a command
		///@param[in] commandID   The ID of the command to unregister
		virtual void UnregisterCommand( CommandID commandID )=0;

		///Get the names of all the registered categories
		///@returns A vector of the names of all registered categories.
		virtual StringVector GetRegisteredCategoryNames()=0;

		///Get the registered command names by the scope they belong to.
		///@param[in] scopeName Name of the scope to get commands for.
		///@param[in] includeHidden Whether to include hidden commands in the list.
		///@returns A vector of all the command names within the given scope.
		virtual std::vector<CommandName> GetRegisteredCommandsByCategory( const Category& scopeName, uint32 includeFlags, uint32 excludeFlags ) = 0;

		///Get the description for this command
		///@param[in] commandID the ID of this command
		///@returns a command description for the command. This can have nothing in it.
		virtual const CommandDescription& GetCommandDescription( CommandID commandID ) const = 0;
	
		///Get the description for this command
		///@param[in] command The command
		///@returns a command description for the command. This can have nothing in it.
		virtual const CommandDescription& GetCommandDescription( CommandName command ) const = 0;

		///Check if any new commands have been added since we last checked.
		///This assumes that ONLY ONE thing will be checking/clearing this, as we really should
		///only need to update one thing if there have been commands added, such as the main menu
		virtual bool CheckIfCommandsAdded() const =0;
		
		///Reset the Commands Added status to mark that we have read and updated
		///based on the current condition of the command manager.
		virtual void ClearCommandsAddedFlag()=0; 
	};

	/************************************************************************/
	/* CommandGroup                                                         */
	/************************************************************************/

	///This is a utility class that can be used to reduce the complexity
	///of handling commands.  Commands are added to the group and then
	///registered as a group when RegisterCommands is called.
	class VaneCoreExport CommandGroup : public ICommandHandler
	{
	public:
		///Creates a command group with the given name
		///@param[in] scope Name of the scope this group of commands belongs to.
		CommandGroup(const Category& category );

		///Add a command to this group
		void AddCommand(const CommandDescription& description);

		///Query for a description of the command. 
		///@returns A descriptive string describing the command
		const CommandDescription & GetCommandDescription( CommandID commandID);

		///Query this command handler for a list of its supported commands
		//@returns A vector of all the commands this handler supports
		std::vector<CommandName> GetSupportedCommands();

		///Get the scope this command handler belongs to.
		///Commands can be preceded by the scope name to avoid overlap
		///example: Simulation, Vehicles, Physics. Simulation.AdvanceTime, Simulation.Exit, etc.
		virtual Category GetCategory() const;
		
		///Enable or disable a command
		void SetCommandEnabled( CommandID command, bool enabled );
	
	protected:
		const Category m_category;

		std::vector<CommandDescription> m_commands;
		
	private:
		CommandGroup& operator= (const CommandGroup& rhs );

#if VANE_INCLUDE_COMMAND_DEBUG
		///Simple extra debug tracking for this class
		struct DebugInfo
		{
			DebugInfo()
			: m_registered(false)
			{

			} 
			
			bool m_registered;
		};	

		DebugInfo m_debugInfo;
#endif

	};
}


#endif // Commands_h__
