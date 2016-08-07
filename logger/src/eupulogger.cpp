#include "eupulogger.h"

#ifdef WITHOUTLOG
CEupuLogger::CEupuLogger(){}
CEupuLogger::CEupuLogger(const char* path){}
CEupuLogger::~CEupuLogger(){}
void CEupuLogger::InitLogger(const char* lpstrConfig){}
void CEupuLogger::Trace(LoggerPtr lpLogger, char* lpstrTrace){}
void CEupuLogger::Info(LoggerPtr lpLogger, char* lpstrInfo){}
void CEupuLogger::Debug(LoggerPtr lpLogger, char* lpstrDebug){}
void CEupuLogger::Fatal(LoggerPtr lpLogger, char* lpstrFatal){}
void CEupuLogger::Warn(LoggerPtr lpLogger, char* lpstrWarn){}
void CEupuLogger::Error(LoggerPtr lpLogger, char* lpstrError){}
#else
CEupuLogger::CEupuLogger()
{
    PropertyConfigurator::configure("loggercfg.cfg");
}

CEupuLogger::CEupuLogger(const char* path)
{
}

CEupuLogger::~CEupuLogger()
{
}

void CEupuLogger::Trace(LoggerPtr lpLogger, char* lpstrTrace)
{
    LOG4CXX_TRACE(lpLogger, lpstrTrace);
}

void CEupuLogger::Info(LoggerPtr lpLogger, char* lpstrInfo)
{
    LOG4CXX_INFO(lpLogger, lpstrInfo);
}

void CEupuLogger::Debug(LoggerPtr lpLogger, char* lpstrDebug)
{
    LOG4CXX_DEBUG(lpLogger, lpstrDebug);
}

void CEupuLogger::Warn(LoggerPtr lpLogger, char* lpstrWarn)
{
    LOG4CXX_WARN(lpLogger, lpstrWarn);
}

void CEupuLogger::Error(LoggerPtr lpLogger, char* lpstrError)
{
    LOG4CXX_ERROR(lpLogger, lpstrError);
}

void CEupuLogger::Fatal(LoggerPtr lpLogger, char* lpstrFatal)
{
    LOG4CXX_FATAL(lpLogger, lpstrFatal);
}
#endif
