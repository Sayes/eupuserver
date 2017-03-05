// Copyright shenyizhong@gmail.com, 2014

#ifndef NETWORK_NETCOMMON_H_
#define NETWORK_NETCOMMON_H_

#include <time.h>
#include <string>

#include "common/globalconfig.h"
#include "common/globaldef.h"

#define CLOSE_CLIENT 100
#define ADD_CLIENT 101

#define LISTEN_TYPE 0
#define CLIENT_TYPE 1
#define MAINSVR_TYPE 2
#define DISSVR_TYPE 3
#define USERCENTERSVR_TYPE 4
#define CENTER_TYPE 5
#define LOGSVR_TYPE 6

typedef struct socket_key {
    int fdkey;
    time_t connect_time;
    socket_key() {
        fdkey = -1;
        connect_time = 0;
    }
} SOCKET_KEY;

typedef struct socket_set {
    SOCKET_KEY *key;
    char part_buf[MAX_SEND_SIZE];
    int part_len;

    std::string peer_ip;
    uint16_t peer_port;

    time_t refresh_time;
    int type;  // 0: listen, 1: client, 2: mainsvr, 3: dissvr, 4: usercentersvr,
               // 5:
               // centersvr, 6: logsvr

    bool init(SOCKET_KEY *pkey, const std::string &ip, uint16_t port,
              int ntype) {
        if (!pkey) return false;

        key = pkey;
        peer_ip = ip;
        peer_port = port;
        type = ntype;
        refresh_time = time(NULL);
        return true;
    }

    socket_set() {
        key = NULL;
        part_len = 0;
        peer_port = 0;
        type = 1;
        refresh_time = 0;
        memset(part_buf, 0, sizeof(part_buf));
    }

    ~socket_set() {
        if (key != NULL) {
            delete key;
            key = NULL;
        }
    }
} SOCKET_SET;

typedef struct net_data {
    int fddat;
    time_t connect_time;
    std::string peer_ip;
    uint16_t peer_port;
    int32_t type;
    char *pdata;
    uint32_t data_len;

    net_data() {
        fddat = -1;
        connect_time = 0;
        peer_port = 0;
        type = 0;
        pdata = NULL;
        data_len = 0;
    }

    ~net_data() {
        if (pdata != NULL) {
            delete[] pdata;
            pdata = NULL;
        }
    }

    bool init(int _fd, time_t conntime, const std::string &ip, uint16_t port,
              int ntype, uint32_t nlen) {
        fddat = _fd;
        connect_time = conntime;
        peer_ip = ip;
        peer_port = port;
        type = ntype;
        data_len = nlen;

        if (nlen > 0) {
            pdata = new char[nlen];
            if (!pdata) {
                return false;
            }
            memset(pdata, 0, nlen);
        }
        return true;
    }
} NET_DATA;

typedef struct net_event {
    int eventid;
    char *data;
    net_event() {
        eventid = 0;
        data = NULL;
    }
} NET_EVENT;

SOCKET_SET *initSocketset(int fd, time_t conntime, const std::string &peerip,
                          uint16_t peerport, int ntype);
bool setNonBlock(int fd);

int recv_msg(int fd, char *buf, int &nlen);
int send_msg(int fd, char *buf, int &nlen);

int doNonblockConnect(PCONNECT_SERVER pServer, int timeout = 3,
                      const std::string &localip = "");

#endif  //NETWORK_NETCOMMON_H_
