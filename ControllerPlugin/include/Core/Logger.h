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
#ifndef VANE_Logger_h
#define VANE_Logger_h

#include "Platform.h"
#include "VaneTypes.h"
#include "CoreTypes.h"

#if VANE_COMPILER == VANE_COMPILER_MSVC
#pragma warning( push )
#pragma warning( disable:4251 )//STL container export warning can be ignored since nothing should use the exported vector...
#endif

namespace VANE
{
	
	///Enumerates the different types of log messages we can log.  They will be notated differently
	///based on this type, and possibly displayed in different colors if we are using a GUI
	enum LogMsgType
	{
		kLogMsgStandard = 0x01, ///< A standard output to the log (will be black in GUI)
		kLogMsgWarning	= 0x02, ///< An important but not critical notification (Yellow)
		kLogMsgError	= 0x04,	///< A critical message for the user, will display in red
		kLogMsgSpecial	= 0x08, ///< Used for a message that should stand out, but is not a warning or error
		kLogMsgPlugin	= 0x10, ///< Used for messages from a plugin
	};

	static const unsigned int kAllLogMsgTypes = 0xFF; 

	///An interface to anything that can accept log messages
	class VaneCoreExport LogListener
	{
	public:
		///Notification that an event has been logged
		virtual void OnMessageLogged(const String& msg, LogMsgType type)=0;
	};

	///Simple interface for logging any type of message to the
	///Vane.log file and the user console
	///@param msg - The string message to be displayed
	///@param LogMsgType Optional param to specify specific message types, such as errors and warnings
	///@param flush Optional param to specify if buffer should be flush to file immediately, if we are writing large
	//              numbers of messages may be more efficient not to.
	///@param logToFileOnly Default is to pass msg to all "Listeners" after writing to file, to write to file
	///                     only set this to true (may be faster)
	void VaneCoreExport LogMessage( const String & msg, LogMsgType type = kLogMsgStandard, bool flush = true, bool logToFileOnly = false);
	

	/// In Release, log message to file only (not console)
	/// otherwise log to Console if logToConsole = true
	///@param msg   - The string message to be displayed
	///@param logToConsole  - if true, log to console in Debug Runs
	///@param flush - If True, flush file buffer immediately
	void VaneCoreExport LogDebugMessage( const String & msg, bool logToConsole = false, bool flush = false);

	///A Data Logger that uses the VANE FileSystem to write logging info to a file
	///This is not a singleton so that we can create more than one logger if we need to
	class VaneCoreExport Logger
	{
	public:
		///Create the default logger and do any other initialization
		static void InitializeLoggingSystem();

		///Destroy the default logger and do any other cleanup for the logging system
		static void ShutdownLoggingSystem();

		///Get the default Data Logger.  This
		///We must have a default one to fall back on if
		///there are no other ways to log data.
		static Logger& GetDefaultLogger();
		
		///Check if the logger has been initialized.
		static bool IsInitialized();
	
	public:
		///default constructor
		Logger();
		
		///create a logger with the specified file name
		///throws an exception if unsuccessful
		Logger(const String& filename);

		//default destructor (closes down file, etc)
		~Logger();

		///Log a message.
		///By default, flush message to file immediately (may be more efficient not to, however)
		/// If "logToFileOnly", listener's will not be called (in particular, nothing will be written
		/// to console)
		void LogMessage(const String& msg, LogMsgType type = kLogMsgStandard, bool flush = true, bool logToFileOnly = false);

		///Flush the file to disk
		void Flush();

		///Add a listener that will receive messages from this log
		///This can be used to forward information to another channel, such
		///as an console window, or a GUI display
		void AddListener(LogListener* listener);

		///Open the log file if we do not currently already have an opened log file
		void OpenLogFile();

		///@return A string specifying the Log initialization details or errors
		String GetLogFilePath() const { return m_logFilePath; }

	private:
		//our file to use
		VANE::FileSystem::FileHandle m_logFile;

		//the listeners to notify
		std::vector<LogListener*> m_listeners;

		//Internally used initialization information
		String m_logFilePath;

		//a static logger so we always have access to at least one.
		static Logger* ms_defaultLogger;
	};

}

#if VANE_COMPILER == VANE_COMPILER_MSVC
#pragma warning( pop )
#endif

#endif
