// Copyright shenyizhong@gmail.com, 2014

#include <sys/stat.h>
#include <sys/types.h>
#include <iostream>
#ifdef OS_LINUX
#include <sys/time.h>
#elif OS_WINDOWS
#include <time.h>
#include <ws2tcpip.h>
#endif
#include <stdarg.h>
#include <string>
#include "common/common.h"
#include "common/utility.h"
#include "logger/eupulogger4system.h"

#ifdef WITHOUTLOG
CEupuLogger4System *CEupuLogger4System::CreateInstance(const char *spath) { return NULL; }
CEupuLogger4System *CEupuLogger4System::Logger() { return NULL; }
void CEupuLogger4System::Release() {}
void CEupuLogger4System::Fatal4Sys(const std::string &strFatal) {}
void CEupuLogger4System::Error4Sys(const std::string &strError) {}
void CEupuLogger4System::Debug4Sys(const std::string &strDebug) {}
void CEupuLogger4System::Fatal4Sys(char *strFatal) {}
void CEupuLogger4System::Error4Sys(char *strError) {}
void CEupuLogger4System::Debug4Sys(char *strDebug) {}
void CEupuLogger4System::WriteMonitorLog(uint32_t type, uint32_t mainid, uint32_t assiantid,
                                         uint32_t action, const char *username,
                                         const char *domain) {}
void CEupuLogger4System::WriteLog(const char *filename, int32_t line, LOGLEVEL level,
                                  const char *fmt, ...) {}
void CEupuLogger4System::SetDebugMode(bool bdebug) {}
void CEupuLogger4System::WriteHex(const char *filename, int32_t line, LOGLEVEL level,
                                  const char *title, const char *buf, int32_t buflen) {}
void CEupuLogger4System::SetLogLevel(LOGLEVEL level) {}
const char *CEupuLogger4System::GetLogLevelStr(LOGLEVEL) { return NULL; }
CEupuLogger4System::CEupuLogger4System() {}
CEupuLogger4System::CEupuLogger4System(const char *spath) {}
CEupuLogger4System::~CEupuLogger4System() {}
#else

#define MAX_MSGSIZE 2048

CEupuLogger4System *CEupuLogger4System::m_pLogger = NULL;

std::string ConvertStr2Hex(const char *buf, int32_t buflen) {
  char sbuf[MAX_MSGSIZE * 3];
  int32_t binlen = sizeof(sbuf);

  if (!buf || (buflen < 0) || (binlen < buflen * 3 + 1)) {
    return "";
  }

  memset(sbuf, 0, binlen);
  unsigned char ch;
  for (int32_t i = 0; i < buflen; i++) {
    ch = (unsigned char)buf[i];
    sprintf(sbuf + 3 * i, "%2.2x ", ch);
  }

  return std::string(sbuf);
}

CEupuLogger4System *CEupuLogger4System::CreateInstance(const char *spath) {
  if (m_pLogger == NULL) {
    m_pLogger = new CEupuLogger4System(spath);
  }

  return m_pLogger;
}

CEupuLogger4System *CEupuLogger4System::Logger() {
  if (m_pLogger == NULL) {
    m_pLogger = new CEupuLogger4System;
  }

  return m_pLogger;
}

void CEupuLogger4System::Release() {
  if (m_pLogger != NULL) {
    delete m_pLogger;
  }
}

CEupuLogger4System::CEupuLogger4System() : m_ErrPtr(0), m_FtlPtr(0), m_BugPtr(0) {
  m_strTmp = "";
  m_Level = LL_DEBUG;
  m_IsDebug = false;
  m_ErrPtr = log4cxx::Logger::getLogger("err4sys");
  m_FtlPtr = log4cxx::Logger::getLogger("fatal4sys");
  m_BugPtr = log4cxx::Logger::getLogger("debug4sys");
}

CEupuLogger4System::CEupuLogger4System(const char *spath)
    : CEupuLogger(spath), m_ErrPtr(0), m_FtlPtr(0), m_BugPtr(0) {
  m_strTmp = "";
  m_Level = LL_INFO;
  m_IsDebug = false;
  m_ErrPtr = log4cxx::Logger::getLogger("err4sys");
  m_FtlPtr = log4cxx::Logger::getLogger("fatal4sys");
  m_BugPtr = log4cxx::Logger::getLogger("debug4sys");
}

CEupuLogger4System::~CEupuLogger4System() {
  // dtor
}

void CEupuLogger4System::Fatal4Sys(const std::string &strFatal) {
  Fatal4Sys((char *)strFatal.c_str());
}

