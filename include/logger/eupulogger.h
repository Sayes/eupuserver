#ifndef _EUPULOGGER_H_
#define _EUPULOGGER_H_

#include <log4cxx/logger.h>
#include <log4cxx/propertyconfigurator.h>
#include <log4cxx/basicconfigurator.h>
#include <log4cxx/helpers/exception.h>
#include <log4cxx/consoleappender.h>
#include <log4cxx/patternlayout.h>
#include <log4cxx/logstring.h>

using namespace log4cxx;

class CEupuLogger {
public:
	CEupuLogger();

	CEupuLogger(const char* path);

	virtual ~CEupuLogger();

protected:
	void InitLogger(const char* lpstrConfig);

	void Trace(LoggerPtr lpLogger, char* lpstrTrace);
	void Info(LoggerPtr lpLogger, char* lpstrInfo);
	void Debug(LoggerPtr lpLogger, char* lpstrDebug);
	void Fatal(LoggerPtr lpLogger, char* lpstrFatal);
	void Warn(LoggerPtr lpLogger, char* lpstrWarn);
	void Error(LoggerPtr lpLogger, char* lpstrError);

	char szDataTime[30];
	char szMsg[256];

};

#endif//_EUPULOGGER_H_
