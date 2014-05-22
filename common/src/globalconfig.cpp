#include <fstream>
#include "json/json.h"
#include "globaldef.h"
#include "globalconfig.h"
#include "eupulogger4system.h"

#define CONFIGURATION   ""
#define SYSTEM          ""
#define LISTEN_PORT     "sys_listen_port"
#define LISTEN_IP       "sys_listen_ip"
#define WORK_THREADS    "sys_work_threads"
#define UPDATE_INTERVAL "sys_update_interval"
#define KEEPALIVE_TIMER "sys_keepalive_timer"
#define UPDATE_PEOPLE_TIMER ""
#define SAVE_TIMER      "sys_save_timer"
#define PING_TIMER      "sys_ping_timer"
#define LOG_LEVEL       "sys_log_level"
#define SEND_QUEUE_SIZE "sys_send_queue_size"
#define RECV_QUEUE_SIZE "sys_recv_queue_size"
#define EPOLL_MAX_SIZE  "sys_epoll_max_size"

#define NODE_ID         ""
#define GAME_ID         ""
#define STEP_MAX_TIME   ""

#define DATABASE        ""
#define USERNAME        ""
#define PASSWORD        ""
#define DBNAME          ""
#define HOST            ""
#define PORT            ""
#define MAX_CONNECTS    "max_connects"

#define MEMCACHE        "memcache"
#define POOL            "pool"
#define MEMSERVERS      "memservers"
#define WEIGHT          "weight"

#define BLACK_SERVER    "black_server"

#define DEF_SEND_BUFFER "svr_def_send_buffer"
#define DEF_RECV_BUFFER "svr_def_recv_buffer"

CGlobalConfig* CGlobalConfig::m_pInstance = NULL;

CGlobalConfig* CGlobalConfig::getInstance()
{
	if (m_pInstance == NULL)
	{
		m_pInstance = new CGlobalConfig;
		LOG(_DEBUG_, "new CGlobalConfig");
	}
	return m_pInstance;
}

void CGlobalConfig::release()
{
	if (m_pInstance != NULL)
	{
		delete m_pInstance;
		m_pInstance = NULL;
		LOG(_DEBUG_, "delete CGlobalConfig");
	}
}

CGlobalConfig::CGlobalConfig()
{
}

CGlobalConfig::~CGlobalConfig()
{
	vector<PMEM_SERVER>::iterator iter = m_memlst.begin();
	for (; iter < m_memlst.end(); ++iter)
	{
		if (*iter)
			delete (*iter);
	}
	m_memlst.clear();
}

bool CGlobalConfig::initSysConfig(const std::string& path)
{
	std::ifstream f;
	f.open(path.c_str());
	if (!f.is_open())
	{
		char szErrMsg[256];
		sprintf(szErrMsg, "%s open failed", path.c_str());
		LOG(_ERROR_, szErrMsg);
		return false;
	}

	Json::Reader r;
	Json::Value v;

	if (!r.parse(f, v, NULL))
	{
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

	int sendbuffer = 8192;
	int recvbuffer = 8192;

	sendbuffer = v[DEF_SEND_BUFFER].asInt();
	recvbuffer = v[DEF_RECV_BUFFER].asInt();

	m_cfg.maxsendbuf = sendbuffer == 0 ? 8192 : sendbuffer;
	m_cfg.maxrecvbuf = recvbuffer == 0 ? 8192 : recvbuffer;

	return true;
}

UINT CGlobalConfig::getQueueTimer()
{
	return m_cfg.que_savetime; 
}

UINT CGlobalConfig::getPingTimer()
{
	return m_cfg.ping_timer;
}

UINT CGlobalConfig::getUpdateInterval()
{
	return m_cfg.update_interval;
}

UINT CGlobalConfig::getKeepaliveTimer()
{
	return m_cfg.keepalive_timer;
}

UINT CGlobalConfig::getLogLevel()
{
	return m_cfg.loglevel;
}

PCONNECT_SERVER CGlobalConfig::getMainServer()
{
	return NULL;
}

PCONNECT_SERVER CGlobalConfig::getDistributeServer()
{
	return NULL;
}

PCONNECT_SERVER CGlobalConfig::getUserCenterServer()
{
	return NULL;
}

PCONNECT_SERVER CGlobalConfig::getLogServer()
{
	return NULL;
}