void CEupuLogger4System::Fatal4Sys(char *strFatal) {
  if (strFatal == NULL || m_FtlPtr == NULL) {
    return;
  }

  Fatal(m_FtlPtr, strFatal);
}

void CEupuLogger4System::Error4Sys(const std::string &strError) {
  Error4Sys((char *)strError.c_str());
}

void CEupuLogger4System::Error4Sys(char *strError) {
  if (strError == NULL || m_ErrPtr == NULL) {
    return;
  }

  Error(m_ErrPtr, strError);
}

void CEupuLogger4System::Debug4Sys(const std::string &strDebug) {
  Debug4Sys((char *)strDebug.c_str());
}

void CEupuLogger4System::Debug4Sys(char *strDebug) {
  if (strDebug == NULL || m_BugPtr == NULL) {
    return;
  }

  Debug(m_BugPtr, strDebug);
}

void CEupuLogger4System::WriteLog(const char *filename, int32_t line, LOGLEVEL level,
                                  const char *fmt, ...) {
  char msgbuf[MAX_MSGSIZE];
  char buf[MAX_MSGSIZE];
  struct timeval tmv;
  struct tm tme;
  va_list arglist;

  // check the level
  if (level > m_Level) {
    // level lower
    return;
  }

  memset(msgbuf, 0, sizeof(msgbuf));
  memset(buf, 0, sizeof(buf));
  memset(&tmv, 0, sizeof(tmv));
  memset(&tme, 0, sizeof(tme));

  gettimeofday(&tmv, NULL);

// get the log string
#ifdef OS_LINUX
  localtime_r(&tmv.tv_sec, &tme);

  va_start(arglist, fmt);
  vsnprintf(msgbuf, sizeof(msgbuf), fmt, arglist);
  va_end(arglist);

  int32_t pid = (int32_t)getpid();
  snprintf(buf, sizeof(buf),
           "%04d/%02d/%02d %02d:%02d:%02d:%06u [0x%08x, "
           "0x%08x] <%-5s> (%s, %06d) %s",
           tme.tm_year + 1900, tme.tm_mon + 1, tme.tm_mday, tme.tm_hour, tme.tm_min, tme.tm_sec,
           (uint32_t)tmv.tv_usec, pid, (int32_t)pthread_self(), GetLogLevelStr(level), filename,
           line, msgbuf);

#elif OS_WINDOWS
  time_t tm_sec = tmv.tv_sec;
  struct tm *p = localtime(&tm_sec);

  va_start(arglist, fmt);
  vsnprintf(msgbuf, sizeof(msgbuf), fmt, arglist);
  va_end(arglist);

  int32_t pid = 0;
  int32_t threadid = 0;
  snprintf(buf, sizeof(buf),
           "%04d/%02d/%02d %02d:%02d:%02d:%06u [0x%08x, "
           "0x%08x] <%-5s> (%s, %06d) %s",
           p->tm_year + 1900, p->tm_mon + 1, p->tm_mday, p->tm_hour, p->tm_min, p->tm_sec,
           (uint32_t)tmv.tv_usec, pid, threadid, GetLogLevelStr(level), filename, line, msgbuf);

#endif

  if (m_IsDebug) {
    std::cout << buf;
  }

  Error(m_ErrPtr, buf);
}

