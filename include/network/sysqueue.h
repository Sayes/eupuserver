#ifndef __SYSQUEUE_H__
#define __SYSQUEUE_H__

#include <list>
#include "threadlock.h"

using std::list;

template <class T>
class SysQueue {
public:
    void clearQueue()
    {
        Lock();
        typename list<T*>::iterator iter;
        for (iter = m_datalst.begin(); iter != m_datalst.end(); ++iter)
        {
            if (*iter)
            {
                delete *iter;
                *iter = NULL;
            }
        }
        m_datalst.clear();
        UnLock();
    }

    virtual ~SysQueue()
    {

    }

    SysQueue()
    {
        m_maxSize = 15000;
    }

    void setQueueSize(int nsize)
    {
        m_maxSize = nsize;
    }

    int size()
    {
        int sum = 0;
        m_lock.Lock();
        sum = m_datalst.size();
        m_lock.UnLock();
        return sum;
    }

    int sizeWithoutLock()
    {
        return m_datalst.size();
    }

    bool isEmpty()
    {
        bool isEmpty = true;
        m_lock.Lock();
        isEmpty = m_datalst.empty();
        m_lock.UnLock();
        return isEmpty;
    }

    bool isEmptyWithoutLock()
    {
        return m_datalst.empty();
    }

    bool inQueue(T* arg, bool bhead)
    {
        bool bret = false;

        m_lock.Lock();
        do
        {
            if (!arg)
            {
                break;
            }

            if (m_datalst.size() >= (UINT)m_maxSize)
            {
                break;
            }

            if (bhead)
            {
                m_datalst.push_front(arg);
            }
            else
            {
                m_datalst.push_back(arg);
            }
            bret = true;
        }
        while (false);
        m_lock.UnLock();
        return bret;

    }

    bool outQueue(T*& arg, bool bhead)
    {
        bool bret = false;
        m_lock.Lock();

        do
        {
            if (m_datalst.size() <= 0)
            {
                break;
            }

            if (bhead)
            {
                arg = m_datalst.front();
                m_datalst.pop_front();
            }
            else
            {
                arg = m_datalst.back();
                m_datalst.pop_back();
            }
            bret = true;
        }
        while (false);

        m_lock.UnLock();

        return bret;
    }

    bool inQueueWithoutLock(T* arg, bool bhead)
    {
        if (!arg)
            return false;

        if (m_datalst.size() >= (UINT)m_maxSize)
            return false;

        if (bhead)
        {
            m_datalst.push_front(arg);
        }
        else
        {
            m_datalst.push_back(arg);
        }
        return true;
    }

    bool outQueueWithoutLock(T*& arg, bool bhead)
    {
        if (m_datalst.size() <= 0)
            return false;
        if (bhead)
        {
            arg = m_datalst.front();
            m_datalst.pop_front();
        }
        else
        {
            arg = m_datalst.back();
            m_datalst.pop_back();
        }
        return true;
    }

    void Lock()
    {
        m_lock.Lock();
    }

    void UnLock()
    {
        m_lock.UnLock();
    }

protected:
    CThreadLock m_lock;
    list<T*> m_datalst;
    int m_maxSize;
};

#endif//__SYSQUEUE_H__
