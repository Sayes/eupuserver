// Copyright shenyizhong@gmail.com, 2014

#ifndef COMMON_EXCEPTIONHANDLE_CPP
#define COMMON_EXCEPTIONHANDLE_CPP

#include "common/exceptionhandle.h"

SignalTranslator<FloatingPointException> g_objFloatingPointExceptionTranslator;
SignalTranslator<SegmentationFault> g_objSegmentationFaultTranslator;

#endif  // COMMON_EXCEPTIONHANDLE_CPP
