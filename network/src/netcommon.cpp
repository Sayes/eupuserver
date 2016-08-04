#include "eupulogger4system.h"
#include "globaldef.h"
#include "netcommon.h"
#include "common.h"

SOCKET_SET* initSocketset(int32_t fd, time_t conntime, const string& peerip, uint16_t peerport, int32_t ntype)
{
    SOCKET_KEY* key = new SOCKET_KEY;
    if (!key)
    {
        LOG(_ERROR_, "initSocketset() error, new SOCKET_KEY failed");
        ::exit(-1);
    }

    SOCKET_SET* socketset = new SOCKET_SET;
    if (!socketset)
    {
        LOG(_ERROR_, "initSocketset() error, new SOCKET_SET failed");
        delete key;
        key = NULL;
        ::exit(-1);
    }

    key->fdkey = fd;
#ifdef STRONG_KEY
    key->connect_time = conntime;
#endif
    if (!socketset->init(key, peerip, peerport, ntype))
    {
        LOG(_ERROR_, "initSocketset() error, socketset->init() failed fd=%d, time = %u, ip=%s, prot=%d, type=%d", fd, conntime, peerip.c_str(), peerport, ntype);
        delete key;
        key = NULL;
        delete socketset;
        socketset = NULL;
        return NULL;
    }

    LOG(_INFO_, "initSocketset() end, fd=%d, time=%u, ip=%s, port=%d, type=%d", fd, conntime, peerip.c_str(), peerport, ntype);
    return socketset;
}

bool setNonBlock(int32_t sockfd)
{
#ifdef OS_LINUX
    int32_t opts = fcntl(sockfd, F_GETFL);
    if (-1 == opts)
    {
        LOG(_ERROR_, "setNonBlock() error, fd=%d", sockfd);
        return false;
    }

    opts = opts | O_NONBLOCK;
    if (fcntl(sockfd, F_SETFL, opts) < 0)
    {
        LOG(_ERROR_, "setNonBlock() error, fcntl() failed");
        return false;
    }
#elif OS_WINDOWS
    u_long mode = 1;
    if (ioctlsocket(sockfd, FIONBIO, &mode) == SOCKET_ERROR)
    {
        LOG(_ERROR_, "setNonBlock() error, ioctlsocket() failed, sockfd=%d, error=%ld", sockfd, WSAGetLastError());
        return false;
    }
#endif
    LOG(_INFO_, "setNonBlock() end, fd=%d", sockfd);
    return true;
}

int32_t recv_msg(int32_t fd, char* buf, int32_t& nlen)
{
    int32_t n = nlen;
    char* p = buf;
    int32_t nRet = 2;
    int32_t nread = 0;

    while (n > 0)
    {
#ifdef OS_LINUX
        nread = recv(fd, p, n, MSG_NOSIGNAL);
#elif OS_WINDOWS
        //TODO, suspend, be sure with param replace MSG_NOSIGNAL
        nread = recv(fd, p, n, 0);
#endif
        if (nread < 0)
        {
            if (errno == EINTR)
            {
                continue;
            }
            else if (errno == EAGAIN)
            {
                nRet = 1;
                break;
            }
            nRet = -1;
            break;
        }
        else if (nread == 0)
        {
            nRet = 0;
            break;
        }

        n -= nread;
        p += nread;
    }

    nlen = nlen - n;
    return nRet;
}

int32_t send_msg(int32_t fd, char* buf, int32_t& nlen)
{
    int32_t n = nlen;
    int32_t nRet = 1;
    int32_t nsend = 0;
    char* p = buf;

    while (n > 0)
    {
#ifdef OS_LINUX
        nsend = send(fd, p, n, MSG_NOSIGNAL);
#elif OS_WINDOWS
        nsend = send(fd, p, n, 0);
#endif
        if (nsend < 0)
        {
            if (errno == EINTR)
            {
                continue;
            }
            else if (errno == EAGAIN)
            {
                nRet = 0;
                break;
            }
            nRet = -1;
            break;
        }
        n -= nsend;
        p += nsend;
    }

    nlen = nlen - n;
    return nRet;
}

