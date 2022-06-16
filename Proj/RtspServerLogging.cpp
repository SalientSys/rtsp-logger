#include "pch.h"

//#include <boost/filesystem.hpp>

#include "Poco/Message.h"
#include "Poco/FormattingChannel.h"
#include "Poco/AsyncChannel.h"
#include "Poco/PatternFormatter.h"
#include "Poco/WindowsConsoleChannel.h"
#include "Poco/SplitterChannel.h"
#include "Poco/FileChannel.h"

#include "RTSPServerlogging.h"

using namespace Poco;
using namespace RTSPServerlogging;


RTSPServerlogging::
RTSPLogger::
RTSPLogger(const std::string& logFileDir, bool rotateOnOpen, bool flushImmediately,
	int logPriority, int numLogDays) :
	m_logFileBaseDir(logFileDir),
	m_rotateOnOpen(rotateOnOpen),
	m_flushImmediately(flushImmediately),
	m_logPriority(logPriority),
	m_numLogDays(numLogDays),
	m_isInitialized(false)
{
	init();
}


void
RTSPLogger::
init()
{
	if (m_isInitialized)
	{
		return;
	}
	else
	{
		//// Setup a formatter for each line in the file.
		AutoPtr<Formatter> pFileFormatter;
		// [Year]-[Month]-[Date] [Hours]:[Minutes]:[Seconds] [Log Level]
		// [Log Message Text]
		pFileFormatter.assign(new PatternFormatter("%Y-%m-%d %H:%M:%S %p\t\t%t\n"));
		// We want local times
		pFileFormatter->setProperty("times", "local");

		// Setup log directory.
		const auto& success = createDir(m_logFileBaseDir);
		assert(success);

		const std::string rotateOnOpenStr = (m_rotateOnOpen ? "true" : "false");
		const std::string flushImmediatelyStr = (m_flushImmediately ? "true" : "false");
		const std::string purgeAgeStr = std::to_string(m_numLogDays) + "days.";

		// Setup logger.
		Logger& logger = Logger::get(LoggerComponentNameStr[static_cast<int>(ComponentId::RtspServerId)]);
		// Set the log level
		logger.setLevel(m_logPriority);

		// Setup the file channel
		AutoPtr<FileChannel> pFileChannel;
		// Set the log path
		pFileChannel.assign(new FileChannel(m_logFileBaseDir + "\\" + LoggerComponentNameStr[static_cast<int>(ComponentId::RtspServerId)] + ".log"));
		// Rotate daily
		pFileChannel->setProperty("rotation", "daily");
		// Flush log statements to file immediately.
		pFileChannel->setProperty("flush", flushImmediatelyStr);
		// Append timestamp to file name for archiving
		pFileChannel->setProperty("archive", "timestamp");
		// Use timestamp in local time
		pFileChannel->setProperty("times", "local");
		// Keep desired number of days of archived logs
		pFileChannel->setProperty("purgeAge", purgeAgeStr);
		// Rotate log file on next open
		pFileChannel->setProperty("rotateOnOpen", rotateOnOpenStr);

		// Setup a formatting channel that contains the file channel
		AutoPtr<FormattingChannel> pFileFormattingChannel;
		pFileFormattingChannel.assign(new FormattingChannel(pFileFormatter.get()));
		// Set file formatter to output to file
		pFileFormattingChannel->setChannel(pFileChannel.get());

		// Setup splitter channel for forwarding to multiple downstream channels
		AutoPtr<SplitterChannel> pSplitterChannel;
		pSplitterChannel.assign(new SplitterChannel());
		// Add channel to splitter
		pSplitterChannel->addChannel(pFileFormattingChannel.get());

		// Async channel used to handle message queuing on separate thread
		AutoPtr<AsyncChannel> pAsyncChannel;
		pAsyncChannel.assign(new AsyncChannel());
		// Wrap all of this in an async channel
		pAsyncChannel->setChannel(pSplitterChannel.get());

		// Set the main channel for the logger
		logger.setChannel(pAsyncChannel.get());

		// Force rotation.
		if (m_rotateOnOpen)
		{
			pFileChannel->open();
		}

		m_isInitialized = true;
	}
}


bool
RTSPLogger::
createDir(const std::string& directory)
{
	assert(!directory.empty());

	// @TODO: Make it portable.
	//auto dirCreated = boost::filesystem::create_directory(directory);
	//return dirCreated;

	DWORD attribs = GetFileAttributes(directory.c_str());
	if (attribs == INVALID_FILE_ATTRIBUTES ||
		(attribs & FILE_ATTRIBUTE_DIRECTORY) == 0)
	{
		if (!CreateDirectory(directory.c_str(), 0))
			return false;
	}

	return true;
}
