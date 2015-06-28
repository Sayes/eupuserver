#include  <stdio.h>
#include  <stdlib.h>
#include "utility.h"

const unsigned char Utility::base64_decode_map[256] =
{
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,

    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,

    255, 255, 255, 62, 255, 255, 255, 63, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 255, 255,

    255, 0, 255, 255, 255, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14,

    15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 255, 255, 255, 255, 255, 255, 26, 27, 28,

    29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48,

    49, 50, 51, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,

    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,

    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,

    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,

    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,

    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,

    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255
};

Utility::Utility()
{
    //ctor
}

Utility::~Utility()
{
    //dtor
}

string Utility::Int2String(int num)
{
    char tmp[100];
    memset(tmp, 0, sizeof(tmp));
    snprintf(tmp, sizeof(tmp), "%d", num);
    return string(tmp);
}

string Utility::uInt2String(uint32 num)
{
    char tmp[100];
    memset(tmp, 0, sizeof(tmp));
    snprintf(tmp, sizeof(tmp), "%u", num);
    return string(tmp);
}


//uint32 Utility::String2UL(const char *str)
//{
//    if (str == NULL)
//    {
//        return 0;
//    }
//
//    return strtoul(str, NULL, 10);
//}

//long Utility::String2L(const char* str)
//{
//    if (str == NULL)
//    {
//        return 0;
//    }
//
//    return strtol(str, NULL, 10);
//}

bool Utility::Compress(uint8* pSrc, int nLen, uint8* pDst, int& nDstLen)
{
    //int nCompleteLen = compress2(pDst,(uLongf*)&nDstLen,pSrc,nLen,5);
    return true;
}

bool Utility::Uncompress(uint8* pSrc, int nLen, uint8* pDst, int& nDstLen)
{
    return true;
}

/*
DWORD Utility::MakeDWORD(uint8 bValue1, uint8 bValue2, uint8 bValue3, uint8 bValue4)
{
	DWORD dwValue = 0;
	dwValue |= (bValue1 << 24);
	dwValue |= (bValue2 << 16);
	dwValue |= (bValue3 << 8);
	dwValue |= (bValue4);

	return dwValue;
}
*/

uint32 Utility::GetCPUComsuming()
{
    return 0;
}

uint32 Utility::GetMemComsuming()
{
    return 0;
}

/*
void Utility::MakeBYTE(DWORD dwValue, uint8& bValue1, uint8& bValue2, uint8& bValue3, uint8& bValue4)
{
	bValue1 = ((dwValue & 0xff000000)>>24);
	bValue2 = ((dwValue & 0x00ff0000)>>16);
	bValue3 = ((dwValue & 0x0000ff00)>>8);
	bValue4 = (dwValue & 0x000000ff);
}
*/

/*
string Utility::Ip2String(uint32 ip)
{
	uint8 value1, value2, value3, value4;
	MakeBYTE(ip, value1, value2, value3, value4);

	string strIp = (Int2String(value1, 10)+"."+Int2String(value2, 10)+"." +Int2String(value3, 10)+"."+Int2String(value4, 10));

	return strIp;
}
*/

/*
uint32 Utility::Ip2uInt(char* ip)
{
	char* p = strtok(ip, ".");
	uint8 value[4];
	int i=0;
	while (p != NULL)
	{
		value[i] = (uint8)atoi(p);
		p = strtok(NULL,".");
		i++;
	}

	return MakeDWORD(value[0], value[1], value[2], value[3]);
}
*/

/*
uint32 Utility::Ip2S_addr(char* ip)
{
	char* p = strtok(ip, ".");
	uint8 value[4];
	int i=0;
	while (p != NULL)
	{
		value[i] = (uint8)atoi(p);
		p = strtok(NULL,".");
		i++;
	}

	return MakeDWORD(value[3], value[2], value[1], value[0]);
}
*/

uint32 Utility::GetMillionTime()
{
    return (uint32)time(0);
}

