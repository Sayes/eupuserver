#ifndef _EUPULOGGER_H_
#define _EUPULOGGER_H_

#ifdef WITHOUTLOG
namespace log4cxx {
class LoggerPtr{

};
}
#else
#include <log4cxx/logger.h>
#include <log4cxx/propertyconfigurator.h>
#include <log4cxx/basicconfigurator.h>
#include <log4cxx/helpers/exception.h>
#include <log4cxx/consoleappender.h>
#include <log4cxx/patternlayout.h>
#include <log4cxx/logstring.h>

#endif

class CEupuLogger {
public:
    CEupuLogger();

    CEupuLogger(const char* path);

    virtual ~CEupuLogger();

protected:
    void InitLogger(const char* lpstrConfig);

    void Trace(log4cxx::LoggerPtr lpLogger, char* lpstrTrace);
    void Info(log4cxx::LoggerPtr lpLogger, char* lpstrInfo);
    void Debug(log4cxx::LoggerPtr lpLogger, char* lpstrDebug);
    void Fatal(log4cxx::LoggerPtr lpLogger, char* lpstrFatal);
    void Warn(log4cxx::LoggerPtr lpLogger, char* lpstrWarn);
    void Error(log4cxx::LoggerPtr lpLogger, char* lpstrError);

    char szDataTime[30];
    char szMsg[256];

};

#endif//_EUPULOGGER_H_
