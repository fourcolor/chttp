#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <time.h>
#include <unistd.h>
#include "error.h"
#define MAX_EVENTS 256
#define SERVER_PORT 8080
#define MAX_BUFFER_SIZE 8192
static void setnonblock(int fd)
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
    struct epoll_event ev = {.events = EPOLLIN | EPOLLET}, events[MAX_EVENTS];
    int epollfd, listenfd, flag;
    epollfd = epoll_create1(0);
    if (epollfd < 0)
    {
        err_handle("Error: Fail to create epoll");
    }
    struct sockaddr_in sever_addr = {
        .sin_family = PF_INET,
        .sin_port = htons(SERVER_PORT),
        .sin_addr.s_addr = htonl(INADDR_ANY),
    };
    socklen_t socklen = sizeof(sever_addr);
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd < 0)
    {
        err_handle("Error: Fail to create a socket. ");
    }

    setnonblock(listenfd);

    /**
     * @brief Binding
     */
    if (bind(listenfd, (struct sockaddr *)&sever_addr, socklen) < 0)
    {
        err_handle("Error: Fail to bind. ");
    }
    printf("Listenner was binded to %s\n", inet_ntoa(sever_addr.sin_addr));

    if (listen(listenfd, SOMAXCONN) < 0)
    {
        err_handle("Error: Fail to listen. ");
    }

    ev.data.fd = listenfd;
    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, listenfd, &ev) < 0)
    {
        err_handle("Error: epoll_ctl can not listen_sock");
    }

    while (1)
    {
        struct sockaddr_in client_addr;
        int nfds;
        if ((nfds = epoll_wait(epollfd, events, MAX_EVENTS, -1)) < 0)
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
                setnonblock(clientfd);
                ev.events = EPOLLIN | EPOLLET;
                ev.data.fd = clientfd;
                if (epoll_ctl(epollfd, EPOLL_CTL_ADD, clientfd, &ev) == -1)
                {
                    err_handle("Error: Fail to epoll_ctl");
                }
            }
            else
            {
                int clientfd = events[i].data.fd;
                char buf[MAX_BUFFER_SIZE];
                int len;
                if ((len = recv(clientfd, buf, MAX_BUFFER_SIZE, 0)) < 0)
                {
                    printf("%s\n", buf);
                    err_handle("Fail to receive. ");
                }
                else
                {
                    printf("%s\n", buf);
                    if (len == 0)
                    {
                        if (close(clientfd) < 0)
                            err_handle("Fail to close");
                    }
                    else
                    {
                        printf("Client #%d :> %s", clientfd, buf);
                        if (send(clientfd, "HTTP/1.0 200 OK\r\n\r\n", 19, 0) < 0)
                            err_handle("Fail to send");
                    }
                }
            }
        }
    }
    return 0;
}