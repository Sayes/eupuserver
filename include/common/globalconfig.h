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

	char* getDbHostName();
	UINT getDbHostPort();
	char* getDbUserName();
	char* getDbUserPssw();
	char* getDbDatabase();
	UINT getMemPoolCount();

	const vector<PMEM_SERVER>* getMemcacheServers()
	{
		return &m_memlst;    
	}

	UINT getListenPort()
	{
		return m_cfg.listen_port;
	}

	std::string getListenIp()
	{
		return m_cfg.listen_ip;
	}

	UINT getSendQueueSize()
	{
		return m_cfg.send_queue_size;
	}

	UINT getRecvQueueSize()
	{
		return m_cfg.recv_queue_size;
	}

	UINT getWorkThreads()
	{
		return m_cfg.work_threads;
	}

	UINT getMaxEpollSize()
	{
		return m_cfg.epoll_max_size;
	}

	UINT getSocketSendBuf()
	{
		return m_cfg.maxsendbuf;
	}

	UINT getSocketRecvBuf()
	{
		return m_cfg.maxrecvbuf;
	}

	UINT getQueueTimer();
	UINT getPingTimer();
	UINT getDbMaxConnects();
	UINT getUpdateInterval();
	UINT getKeepaliveTimer();
	UINT getUpdatePeopleTimer();
	UINT getLogLevel();

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

	UINT mem_pool;
	vector<PMEM_SERVER> m_memlst;

	st_global_cfg m_cfg;
	static CGlobalConfig* m_pInstance;
};

#endif//__GLOBALCONFIG_H__
