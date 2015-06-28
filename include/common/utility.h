#ifndef _UTILITY_H_
#define _UTILITY_H_

#include <time.h>
#include "globaldef.h"

typedef struct tm tm;
class Utility {
public:
    static string Int2String(int num);
    static string uInt2String(uint32 num);
    //static string Char2String(char* str);
    //static uint32 String2UL(const char *str);
    //static long String2L(const char* str);
    static bool Compress(uint8* pSrc, int nLen, uint8* pDst, int& nDstLen);
    static bool Uncompress(uint8* pSrc, int nLen, uint8* pDst, int& nDstLen);
    //static DWORD MakeDWORD(uint8 bValue1, uint8 bValue2, uint8 bValue3, uint8 bValue4);
    //static void MakeBYTE(DWORD dwValue, uint8& bValue1, uint8& bValue2, uint8& bValue3, uint8& bValue4);

    //static string Ip2String(uint32 ip);
    //static uint32 Ip2uInt(char* ip);
    //static uint32 Ip2S_addr(char* ip);

    static uint32 GetMillionTime();
    static void GetTime4Log(char* szTime);
    static void GetDate4Log(char* szDate);
    static void GetDate(char* szDate);
    static void GetDateTime4Log(char* szDateTime);
    static void GetDateTime(char* szDateTime);

    //static LONG GetFileLines(char* szFileName);

    static bool CheckDate(int32 year, int32 month, int32 day);

    //static LONG GetFileSize(char* strFileName);

    static uint32 GetCPUComsuming();
    static uint32 GetMemComsuming();
    static int Base64Decode(unsigned char* szSrc, unsigned char* szDst, int SrcSize);
    static int getIntDotNet(unsigned char* bb, int index);
    static int64 getLongDotNet(unsigned char* bb, int index);
    static int16 getShortDotNet(uint8* b, int index);

    static string& ReplaceAll(string& str, const char* old_value, const char* new_value);

    static void Int2Bytes(int nRaw, unsigned char* pBuf)
    {
        *(pBuf) = (nRaw & 0xff000000) >> 24;
        *(pBuf + 1) = (nRaw & 0x00ff0000) >> 16;
        *(pBuf + 2) = (nRaw & 0x0000ff00) >> 8;
        *(pBuf + 3) = (nRaw & 0x000000ff);

    }
    static int Bytes2Int(unsigned char* pBuf)
    {
        int mask = 0xff;
        int temp = 0;
        int res = 0;
        for (int i = 0; i < 4; i++)
        {
            res <<= 8;
            temp = pBuf[i] & mask;
            res |= temp;
        }
        return res;
    }

    static uint32 str2Time(string strTm);
    static uint64 GenerateKeyByIPAndPort(uint32 uIp, uint32 uPort);
    static time_t FormatTime(char* szTime);
    static uint32 DiffTime(time_t tTime);
    static int abs(int a);

protected:
    Utility();
    virtual ~Utility();

private:
    static const unsigned char base64_decode_map[256];
public:
    //static time_t timep;
    //static struct tm* p;
};

#endif // _UTILITY_H_
