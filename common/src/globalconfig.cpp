#include "common/globalconfig.h"
#include "common/globaldef.h"
#include "logger/eupulogger4system.h"
#include "json/json.h"
#include <fstream>

#define CONFIGURATION ""
#define SYSTEM ""
#define LISTEN_PORT "sys_listen_port"
#define LISTEN_IP "sys_listen_ip"
#define WORK_THREADS "sys_work_threads"
#define UPDATE_INTERVAL "sys_update_interval"
#define KEEPALIVE_TIMER "sys_keepalive_timer"
#define UPDATE_PEOPLE_TIMER ""
#define SAVE_TIMER "sys_save_timer"
#define PING_TIMER "sys_ping_timer"
#define LOG_LEVEL "sys_log_level"
#define SEND_QUEUE_SIZE "sys_send_queue_size"
#define RECV_QUEUE_SIZE "sys_recv_queue_size"
#define EPOLL_MAX_SIZE "sys_epoll_max_size"
#define CONNECT_SERVER "connect_server"
#define SERVER_NAME "server_name"
#define SERVER_HOST "server_host"
#define SERVER_PORT "server_port"
#define SERVER_SNDBUF "send_buffer"
#define SERVER_RCVBUF "recv_buffer"

#define NODE_ID ""
#define GAME_ID ""
#define STEP_MAX_TIME ""

#define DATABASE ""
#define USERNAME ""
#define PASSWORD ""
#define DBNAME ""
#define HOST ""
#define PORT ""
#define MAX_CONNECTS "max_connects"

#define MEMCACHE "memcache"
#define POOL "pool"
#define MEMSERVERS "memservers"
#define WEIGHT "weight"

#define BLACK_SERVER "black_server"

#define DEF_SEND_BUFFER "svr_def_send_buffer"
#define DEF_RECV_BUFFER "svr_def_recv_buffer"

CGlobalConfig *CGlobalConfig::m_pInstance = NULL;

CGlobalConfig *CGlobalConfig::getInstance() {
  if (m_pInstance == NULL) {
    m_pInstance = new CGlobalConfig;
    LOG(_DEBUG_, "new CGlobalConfig");
  }
  return m_pInstance;
}

void CGlobalConfig::release() {
  if (m_pInstance != NULL) {
    delete m_pInstance;
    m_pInstance = NULL;
    LOG(_DEBUG_, "delete CGlobalConfig");
  }
}

CGlobalConfig::CGlobalConfig() {}

CGlobalConfig::~CGlobalConfig() {
  std::list<PCONNECT_SERVER>::iterator iterserver = m_serverlist.begin();
  for (; iterserver != m_serverlist.end(); ++iterserver) {
    if ((*iterserver))
      delete (*iterserver);
  }
  m_serverlist.clear();

  std::vector<PMEM_SERVER>::iterator iter = m_memlst.begin();
  for (; iter < m_memlst.end(); ++iter) {
    if (*iter)
      delete (*iter);
  }
  m_memlst.clear();
}

bool CGlobalConfig::initSysConfig(const std::string &path) {
  std::ifstream f;
  f.open(path.c_str());
  if (!f.is_open()) {
    char szErrMsg[256];
    sprintf(szErrMsg, "%s open failed", path.c_str());
    LOG(_ERROR_, szErrMsg);
    return false;
  }

  Json::Reader r;
  Json::Value v;

  if (!r.parse(f, v, false)) {
    LOG(_ERROR_, "parse config failed");
    return false;
  }

  m_cfg.listen_ip = v[LISTEN_IP].asString();
  m_cfg.listen_port = v[LISTEN_PORT].asInt();
  m_cfg.send_queue_size = v[SEND_QUEUE_SIZE].asInt();
  m_cfg.recv_queue_size = v[RECV_QUEUE_SIZE].asInt();
  m_cfg.work_threads = v[WORK_THREADS].asInt();
  m_cfg.epoll_max_size = v[EPOLL_MAX_SIZE].asInt();
  m_cfg.que_savetime = v[SAVE_TIMER].asInt();
  m_cfg.ping_timer = v[PING_TIMER].asInt();
  m_cfg.update_interval = v[UPDATE_INTERVAL].asInt();
  m_cfg.keepalive_timer = v[KEEPALIVE_TIMER].asInt();
  m_cfg.loglevel = v[LOG_LEVEL].asInt();

  uint32_t sendbuffer = 8192;
  uint32_t recvbuffer = 8192;

  sendbuffer = v[DEF_SEND_BUFFER].asInt();
  recvbuffer = v[DEF_RECV_BUFFER].asInt();

  m_cfg.maxsendbuf = sendbuffer == 0 ? 8192 : sendbuffer;
  m_cfg.maxrecvbuf = recvbuffer == 0 ? 8192 : recvbuffer;

  std::list<PCONNECT_SERVER>::iterator iterserver = m_serverlist.begin();
  for (; iterserver != m_serverlist.end(); ++iterserver) {
    if ((*iterserver)) {
      delete (*iterserver);
    }
  }
  m_serverlist.clear();

  Json::Value::iterator iter = v[CONNECT_SERVER].begin();
  for (; iter != v[CONNECT_SERVER].end(); ++iter) {
    PCONNECT_SERVER pserver = new __CONNECT_SERVER__;
    pserver->send_buffer = sendbuffer;
    pserver->recv_buffer = recvbuffer;
    pserver->name = (*iter)[SERVER_NAME].asString();
    pserver->host = (*iter)[SERVER_HOST].asString();
    pserver->port = (*iter)[SERVER_PORT].asInt();
    pserver->send_buffer = (*iter)[SERVER_SNDBUF].asInt();
    pserver->recv_buffer = (*iter)[SERVER_RCVBUF].asInt();

    m_serverlist.push_back(pserver);
  }

  return true;
}

uint32_t CGlobalConfig::getQueueTimer() { return m_cfg.que_savetime; }

uint32_t CGlobalConfig::getPingTimer() { return m_cfg.ping_timer; }

uint32_t CGlobalConfig::getUpdateInterval() { return m_cfg.update_interval; }

uint32_t CGlobalConfig::getKeepaliveTimer() { return m_cfg.keepalive_timer; }

uint32_t CGlobalConfig::getLogLevel() { return m_cfg.loglevel; }

PCONNECT_SERVER CGlobalConfig::getMainServer() {
  std::list<PCONNECT_SERVER>::iterator iter = m_serverlist.begin();
  for (; iter != m_serverlist.end(); ++iter) {
    if ((*iter)->name == "main") {
      return *iter;
    }
  }
  return NULL;
}

PCONNECT_SERVER CGlobalConfig::getDistributeServer() {
  std::list<PCONNECT_SERVER>::iterator iter = m_serverlist.begin();
  for (; iter != m_serverlist.end(); ++iter) {
    if ((*iter)->name == "distribute") {
      return *iter;
    }
  }
  return NULL;
}

PCONNECT_SERVER CGlobalConfig::getUserCenterServer() {
  std::list<PCONNECT_SERVER>::iterator iter = m_serverlist.begin();
  for (; iter != m_serverlist.end(); ++iter) {
    if ((*iter)->name == "usercenter") {
      return *iter;
    }
  }
  return NULL;
}

PCONNECT_SERVER CGlobalConfig::getLogServer() {
  std::list<PCONNECT_SERVER>::iterator iter = m_serverlist.begin();
  for (; iter != m_serverlist.end(); ++iter) {
    if ((*iter)->name == "log") {
      return *iter;
    }
  }
  return NULL;
}
