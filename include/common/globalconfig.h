#ifndef __GLOBALCONFIG_H__
#define __GLOBALCONFIG_H__

#include <string>


using namespace std;

typedef struct log_cfg
{
    string filename;
    string filenum;
    string filepath;
    string filedataext;
    string filechkext;
} LOG_CFG;

typedef struct st_global_cfg
{
    char db_hostname[256];
    uint32 db_hostport;
    char db_username[256];
    char db_userpssw[256];
    char db_database[256];

    uint32 que_savetime;
    uint32 ping_timer;
    uint32 update_interval;
    uint32 db_maxconnects;
    uint32 keepalive_timer;
    uint32 loglevel;
    uint32 update_people_timer;

    uint16 listen_port;
    string listen_ip;
    uint32 send_queue_size;
    uint32 recv_queue_size;
    uint32 work_threads;
    uint32 epoll_max_size;

    uint32 maxsendbuf;
    uint32 maxrecvbuf;
} st_global_cfg;

struct __CONNECT_SERVER__
{
    string name;
    string host;
    uint16 port;
    uint32 send_buffer;
    uint32 recv_buffer;
    __CONNECT_SERVER__()
    {
        port = 0;
        send_buffer = 8192;
        recv_buffer = 8192;
    }
};

typedef __CONNECT_SERVER__* PCONNECT_SERVER;

struct __MEM_SERVER__
{
    string host;
    uint16 port;
    uint32 weight;
};

typedef struct __MEM_SERVER__* PMEM_SERVER;

class CGlobalConfig {
public:
    static CGlobalConfig* getInstance();
    static void release();

    char* getDbHostName();
    uint32 getDbHostPort();
    char* getDbUserName();
    char* getDbUserPssw();
    char* getDbDatabase();
    uint32 getMemPoolCount();

    const vector<PMEM_SERVER>* getMemcacheServers()
    {
        return &m_memlst;
    }

    uint32 getListenPort()
    {
        return m_cfg.listen_port;
    }

    std::string getListenIp()
    {
        return m_cfg.listen_ip;
    }

    uint32 getSendQueueSize()
    {
        return m_cfg.send_queue_size;
    }

    uint32 getRecvQueueSize()
    {
        return m_cfg.recv_queue_size;
    }

    uint32 getWorkThreads()
    {
        return m_cfg.work_threads;
    }

    uint32 getMaxEpollSize()
    {
        return m_cfg.epoll_max_size;
    }

    uint32 getSocketSendBuf()
    {
        return m_cfg.maxsendbuf;
    }

    uint32 getSocketRecvBuf()
    {
        return m_cfg.maxrecvbuf;
    }

    uint32 getQueueTimer();
    uint32 getPingTimer();
    uint32 getDbMaxConnects();
    uint32 getUpdateInterval();
    uint32 getKeepaliveTimer();
    uint32 getUpdatePeopleTimer();
    uint32 getLogLevel();

    bool initSysConfig(const std::string& path);
    bool initDbConfig(const std::string& path);
    bool initLogConfig(const std::string& path);

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

    uint32 mem_pool;
    vector<PMEM_SERVER> m_memlst;

    list<PCONNECT_SERVER> m_serverlist;
    st_global_cfg m_cfg;
    static CGlobalConfig* m_pInstance;
};

#endif//__GLOBALCONFIG_H__
