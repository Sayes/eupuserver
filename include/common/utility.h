#ifndef _UTILITY_H_
#define _UTILITY_H_

#include "globaldef.h"
#include  <sys/times.h>

typedef struct tm tm;
class cUtility
{
public:
    static long GetTickCount();
    static string Int2String(int num);
    static string uInt2String(UINT num);
    //static string Char2String(char* str);
    static ulong String2UL(const char *str);
    static long String2L(const char* str);
    static bool Compress(BYTE* pSrc, int nLen, BYTE* pDst, int&nDstLen);
    static bool Uncompress(BYTE* pSrc, int nLen, BYTE* pDst, int&nDstLen);
    //static DWORD MakeDWORD(BYTE bValue1, BYTE bValue2, BYTE bValue3, BYTE bValue4);
    //static void MakeBYTE(DWORD dwValue, BYTE& bValue1, BYTE& bValue2, BYTE& bValue3, BYTE& bValue4);

    //static string Ip2String(UINT ip);
    //static UINT Ip2uInt(char* ip);
    //static UINT Ip2S_addr(char* ip);

    static UINT GetMillionTime();
    static void GetTime4Log(char* szTime);
    static void GetDate4Log(char* szDate);
    static void GetDate(char* szDate);
    static void GetDateTime4Log(char* szDateTime);
    static void GetDateTime(char* szDateTime);

    static LONG GetFileLines(char* szFileName);

    static bool CheckDate(INT32 year, INT32 month, INT32 day);

    static LONG GetFileSize(char* strFileName);

    static UINT GetCPUComsuming();
    static UINT GetMemComsuming();
    static int Base64Decode(unsigned char *szSrc, unsigned char* szDst, int SrcSize);
    static int getIntDotNet(unsigned char* bb, int index);
    static INT64 getLongDotNet(unsigned char* bb, int index);
    static short getShortDotNet(BYTE* b, int index);

    static string& ReplaceAll(string& str, const char* old_value, const char* new_value);

    static void Int2Bytes(int nRaw, unsigned char *pBuf)
    {
        *(pBuf) = (nRaw & 0xff000000) >> 24;
        *(pBuf + 1) = (nRaw & 0x00ff0000) >> 16;
        *(pBuf + 2) = (nRaw & 0x0000ff00) >> 8;
        *(pBuf + 3) = (nRaw & 0x000000ff);

    }
    static int Bytes2Int(unsigned char *pBuf)
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

    static UINT str2Time(string strTm);
    static UINT64 GenerateKeyByIPAndPort(UINT uIp, UINT uPort);
    static time_t FormatTime(char * szTime);
    ulong static DiffTime(time_t tTime);//与当前时间计算时间差 
    static int abs(int a);

protected:
    cUtility();
    virtual ~cUtility();

private:
    static const char base64_decode_map[256];
public:
    //static time_t timep;
    //static struct tm* p;
};

#endif // _UTILITY_H_
