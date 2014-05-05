#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <stdarg.h>
#include <string>
#include "eupulogger4system.h"
#include "utility.h"
#include "common.h"

#define MAX_MSGSIZE 2048

using std::cout;
using std::string;
using std::endl;

CEupuLogger4System* CEupuLogger4System::m_pLogger= NULL;
 
string ConvertStr2Hex(const char *buf, int buflen) 
{
	char sbuf[MAX_MSGSIZE * 3];
	int binlen = sizeof(sbuf);

	if (!buf || (buflen < 0) || (binlen < buflen * 3 + 1))
	{
		return "";
	}

	memset(sbuf, 0, binlen);
	unsigned char ch;
	for (int i = 0; i < buflen; i++)
	{
		ch = (unsigned char) buf[i];
		sprintf(sbuf + 3 * i, "%2.2x ", ch);
	}

	return string(sbuf);
}

CEupuLogger4System* CEupuLogger4System::CreateInstance(const char *spath) 
{
    if (m_pLogger == NULL)
    {
        m_pLogger = new CEupuLogger4System(spath);
    }

    return m_pLogger;
}

CEupuLogger4System* CEupuLogger4System::Logger()
{
    if (m_pLogger == NULL)
    {
        m_pLogger = new CEupuLogger4System;
    }

    return m_pLogger;
}

void CEupuLogger4System::Release()
{
    if (m_pLogger != NULL)
    {
        delete m_pLogger;
    }
}

CEupuLogger4System::CEupuLogger4System() :
    m_ErrPtr(0), m_FtlPtr(0), m_BugPtr(0)
{
    m_strTmp = "";
    m_Level = LL_INFO;
    m_IsDebug = false;
    m_ErrPtr = Logger::getLogger("err4sys");
    m_FtlPtr = Logger::getLogger("fatal4sys");
    m_BugPtr = Logger::getLogger("debug4sys");
}

CEupuLogger4System::CEupuLogger4System(const char * spath) : CEupuLogger(spath),
    m_ErrPtr(0), m_FtlPtr(0), m_BugPtr(0)
{
    m_strTmp = "";
    m_Level = LL_INFO;
    m_IsDebug = false;
    m_ErrPtr = Logger::getLogger("err4sys");
    m_FtlPtr = Logger::getLogger("fatal4sys");
    m_BugPtr = Logger::getLogger("debug4sys");
}

CEupuLogger4System::~CEupuLogger4System()
{
    //dtor
}

void CEupuLogger4System::Fatal4Sys(const string &strFatal)
{
    Fatal4Sys((char*)strFatal.c_str());
}

void CEupuLogger4System::Fatal4Sys(char* strFatal)
{
    if (strFatal == NULL || m_FtlPtr == NULL)
    {
        return;
    }

    Fatal(m_FtlPtr, strFatal);
}

void CEupuLogger4System::Error4Sys(const string &strError)
{
    Error4Sys((char*)strError.c_str());
}

void CEupuLogger4System::Error4Sys(char* strError)
{
    if (strError == NULL|| m_ErrPtr == NULL)
    {
        return;
    }

    Error(m_ErrPtr, strError);
}

void CEupuLogger4System::Debug4Sys(const string &strDebug)
{
    Debug4Sys((char*)strDebug.c_str());
}

void CEupuLogger4System::Debug4Sys(char* strDebug)
{
    if (strDebug == NULL|| m_BugPtr == NULL)
    {
        return;
    }

    Debug(m_BugPtr, strDebug);
}

