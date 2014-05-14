#include <fstream>
#include "json/json.h"
#include "globaldef.h"
#include "globalconfig.h"
#include "eupulogger4system.h"

#define CONFIGURATION   ""
#define SYSTEM          ""
#define LISTEN_PORT     "listen_port"
#define LISTEN_IP       "listen_ip"
#define WORK_THREADS    "work_threads"
#define UPDATE_INTERVAL "update_interval"
#define KEEPALIVE_TIMER "keepalive_timer"
#define UPDATE_PEOPLE_TIMER ""
#define SAVE_TIMER      ""
#define PING_TIMER      "ping_timer"
#define LOG_LEVEL       "log_level"
#define SEND_QUEUE_SIZE "send_queue_size"
#define RECV_QUEUE_SIZE "recv_queue_size"
#define EPOLL_MAX_SIZE  "epoll_max_size"

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
    m_cfg.ping_timer = v[PING_TIMER].asInt();
    m_cfg.update_interval = v[UPDATE_INTERVAL].asInt();
    m_cfg.keepalive_timer = v[KEEPALIVE_TIMER].asInt();
    m_cfg.loglevel = v[LOG_LEVEL].asInt();

    return true;
}

UINT CGlobalConfig::getQueueTimer()
{
    //TODO
    return 0; 
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

PCONNECT_SERVER getMainServer()
{
    return NULL;
}

PCONNECT_SERVER getDistributeServer()
{
    return NULL;
}

PCONNECT_SERVER getUserCenterServer()
{
    return NULL;
}

PCONNECT_SERVER getLogServer()
{
    return NULL;
}

