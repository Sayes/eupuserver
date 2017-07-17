// Copyright shenyizhong@gmail.com, 2014

#ifndef NETWORK_COUNTER_H_
#define NETWORK_COUNTER_H_

class CCounter {
   public:
    CCounter() { m_count = 0; }

    ~CCounter() {}

    void Lock() { m_lock.Lock(); }

    void UnLock() { m_lock.UnLock(); }

    void setvalue(int nvalue) {
        m_lock.Lock();
        m_count = nvalue;
        if (m_count < 0) m_count = 0;
        m_lock.UnLock();
    }

    int resetWithoutLock() {
        int nret = 0;
        nret = m_count;
        m_count = 0;
        return nret;
    }

    int value() {
        int nret = 0;
        m_lock.Lock();
        nret = m_count;
        m_lock.UnLock();
        return nret;
    }

    int valueWithoutLock() { return m_count; }

    int increase(int n) {
        int nret = 0;
        m_lock.Lock();
        m_count += n;
        if (m_count < 0) m_count = 0;
        nret = m_count;
        m_lock.UnLock();
        return nret;
    }

    int decrease(int n) {
        int nret = 0;
        m_lock.Lock();
        m_count -= n;
        if (m_count < 0) m_count = 0;
        nret = m_count;
        m_lock.UnLock();
        return nret;
    }

   private:
    CThreadLock m_lock;
    uint32 m_count;
};

#endif  // NETWORK_COUNTER_H_
