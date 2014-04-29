#ifndef _PROTOCOL_H_
#define _PROTOCOL_H_

#include "globaldef.h"
#include "eupustream.h"

struct MessageHead : CEupuStream
{
    public:
        bool In(BYTE* src, UINT& len)
        {
            return false;
        }

        bool Out(BYTE* dst, UINT& len)
        {
            return false;
        }
};

#endif//_PROTOCOL_H_
