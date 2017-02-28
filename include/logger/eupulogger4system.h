#ifndef _EUPULOGGER4SYSTEM_H_
#define _EUPULOGGER4SYSTEM_H_

#include "logger/eupulogger.h"
#include "common/globaldef.h"

typedef enum {
  LL_ERROR = 1,
  LL_WARN,
  LL_INFO,
  LL_DEBUG,
} LOGLEVEL;

#define _ERROR_ __FILE__, __LINE__, LL_ERROR
#define _WARN_ __FILE__, __LINE__, LL_WARN
#define _INFO_ __FILE__, __LINE__, LL_INFO
#define _DEBUG_ __FILE__, __LINE__, LL_DEBUG

#define GETNULLPTR(s) (s) == NULL || strlen((s)) <= 0 ? "NULL" : (s)
#define GETNULLSTR(s) (s).empty() ? "NULL" : (s).c_str()

class CEupuLogger4System : public CEupuLogger {
public:
  static CEupuLogger4System *CreateInstance(const char *spath);
  static CEupuLogger4System *Logger();
  static void Release();
  void Fatal4Sys(const std::string &strFatal);
  void Error4Sys(const std::string &strError);
  void Debug4Sys(const std::string &strDebug);
  void Fatal4Sys(char *strFatal);
  void Error4Sys(char *strError);
  void Debug4Sys(char *strDebug);
  void WriteMonitorLog(uint32_t type, uint32_t mainid, uint32_t assiantid,
                       uint32_t action, const char *username,
                       const char *domain);
  void WriteLog(const char *filename, int32_t line, LOGLEVEL level,
                const char *fmt, ...);
  void SetDebugMode(bool bdebug);
  void WriteHex(const char *filename, int32_t line, LOGLEVEL level,
                const char *title, const char *buf, int32_t buflen);
  void SetLogLevel(LOGLEVEL level);
  const char *GetLogLevelStr(LOGLEVEL);

protected:
  CEupuLogger4System();
  CEupuLogger4System(const char *spath);
  virtual ~CEupuLogger4System();

protected:
  log4cxx::LoggerPtr m_ErrPtr;
  log4cxx::LoggerPtr m_FtlPtr;
  log4cxx::LoggerPtr m_BugPtr;
  std::string m_strTmp;
  bool m_IsDebug;
  LOGLEVEL m_Level;

private:
  static CEupuLogger4System *m_pLogger;
};

#ifdef WITHOUTLOG

#define LOGSETLEVEL
#define LOGSETDEBUG
#define LOG
#define LOGHEX
#else
#define LOGSETLEVEL CEupuLogger4System::Logger()->SetLogLevel
#define LOGSETDEBUG CEupuLogger4System::Logger()->SetDebugMode
#define LOG CEupuLogger4System::Logger()->WriteLog
#define LOGHEX CEupuLogger4System::Logger()->WriteHex
#endif

#endif //_EUPULOGGER4SYSTEM_H_
