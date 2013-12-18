/*
 * =====================================================================================
 *
 *       Filename:  threadlock.cpp
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  2013年11月30日 01时18分37秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (), 
 *   Organization:  
 *
 * =====================================================================================
 */

#include "threadlock.h"

CThreadLock::CThreadLock()
{
	pthread_mutex_init(&m_Mutex, NULL);
}

CThreadLock::~CThreadLock()
{
	pthread_mutex_destroy(&m_Mutex);
}

void CThreadLock::Lock()
{
	pthread_mutex_lock(&m_Mutex);
}

void CThreadLock::UnLock()
{
	pthread_mutex_unlock(&m_Mutex);
}