void Utility::GetTime4Log(char* szTime)
{
#ifdef OS_LINUX
    if (szTime == NULL)
    {
        return;
    }
    time_t timep;
    struct tm ptm = {0};
    time(&timep);
    localtime_r(&timep, &ptm);
    sprintf(szTime, "%.2d%.2d%.2d", ptm.tm_hour, ptm.tm_min, ptm.tm_sec);
#elif OS_WINDOWS
    if (szTime == NULL)
    {
        return;
    }
    time_t timep;
    struct tm* p = NULL;
    time(&timep);
    p = localtime(&timep);

    if (p)
        sprintf(szTime, "%.2d%.2d%.2d", p->tm_hour, p->tm_min, p->tm_sec);
#endif
}

void Utility::GetDate4Log(char* szDate)
{
#ifdef OS_LINUX
    if (szDate == NULL)
    {
        return;
    }

    time_t timep;
    struct tm ptm = {0};
    time(&timep);
    localtime_r(&timep, &ptm);

    sprintf(szDate, "%d%.2d%.2d", (1900 + ptm.tm_year), (1 + ptm.tm_mon), ptm.tm_mday);
#elif OS_WINDOWS
    if (szDate == NULL)
    {
        return;
    }

    time_t timep;
    struct tm* p = NULL;
    time(&timep);
    p = localtime(&timep);

    if (p)
        sprintf(szDate, "%d%.2d%.2d", (1900 + p->tm_year), (1 + p->tm_mon), p->tm_mday);

#endif
}

void Utility::GetDate(char* szDate)
{
    if (szDate == NULL)
    {
        return;
    }

#ifdef OS_LINUX
    time_t timep;
    struct tm p = {0};
    time(&timep);
    localtime_r(&timep, &p);

    sprintf(szDate, "%d-%.2d-%.2d ", (1900 + p.tm_year), (1 + p.tm_mon), p.tm_mday);
#elif OS_WINDOWS
    time_t timep;
    struct tm* p = NULL;
    time(&timep);
    p = localtime(&timep);

    if (p)
        sprintf(szDate, "%d-%.2d-%.2d ", (1900 + p->tm_year), (1 + p->tm_mon), p->tm_mday);

#endif
}

void Utility::GetDateTime4Log(char* szDateTime)
{
    if (szDateTime == NULL)
    {
        return;
    }
#ifdef OS_LINUX
    time_t timep;
    struct tm p = {0};
    time(&timep);
    localtime_r(&timep, &p);

    sprintf(szDateTime, "%d%.2d%.2d%.2d%.2d%.2d", (1900 + p.tm_year), (1 + p.tm_mon), p.tm_mday, p.tm_hour, p.tm_min,
            p.tm_sec);
#elif OS_WINDOWS
    time_t timep;
    struct tm* p = NULL;
    time(&timep);
    p = localtime(&timep);

    if (p)
        sprintf(szDateTime, "%d%.2d%.2d%.2d%.2d%.2d", (1900 + p->tm_year), (1 + p->tm_mon), p->tm_mday, p->tm_hour, p->tm_min, p->tm_sec);
#endif
}