void CEupuLogger4System::WriteLog(const char *filename, int line, LOGLEVEL level, const char *fmt, ...)
{
	char msgbuf[MAX_MSGSIZE];
	char buf[MAX_MSGSIZE];
	struct timeval tmv;
	struct tm tme;
	va_list arglist;

	//check the level
	if (level > m_Level)
	{
		//level lower
		//return;
	}

	memset(msgbuf, 0, sizeof(msgbuf));
	memset(buf, 0, sizeof(buf));
	memset(&tmv, 0, sizeof(tmv));
	memset(&tme, 0, sizeof(tme));

	gettimeofday(&tmv, NULL);
	localtime_r(&tmv.tv_sec, &tme);

	va_start(arglist, fmt);
	vsnprintf(msgbuf, sizeof(msgbuf), fmt, arglist);
	va_end(arglist);

	//get the log string
    snprintf(buf, sizeof(buf), "%04d/%02d/%02d %02d:%02d:%02d:%06u [0x%08x, 0x%08x] <%-5s> (%s, %06d) %s",  
           tme.tm_year+1900, tme.tm_mon+1, tme.tm_mday, tme.tm_hour, tme.tm_min, tme.tm_sec, (unsigned int)tmv.tv_usec,
           (int)getpid(), (int)pthread_self(), GetLogLevelStr(level), filename, line, msgbuf);

	//if (m_IsDebug)
	//{
//		cout << buf;
//	}

    switch (level)
    {
        case LL_DEBUG:
            Debug(m_BugPtr, buf);
            break;
        case LL_ERROR:
            Error(m_ErrPtr, buf);
            break;
        default:
            Error(m_ErrPtr, buf);
            break;

    }

}

void CEupuLogger4System::WriteMonitorLog(UINT type, UINT mainid, UINT assiantid, UINT action, const char *username, const char *domain)
{
    //char msgbuf[MAX_MSGSIZE];
    char buf[MAX_MSGSIZE];
    struct timeval tmv;
    struct tm tme;

    //memset(msgbuf, 0, sizeof(msgbuf));
    memset(buf, 0, sizeof(buf));
    memset(&tmv, 0, sizeof(tmv));
    memset(&tme, 0, sizeof(tme));

    gettimeofday(&tmv, NULL);
    localtime_r(&tmv.tv_sec, &tme);

    char tmpbuf[100];
    bzero(tmpbuf, sizeof(tmpbuf));
    string strkey;
    if(type == 1)
    {
        strkey = fgNtoA(mainid);
        snprintf(tmpbuf, sizeof(tmpbuf), "%s_%u",strkey.c_str(), assiantid);
        strkey = string(tmpbuf);
    }else
    {
        snprintf(tmpbuf, sizeof(tmpbuf), "%u_%u", mainid, assiantid);
        strkey = string(tmpbuf);
    }

    //get the log string
    snprintf(buf, sizeof(buf), "%04d-%02d-%02d %02d:%02d:%02d---%u---%s---%s---%s---%u",  
           tme.tm_year+1900, tme.tm_mon+1, tme.tm_mday, tme.tm_hour, tme.tm_min, tme.tm_sec,
            type, GETNULLSTR(strkey), GETNULLPTR(username), GETNULLPTR(domain), action);

    Fatal(m_FtlPtr, buf);
}

void CEupuLogger4System::SetDebugMode(bool bdebug)
{
    m_IsDebug = bdebug;
}

void CEupuLogger4System::SetLogLevel(LOGLEVEL level)
{
    m_Level = level;
}

void CEupuLogger4System::WriteHex(const char *filename, int line, LOGLEVEL level, const char *title, const char *buf, int buflen)
{
	char msgbuf[MAX_MSGSIZE];
	struct timeval tmv;
	struct tm tme;

	//parameter check
	if (!buf || (buflen <= 0))
	{
		return;
	}

	//check the level
	if (level > m_Level)
	{
		//level lower
		return;
	}

	memset(msgbuf, 0, sizeof(msgbuf));
	memset(&tmv, 0, sizeof(tmv));
	memset(&tme, 0, sizeof(tme));

	gettimeofday(&tmv, NULL);
	localtime_r(&tmv.tv_sec, &tme);

	string hexdata = ConvertStr2Hex(buf, buflen);

  snprintf(msgbuf, sizeof(msgbuf), "%04d/%02d/%02d %02d:%02d:%02d:%06u [0x%08x, 0x%08x] <%-5s> (%s, %06d) %s: \n%s",
           tme.tm_year+1900, tme.tm_mon+1, tme.tm_mday, tme.tm_hour, tme.tm_min, tme.tm_sec, (unsigned int)tmv.tv_usec,
           (int)getpid(), (int)pthread_self(), GetLogLevelStr(level), filename, 
           line, GETNULLPTR(title), GETNULLSTR(hexdata));
/*
  if(m_IsDebug)
  {
     cout << msgbuf << endl; 
  }
*/
  Error(m_ErrPtr, msgbuf);
}

const char * CEupuLogger4System::GetLogLevelStr(LOGLEVEL level)
{
    switch(level)
    {
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
