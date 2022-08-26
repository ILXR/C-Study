#include <fcntl.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

/* 什么是信号驱动IO？

主要的实现：
    * Berkeley的实现使用SIGIO信号支持套接字和终端设备上的信号驱动式I/O；
    * SVR4使用SIGPOLL信号支持流设备上的信号驱动。

信号驱动IO，预先在内核中设置一个回调函数，当某个事件发生时，内核使用信号（SIGIO）通知进程来处理（运行回调函数）。 
它也可以看成是一种异步IO，因为检测 fd 是否有数据和是否可读写是在两个流程中做的。
它的优势是，进程没有收到 SIGIO 信号之前，不被阻塞，可以做其他事情。
它的劣势是，当数据量变大时，信号产生太频繁，性能会非常低。内核需要不断的把数据复制到用户态。

*/

int socket_fd = 0;

void do_sometime(int signal)
{
    struct sockaddr_in cli_addr;
    int clilen = sizeof(cli_addr);
    int clifd = 0;

    char buffer[256] = {0};
    // 此处调用 recvfrom 时，内核缓冲区已经有了数据，只需要复制到用户进程空间即可
    int len = recvfrom(socket_fd, buffer, 256, 0, (struct sockaddr *)&cli_addr,
                       (socklen_t *)&clilen);
    printf("Mes:%s", buffer);

    sendto(socket_fd, buffer, len, 0, (struct sockaddr *)&cli_addr, clilen);
}

int main(int argc, char const *argv[])
{
    socket_fd = socket(AF_INET, SOCK_DGRAM, 0);

    /* 如何实现信号驱动？

    针对套接字使用信号驱动式I/O（SIGIO），要求进程执行以下3个步骤：
        1. 建立SIGIO信号的信号处理函数；
        2. 设置套接字的属主，通常使用fcntl的F_SETOWN命令设置；
        3. 开启套接字的信号驱动式I/O，通常使用fcntl的F_SETFL命令打开O_ASYNC标志。
    */

    struct sigaction act;
    act.sa_flags = 0;
    act.sa_handler = do_sometime;
    sigaction(SIGIO, &act, NULL); // 设置信号处理函数，监听IO事件

    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));

    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(8888);
    servaddr.sin_addr.s_addr = INADDR_ANY;

    //设置将要在socket_fd上接收SIGIO的进程，这样当 socket_fd 触发io事件时，内核才知道通知哪个进程
    fcntl(socket_fd, F_SETOWN, getpid());

    int flags = fcntl(socket_fd, F_GETFL, 0);
    flags |= O_NONBLOCK;
    flags |= O_ASYNC;
    fcntl(socket_fd, F_SETFL, flags);

    bind(socket_fd, (struct sockaddr *)&servaddr, sizeof(servaddr));
    while (1)
        sleep(1);

    close(socket_fd);

    return 0;
}

/* Tcp / Udp

以下条件均会导致对一个 TCP 套接字产生SIGIO信号：
    * 监听套接字上某个连接请求已经完成；
    * 某个断连请求已经发起；
    * 某个断连请求已经完成；
    * 某个连接之半已经关闭；
    * 数据到达套接字；
    * 数据已经从套接字发送走；
    * 发生某个异步错误。

在 UDP 套接字中，只有以下两个条件会产生SIGIO信号：
    * 数据报到达套接字；
    * 套接字上发生异步错误。

*/