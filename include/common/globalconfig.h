#ifndef __GLOBALCONFIG_H__
#define __GLOBALCONFIG_H__

#include <string>

using namespace std;

typedef struct log_cfg {
    string filename;
    string filenum;
    string filepath;
    string filedataext;
    string filechkext;
}LOG_CFG;

typedef struct st_global_cfg {
    char db_hostname[256];
    UINT db_hostport;
    char db_username[256];
    char db_userpssw[256];
    char db_database[256];

    UINT que_savetime;
    UINT ping_timer;
    UINT update_interval;
    UINT db_maxconnects;
    UINT keepalive_timer;
    UINT loglevel;
    UINT update_people_timer;

    unsigned short listen_port;
    string listen_ip;
    UINT send_queue_size;
    UINT recv_queue_size;
    UINT work_threads;
    UINT epoll_max_size;

    UINT maxsendbuf;
    UINT maxrecvbuf;
}st_global_cfg;

struct __CONNECT_SERVER__ {
	string name;
	string host;
	unsigned short port;
	unsigned int send_buffer;
	unsigned int recv_buffer;
	__CONNECT_SERVER__()
	{
		port = 0;
		send_buffer = 8192;
		recv_buffer = 8192;
	}
};

typedef __CONNECT_SERVER__*	PCONNECT_SERVER;

struct __MEM_SERVER__ {
    string host;
    unsigned short port;
    unsigned int weight;
};

typedef struct __MEM_SERVER__* PMEM_SERVER;

class CGlobalConfig {
public:
    static CGlobalConfig* getInstance();
    static void release();

    char* GetDbHostName();
    UINT GetDbHostPort();
    char* GetDbUserName();
    char* GetDbUserPssw();
    char* GetDbDatabase();
    UINT GetMemPoolCount();

    const vector<PMEM_SERVER>* GetMemcacheServers()
    {
        return &m_memlst;    
    }

    UINT GetListenPort()
    {
        return m_cfg.listen_port;
    }

    std::string GetListenIp()
    {
        return m_cfg.listen_ip;
    }

    UINT GetSendQueueSize()
    {
        return m_cfg.send_queue_size;
    }

    UINT GetRecvQueueSize()
    {
        return m_cfg.recv_queue_size;
    }

    UINT GetWorkThreads()
    {
        return m_cfg.work_threads;
    }

    UINT GetEpollMaxSize()
    {
        return m_cfg.epoll_max_size;
    }

    UINT GetSocketSendBuf()
    {
        return m_cfg.maxsendbuf;
    }

    UINT GetSocketRecvBuf()
    {
        return m_cfg.maxrecvbuf;
    }

    UINT GetQueueTimer();
    UINT GetPingTimer();
    UINT GetDbMaxConnects();
    UINT GetUpdateInterval();
    UINT GetKeepaliveTimer();
    UINT GetUpdatePeopleTimer();
    UINT GetLogLevel();

    bool initSysConfig(const std::string& path);
    bool initDbConfig(const std::string& path);
    bool initLogConfig(const std::string& path);

protected:
    CGlobalConfig();
    virtual ~CGlobalConfig();

private:
    LOG_CFG onlinetime;
    LOG_CFG onlinecount;
    LOG_CFG onlinetotal;

    UINT mem_pool;
    vector<PMEM_SERVER> m_memlst;

    st_global_cfg m_cfg;
    static CGlobalConfig* m_pInstance;
};

#endif//__GLOBALCONFIG_H__
