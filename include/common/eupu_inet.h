// Copyright shenyizhong@gmail.com, 2014

#ifndef EUPU_INET_H_
#define EUPU_INET_H_

#ifdef __cplusplus
extern "C" {
#endif

const char *inet_ntop4(const unsigned char *src, char *dst, size_t size);
#ifdef AF_INET6
const char *inet_ntop6(const unsigned char *src, char *dst, size_t size);
#endif

int inet_pton4(const char *src, unsigned char *dst);
#ifdef AF_INET6
int inet_pton6(const char *src, unsigned char *dst);
#endif

const char *eupu_inet_ntop(int af, const void *src, char *dst, size_t size);
int eupu_inet_pton(int af, const char *src, void *dst);

#ifdef __cplusplus
}
#endif

#endif //  EUPU_INET_H_
