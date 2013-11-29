#ifndef _EXCEPTIONHANDLE_H_INCLUDED
#define _EXCEPTIONHANDLE_H_INCLUDED

#include <stdlib.h>
#include <execinfo.h>
#include <signal.h>

#include <exception>
#include <iostream>
#include <string>
#include <ostream>
#include <iomanip>

#define ERROR_MSG_LENGTH                256

//#define __try  try {

//#define __catch  } catch (std::exception& e) {
    //char szErrorMsg[ERROR_MSG_LENGTH];
    //std::cout << __FILE__ << __FUNCTION__ << __LINE__ << e.what()<< std::endl;

//#define __endcatch }

///////////////////////////////////////////////////////////
class ExceptionTracer
{
public:
    ExceptionTracer()
    {
        void* array[25];
        int nSize = backtrace(array, 25);
        char** symbols = backtrace_symbols(array, nSize);

        for (int i = 0; i < nSize; i++)
        {
            std::cout << symbols[i] << std::endl;
        }
        free(symbols);
    }
};

///////////////////////////////////////////////////////////

template<class SignalException> class SignalTranslator
{
private:
    class SingleTonTranslator
    {
public:
        SingleTonTranslator()
        {
            signal(SignalException::GetSignalNumber(), SignalHandler);
        }
        static void SignalHandler(int)
        {
            throw SignalException();
        }
    };

public:
    SignalTranslator()
    {
        static SingleTonTranslator s_objTranslator;
    }
};

///////////////////////////////////////////////////////////
class SegmentationFault : public std::exception//,public ExceptionTracer
{

public:
    SegmentationFault()
    {
        _sExceptionInfo = "SegmentationFault";
    }

    virtual ~SegmentationFault() throw()
    {
    }

    static int GetSignalNumber()
    {
        return SIGSEGV;
    }

    virtual const char* what() const throw()
    {
        return _sExceptionInfo.c_str();
    }

private:
    std::string _sExceptionInfo;

};

///////////////////////////////////////////////////////////
class FloatingPointException : public std::exception //, public ExceptionTracer
{
public:
    FloatingPointException()
    {
        _sExceptionInfo = "FloatingPointException";
    }

    virtual ~FloatingPointException() throw()
    {

    }
    static int GetSignalNumber()
    {
        return SIGFPE;
    }

    virtual const char* what() const throw()
    {
        return _sExceptionInfo.c_str();
    }

private:
    std::string _sExceptionInfo;
};

///////////////////////////////////////////////////////////
class BusException : public ExceptionTracer, public std::exception //,public ExceptionTracer
{
public:
    BusException()
    {
        _sExceptionInfo = "BusException";
    }
    virtual ~BusException() throw()
    {

    }
    static int GetSignalNumber()
    {
        return SIGBUS;
    }

    virtual const char* what() const throw()
    {
        return _sExceptionInfo.c_str();
    }

private:
    std::string _sExceptionInfo;
};
//SignalTranslator<BusException> g_objBusExceptionTranslator;

extern SignalTranslator<FloatingPointException> g_objFloatingPointExceptionTranslator;
extern SignalTranslator<SegmentationFault> g_objSegmentationFaultTranslator;

///////////////////////////////////////////////////////////
/*
 class ExceptionHandler
 {
 private:
 class SingleTonHandler
 {
 public:
 SingleTonHandler()
 {
 set_terminate(Handler);
 }

 static void Handler()
 {
 // Exception from construction/destruction of global variables
 try
 {
 // re-throw
 throw;
 }
 catch (SegmentationFault &)
 {
 cout << __FILE__ << __FUNCTION__ << __LINE__ << endl;
 cout << "SegmentationFault" << endl;
 }
 catch (FloatingPointException &)
 {
 cout << __FILE__ << __FUNCTION__ << __LINE__ << endl;
 cout << "FloatingPointException" << endl;
 }
 catch (...)
 {
 cout << __FILE__ << __FUNCTION__ << __LINE__ << endl;
 cout << "Unknown Exception" << endl;
 }

 //if this is a thread performing some core activity
 //abort();
 // else if this is a thread used to service requests
 // pthread_exit();
 }
 };

 public:
 ExceptionHandler()
 {
 static SingleTonHandler s_objHandler;
 }
 };
 */

#endif // _EXCEPTIONHANDLE_H_INCLUDED
