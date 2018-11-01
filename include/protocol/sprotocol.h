// Copyright shenyizhong@gmail.com, 2014

#ifndef PROTOCOL_SPROTOCOL_H_
#define PROTOCOL_SPROTOCOL_H_

#include "common/globaldef.h"
#include "protocol/eupustream.h"
#include "protocol/protocol.h"

#define RC_CONNECTED 1
#define RC_DISCONNECTED 2

#define RS_CONNECTED 11
#define RS_DISCONNECTED 12

#define KEEP_ALIVE_PING 20
#define C2S_DATA 30

struct MP_Server_Connected : public CEupuStream {
  MP_Server_Connected() {
    msgHead.uMainID = RS_CONNECTED;
    msgHead.uMessageSize = NET_HEAD_SIZE + sizeof(int);
    m_nServer = 0;
  }

  NetMessageHead msgHead;
  int m_nServer;

  void Debug() {
    LOG(_DEBUG_, "MP_Server_Connected::Debug() object members:");
    msgHead.Debug();
    LOG(_DEBUG_, "\tm_nServer: %d", m_nServer);
  }

  bool Out(BYTE *pDest, uint32_t &nlen) {
    int32_t nret = 0;
    uint32_t ntmp = 0;
    uint32_t nbuffer = nlen;

    if (msgHead.Out(pDest, nbuffer)) {
      ntmp = nbuffer;
      nret = OutputValue(pDest + ntmp, nlen - ntmp, m_nServer);

      if (nret < 0) {
        return false;
      }

      ntmp += nret;
      nlen = ntmp;

      return true;
    }
    return false;
  }

  bool In(BYTE *pSrc, uint32_t &nlen) {
    int32_t nret = 0;
    uint32_t ntmp = 0;
    uint32_t nbuflen = nlen;

    if (msgHead.In(pSrc, nbuflen)) {
      ntmp = nbuflen;
      nret = InputValue(pSrc + ntmp, nlen - ntmp, m_nServer);
      if (nret < 0) {
        return false;
      }
      ntmp += nret;
      nlen = ntmp;

      return true;
    }
    return false;
  }
};

struct MP_Server_DisConnected : public CEupuStream {
  MP_Server_DisConnected() {
    msgHead.uMainID = RS_DISCONNECTED;
    msgHead.uMessageSize = NET_HEAD_SIZE + sizeof(int);
    m_nServer = 0;
  }

  NetMessageHead msgHead;
  int m_nServer;

  void Debug() {
    LOG(_DEBUG_, "struct MP_Server_DisConnected object members:");
    msgHead.Debug();
    LOG(_DEBUG_, "\tm_nServer: %d", m_nServer);
  }

  bool Out(BYTE *pDest, uint32_t &nlen) {
    int32_t nret = 0;
    uint32_t ntmp = 0;
    uint32_t nbuflen = nlen;

    if (msgHead.Out(pDest, nbuflen)) {
      ntmp = nbuflen;
      nret = OutputValue(pDest + ntmp, nlen - ntmp, m_nServer);

      if (nret < 0) {
        return false;
      }

      ntmp += nret;
      nlen = ntmp;

      return true;
    }
    return false;
  }

  bool In(BYTE *pSrc, uint32_t &nlen) {
    int32_t nret = 0;
    uint32_t ntmp = 0;
    uint32_t nbuflen = nlen;

    if (msgHead.In(pSrc, nbuflen)) {
      ntmp = nbuflen;
      nret = InputValue(pSrc + ntmp, nlen - ntmp, m_nServer);
      if (nret < 0) {
        return false;
      }

      ntmp += nret;

      nlen = ntmp;

      return true;
    }
    return false;
  }
};

#endif  // PROTOCOL_SPROTOCOL_H_
