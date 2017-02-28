// Copyright shenyizhong@gmail.com, 2014

#ifndef UTILITY_H_
#define UTILITY_H_

#include "common/globaldef.h"
#include <time.h>

typedef struct tm tm;
class Utility {
public:
  static std::string Int2String(int32_t num);
  static std::string uInt2String(uint32_t num);
  // static std::string Char2String(char* str);
  // static uint32_t String2UL(const char *str);
  // static long String2L(const char* str);
  static bool Compress(uint8_t *pSrc, int32_t nLen, uint8_t *pDst,
                       int32_t &nDstLen);
  static bool Uncompress(uint8_t *pSrc, int32_t nLen, uint8_t *pDst,
                         int32_t &nDstLen);
  // static DWORD MakeDWORD(uint8_t bValue1, uint8_t bValue2, uint8_t bValue3,
  // uint8_t bValue4);
  // static void MakeBYTE(DWORD dwValue, uint8_t& bValue1, uint8_t& bValue2,
  // uint8_t& bValue3, uint8_t& bValue4);

  // static std::string Ip2String(uint32_t ip);
  // static uint32_t Ip2uInt(char* ip);
  // static uint32_t Ip2S_addr(char* ip);

  static uint32_t GetMillionTime();
  static void GetTime4Log(char *szTime);
  static void GetDate4Log(char *szDate);
  static void GetDate(char *szDate);
  static void GetDateTime4Log(char *szDateTime);
  static void GetDateTime(char *szDateTime);

  // static LONG GetFileLines(char* szFileName);

  static bool CheckDate(int32_t year, int32_t month, int32_t day);

  // static LONG GetFileSize(char* strFileName);

  static uint32_t GetCPUComsuming();
  static uint32_t GetMemComsuming();
  static int32_t Base64Decode(unsigned char *szSrc, unsigned char *szDst,
                              int32_t SrcSize);
  static int32_t getIntDotNet(unsigned char *bb, int32_t index);
  static int64_t getLongDotNet(unsigned char *bb, int32_t index);
  static int16_t getShortDotNet(uint8_t *b, int32_t index);

  static std::string &ReplaceAll(std::string &str, const char *old_value,
                                 const char *new_value);

  static void Int2Bytes(int32_t nRaw, unsigned char *pBuf) {
    *(pBuf) = (nRaw & 0xff000000) >> 24;
    *(pBuf + 1) = (nRaw & 0x00ff0000) >> 16;
    *(pBuf + 2) = (nRaw & 0x0000ff00) >> 8;
    *(pBuf + 3) = (nRaw & 0x000000ff);
  }
  static int32_t Bytes2Int(unsigned char *pBuf) {
    int32_t mask = 0xff;
    int32_t temp = 0;
    int32_t res = 0;
    for (int32_t i = 0; i < 4; i++) {
      res <<= 8;
      temp = pBuf[i] & mask;
      res |= temp;
    }
    return res;
  }

  static uint32_t str2Time(std::string strTm);
  static uint64_t GenerateKeyByIPAndPort(uint32_t uIp, uint32_t uPort);
  static time_t FormatTime(char *szTime);
  static uint32_t DiffTime(time_t tTime);
  static int32_t abs(int32_t a);

protected:
  Utility();
  virtual ~Utility();

private:
  static const unsigned char base64_decode_map[256];

public:
  // static time_t timep;
  // static struct tm* p;
};

#endif // UTILITY_H_
