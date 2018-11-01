/** Copyright shenyizhong@gmail.com, 2014
 *  This file is part of the Mingw32 package.
 *  unistd.h maps     (roughly) to io.h
 */
#ifndef COMMON_MYUNISTD_H_
#define COMMON_MYUNISTD_H_

#ifdef OS_WINDOWS
#include <io.h>
#include <process.h>
#endif

#endif  // COMMON_MYUNISTD_H_
