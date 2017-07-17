// Copyright shenyizhong@gmail.com, 2014

#include "logger/eupulogger.h"

#ifdef WITHOUTLOG
CEupuLogger::CEupuLogger() {}
CEupuLogger::CEupuLogger(const char *path) {}
CEupuLogger::~CEupuLogger() {}
void CEupuLogger::InitLogger(const char *lpstrConfig) {}
void CEupuLogger::Trace(log4cxx::LoggerPtr lpLogger, char *lpstrTrace) {}
void CEupuLogger::Info(log4cxx::LoggerPtr lpLogger, char *lpstrInfo) {}
void CEupuLogger::Debug(log4cxx::LoggerPtr lpLogger, char *lpstrDebug) {}
void CEupuLogger::Fatal(log4cxx::LoggerPtr lpLogger, char *lpstrFatal) {}
void CEupuLogger::Warn(log4cxx::LoggerPtr lpLogger, char *lpstrWarn) {}
void CEupuLogger::Error(log4cxx::LoggerPtr lpLogger, char *lpstrError) {}
#else
CEupuLogger::CEupuLogger() {
    log4cxx::PropertyConfigurator::configure("loggercfg.cfg");
}

CEupuLogger::CEupuLogger(const char *path) {}

CEupuLogger::~CEupuLogger() {}

void CEupuLogger::Trace(log4cxx::LoggerPtr lpLogger, char *lpstrTrace) {
    LOG4CXX_TRACE(lpLogger, lpstrTrace);
}

void CEupuLogger::Info(log4cxx::LoggerPtr lpLogger, char *lpstrInfo) {
    LOG4CXX_INFO(lpLogger, lpstrInfo);
}

void CEupuLogger::Debug(log4cxx::LoggerPtr lpLogger, char *lpstrDebug) {
    LOG4CXX_DEBUG(lpLogger, lpstrDebug);
}

void CEupuLogger::Warn(log4cxx::LoggerPtr lpLogger, char *lpstrWarn) {
    LOG4CXX_WARN(lpLogger, lpstrWarn);
}

void CEupuLogger::Error(log4cxx::LoggerPtr lpLogger, char *lpstrError) {
    LOG4CXX_ERROR(lpLogger, lpstrError);
}

void CEupuLogger::Fatal(log4cxx::LoggerPtr lpLogger, char *lpstrFatal) {
    LOG4CXX_FATAL(lpLogger, lpstrFatal);
}
#endif