void Utility::GetDateTime(char* szDateTime)
{
    if (szDateTime == NULL)
    {
        return;
    }
#ifdef OS_LINUX
    time_t timep;
    struct tm p = {0};
    time(&timep);
    localtime_r(&timep, &p);

    sprintf(szDateTime, "%d-%.2d-%.2d %.2d:%.2d:%.2d", (1900 + p.tm_year), (1 + p.tm_mon), p.tm_mday, p.tm_hour,
            p.tm_min, p.tm_sec);
#elif OS_WINDOWS
    time_t timep;
    struct tm* p = NULL;
    time(&timep);
    p = localtime(&timep);

    if (p)
        sprintf(szDateTime, "%d-%.2d-%.2d %.2d:%.2d:%.2d", (1900 + p->tm_year), (1 + p->tm_mon), p->tm_mday, p->tm_hour, p->tm_min, p->tm_sec);
#endif
}
time_t Utility::FormatTime(char* szTime)
{
    if (!(szTime && szTime[0]))
    {
        //time_t tempTime;
        //time(&tempTime);
        return 0;
    }

    struct tm tm1;
    time_t time1;

    int nRet = sscanf(szTime, "%4d-%2d-%2d %2d:%2d:%2d",

                      &tm1.tm_year,

                      &tm1.tm_mon,

                      &tm1.tm_mday,

                      &tm1.tm_hour,

                      &tm1.tm_min,

                      &tm1.tm_sec);

    if (6 != nRet)
    {
        //time_t tempTime;
        //time(&tempTime);
        return 0;
    }

    tm1.tm_year -= 1900;
    tm1.tm_mon --;
    tm1.tm_isdst = -1;

    time1 = mktime(&tm1);

    return time1;


}
uint32 Utility::DiffTime(time_t tTime)
{
    time_t currentTime;
    time(&currentTime);
    double cost = difftime(currentTime, tTime);
    uint32 uCost = uint32(cost);

    return uCost;
}
//LONG Utility::GetFileLines(char* szFileName)
//{
//    FILE* fp;
//
//    fp = fopen(szFileName, "r"); // "r+t" );
//
//    if (fp == NULL)
//    {
//        return -1;
//    }
//
//    char str[ 4096 ];
//    memset(str, 0, sizeof(str));
//    LONG cnt = 0;
//
//    while (fgets(str, 4096, fp) != NULL)
//    {
//        cnt++;
//        memset(str, 0, sizeof(str));
//    }
//
//    fclose(fp);
//
//    return cnt;
//
//}

bool Utility::CheckDate(int32 year, int32 month, int32 day)
{
    if (year < 1900 || month <= 0 || month > 12 || day <= 0 || day > 31)
    {
        return false;
    }

#ifdef OS_LINUX
    //form time
    struct tm tm_new;
    tm_new.tm_year = year - 1900;
    tm_new.tm_mon = month - 1;
    tm_new.tm_mday = day;
    tm_new.tm_hour = 0;
    tm_new.tm_min = 0;
    tm_new.tm_sec = 0;
    time_t time_new = mktime(&tm_new);

    localtime_r(&time_new, &tm_new);

    if (tm_new.tm_year != (year - 1900) || tm_new.tm_mon != (month - 1) || tm_new.tm_mday != day)
    {
        return false;
    }
    else
    {
        return true;
    }

#elif OS_WINDOWS

    //form time
    struct tm tm_new;
    tm_new.tm_year = year - 1900;
    tm_new.tm_mon = month - 1;
    tm_new.tm_mday = day;
    tm_new.tm_hour = 0;
    tm_new.tm_min = 0;
    tm_new.tm_sec = 0;
    time_t time_new = mktime(&tm_new);
    struct tm* p = localtime(&time_new);

    if (!p)
        return false;

    if (p->tm_year != (year - 1900) || p->tm_mon != (month - 1) || p->tm_mday != day)
    {
        return false;
    }
    else
    {
        return true;
    }
#endif
}

/*
LONG Utility::GetFileSize(char* strFileName)
{
	if (strFileName == NULL)
	{
		return -1;
	}
	struct stat f_stat;
	if (stat(strFileName, &f_stat) == -1)
	{
		return -1;
	}

	return (long)f_stat.st_size;
}
*/

int Utility::Base64Decode(unsigned char* szSrc, unsigned char* szDst, int SrcSize)
{
    int i = 0, j = 0;

    int count = 0;

    int nLen = strlen((char*)szSrc);
    int nFinal = 0;
    for (i = nLen - 1; i >= 0; i--)
    {
        if (szSrc[i] == '=')
        {
            nFinal++;
        }
        else
        {
            break;
        }
    }

    for (i = 0; i < SrcSize; i += 4)
    {

        szDst[j++] = base64_decode_map[szSrc[i]] << 2 | base64_decode_map[szSrc[i + 1]] >> 4;

        szDst[j++] = base64_decode_map[szSrc[i + 1]] << 4 | base64_decode_map[szSrc[i + 2]] >> 2;

        szDst[j++] = base64_decode_map[szSrc[i + 2]] << 6 | base64_decode_map[szSrc[i + 3]];

        count += 3;
    }

    count -= nFinal;
    return count;
}

