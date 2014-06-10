#ifndef _RINGBUFFER_H_
#define _RINGBUFFER_H_
#include <string>

class CRingBuffer
{
public:
    CRingBuffer()
    : buf(NULL)
    , bufsize(0)
    , h(0)
    , r(0)
    {
    }

    ~CRingBuffer()
    {
        release();
    }

    void release()
    {
        if (buf)
        {
            delete buf;
            buf = NULL;
        }
        bufsize = 0;
        h = 0;
        r = 0;
    }

    void reset()
    {
        h = 0;
        r = 0;
        memset(buf, 0, bufsize);
    }

    void init(int bfsz)
    {
        release();
        buf = new unsigned char[bfsz];
        if (buf)
        {
            bufsize = bfsz;
            memset(buf, 0, bufsize);
        }
    }

    bool isfull()
    {
        return ((r + 1) % bufsize == h);
    }

    bool isempty()
    {
        return (r == h);
    }

    unsigned int getfree()
    {
        return bufsize - getsize() - 1;
    }

    unsigned int getsize()
    {
        return ((r + bufsize - h) % bufsize);
    }

    bool put(const unsigned char* p, const unsigned int len)
    {
        if (getfree() >= len)
        {
            if ( (bufsize - r) >= len)
            {
                memcpy(&buf[r], p, len);
                r += len;
            }
            else
            {
                memcpy(&buf[r], p, bufsize - r);
                memcpy(buf, static_cast<const unsigned char*>(p + (bufsize - r)), len - (bufsize - r));
                r = len - (bufsize - r);
            }
            return true;
        }
        else
        {
            return false;
        }
    }

    bool get(const unsigned int len, unsigned char* dst)
    {
        if (len == 0)
            return false;

        if (len <= getsize())
        {
            unsigned int org = h;
            if (bufsize - h >= len)
            {
                h = (h + len) % bufsize;
                memcpy(dst, &buf[org], len);
                return true;
            }
            else
            {
                h = len - (bufsize - h);
                memcpy(dst, &buf[org], bufsize - org);
                memcpy(dst + bufsize - org, buf, h);
                return true;
            }
        }
        else
        {
            return false;
        }
    }

    unsigned int total()
    {
        return bufsize;
    }

private:
    unsigned char* buf;
    unsigned int bufsize;
    unsigned int h;
    unsigned int r;
};

#endif//_RINGBUFFER_H_