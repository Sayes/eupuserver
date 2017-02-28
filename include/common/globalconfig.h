// Copyright shenyizhong@gmail.com, 2014

#ifndef GLOBALCONFIG_H_
#define GLOBALCONFIG_H_

#include <string>
#include <vector>
#include <list>

typedef struct log_cfg {
  std::string filename;
  std::string filenum;
  std::string filepath;
  std::string filedataext;
  std::string filechkext;
} LOG_CFG;

typedef struct st_global_cfg {
  char db_hostname[256];
  uint32_t db_hostport;
  char db_username[256];
  char db_userpssw[256];
  char db_database[256];

  uint32_t que_savetime;
  uint32_t ping_timer;
  uint32_t update_interval;
  uint32_t db_maxconnects;
  uint32_t keepalive_timer;
  uint32_t loglevel;
  uint32_t update_people_timer;

  uint16_t listen_port;
  std::string listen_ip;
  uint32_t send_queue_size;
  uint32_t recv_queue_size;
  uint32_t work_threads;
  uint32_t epoll_max_size;

  uint32_t maxsendbuf;
  uint32_t maxrecvbuf;
} st_global_cfg;

struct __CONNECT_SERVER__ {
  std::string name;
  std::string host;
  uint16_t port;
  uint32_t send_buffer;
  uint32_t recv_buffer;
  __CONNECT_SERVER__() {
    port = 0;
    send_buffer = 8192;
    recv_buffer = 8192;
  }
};

typedef __CONNECT_SERVER__ *PCONNECT_SERVER;

struct __MEM_SERVER__ {
  std::string host;
  uint16_t port;
  uint32_t weight;
};

typedef struct __MEM_SERVER__ *PMEM_SERVER;

class CGlobalConfig {
public:
  static CGlobalConfig *getInstance();
  static void release();

  char *getDbHostName();
  uint32_t getDbHostPort();
  char *getDbUserName();
  char *getDbUserPssw();
  char *getDbDatabase();
  uint32_t getMemPoolCount();

  const std::vector<PMEM_SERVER> *getMemcacheServers() { return &m_memlst; }

  uint32_t getListenPort() { return m_cfg.listen_port; }

  std::string getListenIp() { return m_cfg.listen_ip; }

  uint32_t getSendQueueSize() { return m_cfg.send_queue_size; }

  uint32_t getRecvQueueSize() { return m_cfg.recv_queue_size; }

  uint32_t getWorkThreads() { return m_cfg.work_threads; }

  uint32_t getMaxEpollSize() { return m_cfg.epoll_max_size; }

  uint32_t getSocketSendBuf() { return m_cfg.maxsendbuf; }

  uint32_t getSocketRecvBuf() { return m_cfg.maxrecvbuf; }

  uint32_t getQueueTimer();
  uint32_t getPingTimer();
  uint32_t getDbMaxConnects();
  uint32_t getUpdateInterval();
  uint32_t getKeepaliveTimer();
  uint32_t getUpdatePeopleTimer();
  uint32_t getLogLevel();

  bool initSysConfig(const std::string &path);
  bool initDbConfig(const std::string &path);
  bool initLogConfig(const std::string &path);

  PCONNECT_SERVER getMainServer();
  PCONNECT_SERVER getDistributeServer();
  PCONNECT_SERVER getUserCenterServer();
  PCONNECT_SERVER getLogServer();

protected:
  CGlobalConfig();
  virtual ~CGlobalConfig();

private:
  LOG_CFG onlinetime;
  LOG_CFG onlinecount;
  LOG_CFG onlinetotal;

  uint32_t mem_pool;
  std::vector<PMEM_SERVER> m_memlst;

  std::list<PCONNECT_SERVER> m_serverlist;
  st_global_cfg m_cfg;
  static CGlobalConfig *m_pInstance;
};

#endif //  GLOBALCONFIG_H_