void CEupuLogger4System::WriteMonitorLog(uint32_t type, uint32_t mainid, uint32_t assiantid,
                                         uint32_t action, const char *username,
                                         const char *domain) {
  char buf[MAX_MSGSIZE];
  struct timeval tmv;
  struct tm tme;

  memset(buf, 0, sizeof(buf));
  memset(&tmv, 0, sizeof(tmv));
  memset(&tme, 0, sizeof(tme));

  gettimeofday(&tmv, NULL);

#ifdef OS_LINUX
  localtime_r(&tmv.tv_sec, &tme);

  char tmpbuf[100];
  memset(tmpbuf, 0, sizeof(tmpbuf));
  std::string strkey;
  if (type == 1) {
    strkey = fgNtoA(mainid);
    snprintf(tmpbuf, sizeof(tmpbuf), "%s_%u", strkey.c_str(), assiantid);
    strkey = std::string(tmpbuf);
  } else {
    snprintf(tmpbuf, sizeof(tmpbuf), "%u_%u", mainid, assiantid);
    strkey = std::string(tmpbuf);
  }

  // get the log string
  snprintf(buf, sizeof(buf), "%04d-%02d-%02d %02d:%02d:%02d---%u---%s---%s---%s---%u",
           tme.tm_year + 1900, tme.tm_mon + 1, tme.tm_mday, tme.tm_hour, tme.tm_min, tme.tm_sec,
           type, GETNULLSTR(strkey), GETNULLPTR(username), GETNULLPTR(domain), action);

#elif OS_WINDOWS
  time_t tm_sec = tmv.tv_sec;
  struct tm *p = localtime(&tm_sec);

  char tmpbuf[100];
  memset(tmpbuf, 0, sizeof(tmpbuf));
  std::string strkey;
  if (type == 1) {
    strkey = fgNtoA(mainid);
    snprintf(tmpbuf, sizeof(tmpbuf), "%s_%u", strkey.c_str(), assiantid);
    strkey = std::string(tmpbuf);
  } else {
    snprintf(tmpbuf, sizeof(tmpbuf), "%u_%u", mainid, assiantid);
    strkey = std::string(tmpbuf);
  }

  // get the log string
  snprintf(buf, sizeof(buf), "%04d-%02d-%02d %02d:%02d:%02d---%u---%s---%s---%s---%u",
           p->tm_year + 1900, p->tm_mon + 1, p->tm_mday, p->tm_hour, p->tm_min, p->tm_sec, type,
           GETNULLSTR(strkey), GETNULLPTR(username), GETNULLPTR(domain), action);

#endif

  Fatal(m_FtlPtr, buf);
}

void CEupuLogger4System::SetDebugMode(bool bdebug) { m_IsDebug = bdebug; }

void CEupuLogger4System::SetLogLevel(LOGLEVEL level) { m_Level = level; }

void CEupuLogger4System::WriteHex(const char *filename, int32_t line, LOGLEVEL level,
                                  const char *title, const char *buf, int32_t buflen) {
  char msgbuf[MAX_MSGSIZE];
  struct timeval tmv;
  struct tm tme;

  // parameter check
  if (!buf || (buflen <= 0)) {
    return;
  }

  // check the level
  if (level > m_Level) {
    // level lower
    return;
  }

  memset(msgbuf, 0, sizeof(msgbuf));
  memset(&tmv, 0, sizeof(tmv));
  memset(&tme, 0, sizeof(tme));

  gettimeofday(&tmv, NULL);

#ifdef OS_LINUX
  localtime_r(&tmv.tv_sec, &tme);

  std::string hexdata = ConvertStr2Hex(buf, buflen);
  int32_t pid = (int32_t)getpid();
  snprintf(msgbuf, sizeof(msgbuf),
           "%04d/%02d/%02d %02d:%02d:%02d:%06u "
           "[0x%08x, 0x%08x] <%-5s> (%s, %06d) %s: "
           "\n%s",
           tme.tm_year + 1900, tme.tm_mon + 1, tme.tm_mday, tme.tm_hour, tme.tm_min, tme.tm_sec,
           (uint32_t)tmv.tv_usec, pid, (int32_t)pthread_self(), GetLogLevelStr(level), filename,
           line, GETNULLPTR(title), GETNULLSTR(hexdata));
#elif OS_WINDOWS
  time_t tm_sec = tmv.tv_sec;
  struct tm *p = localtime(&tm_sec);

  std::string hexdata = ConvertStr2Hex(buf, buflen);
  int32_t pid = 0;
  int32_t threadid = 0;
  snprintf(msgbuf, sizeof(msgbuf),
           "%04d/%02d/%02d %02d:%02d:%02d:%06u [0x%08x, "
           "0x%08x] <%-5s> (%s, %06d) %s: \n%s",
           p->tm_year + 1900, p->tm_mon + 1, p->tm_mday, p->tm_hour, p->tm_min, p->tm_sec,
           (uint32_t)tmv.tv_usec, pid, threadid, GetLogLevelStr(level), filename, line,
           GETNULLPTR(title), GETNULLSTR(hexdata));
#endif

  // if(m_IsDebug)
  //{
  //  cout << msgbuf << endl;
  //}

  Error(m_ErrPtr, msgbuf);
}

const char *CEupuLogger4System::GetLogLevelStr(LOGLEVEL level) {
  switch (level) {
    case LL_ERROR:
      return "ERROR";
    case LL_WARN:
      return "WARN";
    case LL_INFO:
      return "INFO";
    case LL_DEBUG:
      return "DEBUG";
    default:
      return "UNUSE";
  }
  return "UNUSE";
}

#endif