int32_t doNonblockConnect(PCONNECT_SERVER pserver, int32_t timeout, const string& localip)
{
    if (!pserver)
    {
        LOG(_ERROR_, "doNonblockConnect() error, pserver == NULL");
        return -1;
    }

    if (localip.empty())
    {
        LOG(_ERROR_, "doNonblockConnect() error, localip is empty");
        return -1;
    }

    int32_t fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0)
    {
        LOG(_ERROR_, "doNonblockConnect() error, socket() failed");
        return -1;
    }

    struct sockaddr_in localaddr = {0};
    localaddr.sin_family = AF_INET;
    localaddr.sin_addr.s_addr = fgAtoN(localip.c_str());
    if (!bind(fd, (struct sockaddr*)&localaddr, sizeof(localaddr)) < 0)
    {
        LOG(_ERROR_, "doNonblockConnect() error, bind() failed");
#ifdef OS_LINUX
        close(fd);
#elif OS_WINDOWS
        ::closesocket(fd);
#endif
        return -1;
    }

    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(pserver->port);
    addr.sin_addr.s_addr = fgAtoN(pserver->host.c_str());

    if (!setNonBlock(fd))
    {
        LOG(_ERROR_, "doNonblockConnect() error, setNonBlock() failed");
#ifdef OS_LINUX
        close(fd);
#elif OS_WINDOWS
        ::closesocket(fd);
#endif
        return -1;
    }

    uint32_t nsend = pserver->send_buffer;
    uint32_t nrecv = pserver->recv_buffer;

    if (setsockopt(fd, SOL_SOCKET, SO_SNDBUF, (const char*)&nsend, sizeof(nsend)) < 0)
    {
        LOG(_ERROR_, "doNonblockConnect() error, setsockopt(SO_SNDBUF) failed");
#ifdef OS_LINUX
        close(fd);
#elif OS_WINDOWS
        ::closesocket(fd);
#endif
        return -1;
    }

    if (setsockopt(fd, SOL_SOCKET, SO_RCVBUF, (const char*)&nrecv, sizeof(nrecv)) < 0)
    {
        LOG(_ERROR_, "doNonblockConnect() error, setsockopt(SO_RCVBUF) failed");
#ifdef OS_LINUX
        close(fd);
#elif OS_WINDOWS
        ::closesocket(fd);
#endif
        return -1;
    }

    int32_t opt = 1;
    if (setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (const char*)&opt, sizeof(opt)) < 0)
    {
        LOG(_ERROR_, "doNonblockConnect() error, setsockopt(TCP_NODELAY) failed");
#ifdef OS_LINUX
        close(fd);
#elif OS_WINDOWS
        ::closesocket(fd);
#endif
        return -1;
    }

    int32_t nret = connect(fd, (struct sockaddr*)&addr, sizeof(addr));
    if (nret == 0)
    {
        LOG(_INFO_, "doNonblockConnect() done, connect() successed");
        return fd;
    }

#ifdef OS_LINUX
    if (nret < 0)
    {
        if (!(errno == EINPROGRESS || errno == EWOULDBLOCK))
        {
            LOG(_INFO_, "doNonblockConnect() error, connect() failed");
            close(fd);
            return -1;
        }
    }
#elif OS_WINDOWS
    if (nret == SOCKET_ERROR)
    {
        int32_t err = WSAGetLastError();
        if (!(err == WSAEINPROGRESS || err == WSAEWOULDBLOCK))
        {
            LOG(_INFO_, "doNonblockConnect() error, connect() failed, error=%d", err);
            ::closesocket(fd);
            return -1;
        }
    }
#endif

    fd_set wset;
    struct timeval tv = {0};

    tv.tv_sec = timeout;
    tv.tv_usec = 0;

    FD_ZERO(&wset);
    FD_SET(fd, &wset);

    nret = select(fd + 1, NULL, &wset, NULL, &tv);
    if (nret == 0)
    {
        LOG(_ERROR_, "doNonblockConnect() error, select() failed, return 0");
#ifdef OS_LINUX
        close(fd);
#elif OS_WINDOWS
        ::closesocket(fd);
#endif
        return -1;
    }
    else if (nret < 0)
    {
        LOG(_ERROR_, "doNonblockConnect() error, select() failed, return < 0");
#ifdef OS_LINUX
        close(fd);
#elif OS_WINDOWS
        ::closesocket(fd);
#endif
        return -1;
    }

    int32_t sock_err = 0;
    int32_t sock_err_len = sizeof(sock_err);
    nret = getsockopt(fd, SOL_SOCKET, SO_ERROR, (char*)&sock_err, (socklen_t*)&sock_err_len);

    if (nret < 0 || sock_err != 0)
    {
        LOG(_ERROR_, "doNonblockConnect() error, getsockopt(SO_ERROR) failed");
#ifdef OS_LINUX
        close(fd);
#elif OS_WINDOWS
        ::closesocket(fd);
#endif
        return -1;
    }

    LOG(_INFO_, "doNonblockConnect() end, fd=%d", fd);
    return fd;
}


