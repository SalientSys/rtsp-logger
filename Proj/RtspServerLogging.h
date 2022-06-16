///
/// This file contains declarations that are useful for RTSP Server logging.

#pragma once

#include <Poco/Logger.h>
#include <thread>
#include <sstream>
#include <cassert>
#include <vector>

#include "string"

namespace RTSPServerlogging
{
	/// Logger names for components.	
	/// 
	/// @Note: This should match our `logging::LoggerComponentNameStr`
	const char* const LoggerComponentNameStr[] =
	{
		"Dummy_Component_1",
		"Dummy_Component_2",
		"Dummy_Component_3",
		"Dummy_Component_4",
		"Dummy_Component_5",
		"Dummy_Component_6",
		"Dummy_Component_7",
		"Dummy_Component_8",
		"Dummy_Component_9",
		"Dummy_Component_10",
		"Dummy_Component_11",
		"Dummy_Component_12",
		"Dummy_Component_13",
		"Dummy_Component_14",
		"Dummy_Component_15",
		"Dummy_Component_16",
		"Dummy_Component_17",
		"Dummy_Component_18",

		"MainServer.RtspServer"				// Rtsp Server Component
	};


	/// Component IDs
	enum class ComponentId
	{
		RtspServerId = 18,			// We need this value to match our other components too.
	};


	/// Default number of days to keep log files for
	const int DEFAULT_NUM_LOG_DAYS = 7;
	/// Default log priority for the component.
	const int DEFAULT_LOG_PRIORITY = static_cast<int>(Poco::Message::Priority::PRIO_TRACE);


	/// Handles logging message along with other details.
	class LogDetails
	{
	public:
		///
		/// Constructor.
		///
		/// @param[in] Msg		Message to log.
		/// @param[in] File		File that message originated from.
		/// @param[in] Function	Function that message originated from.
		/// @param[in] Line		Line that message originated on.
		/// @param[in] Tid		Thread ID that message originated on.
		LogDetails(const std::string& Msg, const char* File, const char* Function, int Line,
			const std::thread::id Tid) :
			m_msg(Msg),
			m_fileName(File),
			m_functionName(Function),
			m_lineNumber(Line),
			m_threadId(Tid)
		{
		}

		///
		/// Concatenates message and relevant information for logging.
		///
		/// @return Complete string ready to be logged.
		inline std::string ToString()
		{
			std::stringstream stream;
			stream << m_threadId;

			// Hide any personal info.
			size_t pos = m_fileName.find_last_of("/\\");
			std::string processedFileName = m_fileName.substr(pos + 1);

			return (processedFileName + "\t"
				+ std::to_string(m_lineNumber) + "\t"
				+ m_functionName + "\t"
				+ "TID: " + stream.str() + "\t"
				+ m_msg);
		}

	private:
		/// Message to be logged.
		const std::string m_msg;

		/// Thread ID.
		const std::thread::id m_threadId;

		/// Filename.
		std::string m_fileName;

		/// Function name.
		std::string m_functionName;

		/// Line number.
		int m_lineNumber;
	};


	///
	/// Getter for logger instance.
	///
	/// @param[in] ID enum for the component to get logger for.
	///
	/// @return Logger instance for that component.
	inline Poco::Logger& GetLogger(const ComponentId ID)
	{
		return (Poco::Logger::get(LoggerComponentNameStr[static_cast<int>(ID)]));
	}

	/// Get handle to RTSP logger.
	/// 
	/// @return RTSP logger handle.
	inline Poco::Logger& GetRtspServerLogger() {
		auto& logger = GetLogger(ComponentId::RtspServerId);
		return (logger);
	}


	/// #define's for logging levels.
#define rtsp_fatal(logger, msg) \
		if (logger.fatal()) \
			logger.fatal(RTSPServerlogging::LogDetails(msg, __FILE__, __FUNCTION__, __LINE__, std::this_thread::get_id()).ToString()); \
		else \
			(void) 0

#define rtsp_error(logger, msg) \
		if (logger.error()) \
			logger.error(RTSPServerlogging::LogDetails(msg, __FILE__, __FUNCTION__, __LINE__, std::this_thread::get_id()).ToString()); \
		else \
			(void) 0

#define rtsp_warning(logger, msg) \
		if (logger.warning()) \
			logger.warning(RTSPServerlogging::LogDetails(msg, __FILE__, __FUNCTION__, __LINE__, std::this_thread::get_id()).ToString()); \
		else \
			(void) 0

#define rtsp_information(logger, msg) \
		if (logger.information()) \
			logger.information(RTSPServerlogging::LogDetails(msg, __FILE__, __FUNCTION__, __LINE__, std::this_thread::get_id()).ToString()); \
		else \
			(void) 0

#define rtsp_trace(logger, msg) \
		if (logger.trace()) \
			logger.trace(RTSPServerlogging::LogDetails(msg, __FILE__, __FUNCTION__, __LINE__, std::this_thread::get_id()).ToString()); \
		else \
			(void) 0

#ifdef DEBUG
#define rtsp_debug(logger, msg) \
		if (logger.debug()) \
			logger.debug(RTSPServerlogging::LogDetails(msg, __FILE__, __FUNCTION__, __LINE__, std::this_thread::get_id()).ToString()); \
		else \
			(void) 0
#else
#define rtsp_debug(logger,msg) 
#endif


/// #define's we are using to log.
#define log_rtsp_fatal(msg)			rtsp_fatal(RTSPServerlogging::GetRtspServerLogger(), msg)
#define log_rtsp_error(msg)			rtsp_error(RTSPServerlogging::GetRtspServerLogger(), msg)
#define log_rtsp_warning(msg)		rtsp_warning(RTSPServerlogging::GetRtspServerLogger(), msg)
#define log_rtsp_information(msg)	rtsp_information(RTSPServerlogging::GetRtspServerLogger(), msg)
#define log_rtsp_trace(msg)			rtsp_trace(RTSPServerlogging::GetRtspServerLogger(), msg)
#define log_rtsp_debug(msg)			rtsp_debug(RTSPServerlogging::GetRtspServerLogger(), msg)

	class RTSPLogger
	{
	public:
		/// Default constructor.
		/// Note: Do not use this explicitly.
		RTSPLogger() = default;

		/// Create.
		/// 
		/// @param[in]	logFileDir			Directory for log file.
		/// @param[in]	rotateOnOpen		Whether to roatate file.
		/// @param[in]	flushImmediately	Whether to flush messages immediately.
		/// @param[in]	logPriority			Log level priority.
		/// @param[in]	numLogDays			Duration to purge file.
		RTSPLogger(const std::string& logFileDir, bool rotateOnOpen = false, bool flushImmediately = false,
			int logPriority = DEFAULT_LOG_PRIORITY, int numLogDays = DEFAULT_NUM_LOG_DAYS);

		/// Destroy.
		~RTSPLogger() = default;

	private:
		/// Initalizes logger.
		void init();

		/// Creates directory.
		/// 
		/// @param[in]	directory	Directory to be created.
		/// 
		/// @return true if successfull.
		bool createDir(const std::string& directory);

		// Data -
			/// Initialized ?
		bool m_isInitialized;

		/// Log file destination directory.
		std::string m_logFileBaseDir;

		/// file rotation?
		bool m_rotateOnOpen;

		// Flush messages?
		bool m_flushImmediately;

		// Log level priority.
		int m_logPriority;

		// Duration to purge file.
		int m_numLogDays;
	};

}// RTSPServerlogging
