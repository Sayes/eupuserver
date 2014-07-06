#include "eupulogger4system.h"
#include "globaldef.h"
#include "netcommon.h"
#include "common.h"

SOCKET_SET* initSocketset(int fd, time_t conntime, const string& peerip, unsigned short peerport, int ntype)
{
    SOCKET_KEY* key = new SOCKET_KEY;
    if (!key)
    {
        exit(-1);
    }

    SOCKET_SET* socketset = new SOCKET_SET;
    if (!socketset)
    {
        delete key;
        key = NULL;
        exit(-1);
    }

    key->fd = fd;
    key->connect_time = conntime;
    if (!socketset->init(key, peerip, peerport, ntype))
    {
        delete key;
        key = NULL;
        delete socketset;
        socketset = NULL;
        return NULL;
    }

    LOG(_INFO_, "initSocketset() end, fd=%d, time=%u, ip=%s, port=%d, type=%d", fd, conntime, peerip.c_str(), peerport, ntype);
    return socketset;
}

bool setNonBlock(int sockfd)
{
#ifdef OS_LINUX
    int opts = fcntl(sockfd, F_GETFL);
    if (-1 == opts)
    {
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
    return true;
}

int recv_msg(int fd, char* buf, int &nlen)
{
    int n = nlen;
    char* p = buf;
    int nRet = 2;
    int nread = 0;

    while (n > 0)
    {
#ifdef OS_LINUX
        nread = recv(fd, p, n, MSG_NOSIGNAL);
#elif OS_WINDOWS
        //TODO, be sure with param replace MSG_NOSIGNAL
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

int send_msg(int fd, char* buf, int &nlen)
{
    int n = nlen;
    int nRet = 1;
    int nsend = 0;
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

int doNonblockConnect(PCONNECT_SERVER pserver, int timeout, const string& localip)
{
    if (!pserver)
    {
        LOG(_ERROR_, "doNonblockConnect() error, pserver == NULL");
        return -1;
    }

    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0)
    {
        LOG(_ERROR_, "doNonblockConnect() error, socket() failed");
        return -1;
    }

    if (!localip.empty())
    {
        struct sockaddr_in localaddr = {0};
        localaddr.sin_family = AF_INET;
        localaddr.sin_addr.s_addr = fgAtoN(localip.c_str());

        if (!bind(fd, (struct sockaddr*)&localaddr, sizeof(localaddr)) < 0)
        {
            LOG(_ERROR_, "doNonblockConnect() error, bind() failed");
#ifdef OS_LINUX
            close(fd);
#elif OS_WINDOWS
            closesocket(fd);
#endif
            return -1;
        }
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
        closesocket(fd);
#endif
        return -1;
    }

    unsigned int nsend = pserver->send_buffer;
    unsigned int nrecv = pserver->recv_buffer;

    if (setsockopt(fd, SOL_SOCKET, SO_SNDBUF, (const char*)&nsend, sizeof(nsend)) < 0)
    {
        LOG(_ERROR_, "doNonblockConnect() error, setsockopt(SO_SNDBUF) failed");
#ifdef OS_LINUX
        close(fd);
#elif OS_WINDOWS
        closesocket(fd);
#endif
        return -1;
    }

    if (setsockopt(fd, SOL_SOCKET, SO_RCVBUF, (const char*)&nrecv, sizeof(nrecv)) < 0)
    {
        LOG(_ERROR_, "doNonblockConnect() error, setsockopt(SO_RCVBUF) failed");
#ifdef OS_LINUX
        close(fd);
#elif OS_WINDOWS
        closesocket(fd);
#endif
        return -1;
    }

    int opt = 1;
    if (setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (const char*)&opt, sizeof(opt)) < 0)
    {
        LOG(_ERROR_, "doNonblockConnect() error, setsockopt(TCP_NODELAY) failed");
#ifdef OS_LINUX
        close(fd);
#elif OS_WINDOWS
        closesocket(fd);
#endif
        return -1;
    }

    int nret = connect(fd, (struct sockaddr*)&addr, sizeof(addr));
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
            LOG(_ERROR_, "doNonblockConnect() error, connect() failed");
            close(fd);
            return -1;
        }
    }
#elif OS_WINDOWS
    if (nret == SOCKET_ERROR)
    {
        int err = WSAGetLastError();
        if (!(err == WSAEINPROGRESS || err == WSAEWOULDBLOCK))
        {
            LOG(_ERROR_, "doNonblockConnect() error, connect() failed, error=%d", err);
            closesocket(fd);
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
        closesocket(fd);
#endif
        return -1;
    }
    else if (nret < 0)
    {
        LOG(_ERROR_, "doNonblockConnect() error, select() failed, return < 0");
#ifdef OS_LINUX
        close(fd);
#elif OS_WINDOWS
        closesocket(fd);
#endif
        return -1;
    }

    int sock_err = 0;
    int sock_err_len = sizeof(sock_err);
    nret = getsockopt(fd, SOL_SOCKET, SO_ERROR, (char*)&sock_err, (socklen_t*)&sock_err_len);

    if (nret < 0 || sock_err != 0)
    {
        LOG(_ERROR_, "doNonblockConnect() error, getsockopt(SO_ERROR) failed");
#ifdef OS_LINUX
        close(fd);
#elif OS_WINDOWS
        closesocket(fd);
#endif
        return -1;
    }

    LOG(_INFO_, "doNonblockConnect() end, fd=%d", fd);
    return fd;
}