int Utility::getIntDotNet(unsigned char* bb, int index)
{
    return (int)((((bb[index + 3] & 0xff) << 24) | ((bb[index + 2] & 0xff) << 16) | ((bb[index + 1] & 0xff) << 8)
                  | ((bb[index + 0] & 0xff) << 0)));
}

int64 Utility::getLongDotNet(unsigned char* bb, int index)
{
    return ((((int64) bb[index + 7] & 0xff) << 56) | (((int64) bb[index + 6] & 0xff) << 48) | (((int64) bb[index + 5]
            & 0xff) << 40) | (((int64) bb[index + 4] & 0xff) << 32) | (((int64) bb[index + 3] & 0xff) << 24)
            | (((int64) bb[index + 2] & 0xff) << 16) | (((int64) bb[index + 1] & 0xff) << 8) | (((int64) bb[index + 0]
                    & 0xff) << 0));
}

short Utility::getShortDotNet(uint8* b, int index)
{
    return (short)(((b[index + 1] << 8) | (b[index] & 0xff)));
}

string& Utility::ReplaceAll(string& str, const char* old_value, const char* new_value)
{
    string::size_type pos = str.find(old_value);

    if (pos == string::npos)
    {
        return str;
    }

    do
    {
        str.replace(pos, strlen(old_value), new_value);
        pos = str.find(old_value, pos + strlen(new_value));
    }
    while (pos != string::npos);

    return str;
}

uint32 Utility::str2Time(string strTm)
{
    time_t uTime = 0;
    int index = strTm.find(" ");
    string strTmp1 = strTm.substr(0, index);
    string strTmp2 = strTm.substr(index + 1, strTm.length() - index - 1);

    index = strTmp1.find("-");
    string strYear = strTmp1.substr(0, index);
    strTmp1 = strTmp1.substr(index + 1, strTmp1.length() - index - 1);

    index = strTmp1.find("-");
    string strMonth = strTmp1.substr(0, index);
    strTmp1 = strTmp1.substr(index + 1, strTmp1.length() - index - 1);

    index = strTmp1.find("-");
    string strDate = strTmp1.substr(0, index);

    index = strTmp2.find(":");
    string Hour = strTmp2.substr(0, index);
    strTmp2 = strTmp2.substr(index + 1, strTmp2.length() - index - 1);

    index = strTmp2.find(":");
    string Minute = strTmp2.substr(0, index);
    strTmp2 = strTmp2.substr(index + 1, strTmp2.length() - index - 1);

    index = strTmp2.find(":");
    string Second = strTmp2.substr(0, index);

    struct tm tm1;
    //uint64 uNowTime = Utility::GetMillionTime();
    //struct tm *tmUTC = gmtime((time_t*)&uNowTime);

    tm1.tm_year = atoi(strYear.c_str()) - 1900;
    tm1.tm_mon = atoi(strMonth.c_str()) - 1;
    tm1.tm_mday = atoi(strDate.c_str());

    tm1.tm_hour = atoi(Hour.c_str());
    tm1.tm_min = atoi(Minute.c_str());
    tm1.tm_sec = atoi(Second.c_str());
#ifdef OS_LINUX
    tm1.tm_gmtoff = 0;//tmUTC->tm_gmtoff;
#endif
    tm1.tm_isdst = 0;//tmUTC->tm_isdst;
    //tm1.tm_zone = new char[ strlen(tmUTC.tm_zone) + 1];
    //strcpy((char*)tm1.tm_zone,(const char*)tmUTC->tm_zone);

    uTime = mktime(&tm1);

    //tmUTC = localtime(&uTime);
    return uTime;
}

uint64 Utility::GenerateKeyByIPAndPort(uint32 uIp, uint32 uPort)
{
    uint64 uKey = (((uint64)uIp & 0xffffffff) << 16) | uPort;

    return uKey;
}

int Utility::abs(int a)
{
    if (a < 0)
    {
        return -a;
    }

    return a;
}
