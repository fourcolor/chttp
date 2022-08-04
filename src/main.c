#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <time.h>
#include <unistd.h>
#define MAX_EVENTS 256
#define SERVER_PORT 8080

static void err_handle(const char *str)
{
    perror(str);
    exit(-1);
}

static void setnonblocking(int fd)
{
    int flag;
    if (flag = fcntl(fd, F_GETFL, 0) < 0)
    {
        err_handle("Error: Fail to get socket flag. ");
    }
    flag |= O_NONBLOCK;
    flag |= O_NDELAY;
    if (fcntl(fd, F_SETFD, flag) < 0)
    {
        err_handle("Error: Fail to set socket flag. ");
    }
}

int main()
{
    struct epoll_event ev, events[MAX_EVENTS];
    int nfds, epollfd, listenfd, flag;
    epollfd = epoll_create1(0);
    if (epollfd < 0)
    {
        err_handle("Error: Fail to create epoll");
    }
    struct sockaddr_in sever_addr = {
        .sin_addr.s_addr = htonl(INADDR_ANY),
        .sin_family = AF_INET,
        .sin_port = htons(SERVER_PORT),
    };
    socklen_t socklen = sizeof(sever_addr);
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd < 0)
    {
        err_handle("Error: Fail to create a socket. ");
    }

    /**
     * @brief Binding
     */
    if (bind(listen, (struct sockaddr *)&sever_addr, socklen) < 0)
    {
        err_handle("Error: Fail to bind. ");
    }
    printf("Listner was binded to %s\n", inet_ntoa(sever_addr.sin_addr));

    if (listen(listenfd, SOMAXCONN) < 0)
    {
        err_handle("Error: Fail to listen. ");
    }

    ev.events = EPOLLIN;
    ev.data.fd = listenfd;
    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, listenfd, &ev) == -1)
    {
        err_handle("Error: epoll_ctl can not listen_sock");
    }

    while (1)
    {
        struct sockaddr_in client_addr;
        int clientfd;
        if (nfds = epoll_wait(epollfd, events, MAX_EVENTS, -1) == -1)
        {
            err_handle("Error: epoll_wait fail");
        }
        for (int i = 0; i < nfds; i++)
        {
            if (events[i].data.fd == listenfd)
            {
                int clientfd = accept(listenfd, (struct sockaddr *)&client_addr, &socklen);
                if (clientfd < 0)
                {
                    err_handle("Error: Fail to accept. ");
                }
                setnonblocking(clientfd);
                ev.events = EPOLLIN | EPOLLET;
                ev.data.fd = clientfd;
                if (epoll_ctl(epollfd, EPOLL_CTL_ADD, clientfd, &ev) == -1)
                {
                    err_handle("Error: Fail to epoll_ctl");
                }
                else
                {
                    do_use_fd(events[i].data.fd);
                }
            }
        }
    }
    return 0;
}