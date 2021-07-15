#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <iostream>
#include <string>
#include <signal.h>

#include <spdlog/spdlog.h>
#include "spdlog/cfg/env.h"

int connect_server(const char *server_ip, uint16_t port)
{
    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0)
    {
        printf("create socket error: %s(errno: %d)\n", strerror(errno), errno);
        exit(0);
    }
    struct sockaddr_in servaddr;

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);
    if (inet_pton(AF_INET, server_ip, &servaddr.sin_addr) <= 0)
    { //inet_pton是Linux下IP地址转换函数，将IP地址在“点分十进制”和“整数”之间转换
        printf("inet_pton error for %s\n", server_ip);
        exit(0);
    }

    if (connect(socket_fd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
    {
        printf("connect error: %s(errno: %d)\n", strerror(errno), errno);
        exit(0);
    }
    return socket_fd;
}
static volatile int keep_running = 1;
static bool sent_quit = false;
void intHandler(int dummy)
{
    keep_running = 0;
    spdlog::debug("intHandler.");
}

int main(int argc, const char **argv)
{
    spdlog::cfg::load_env_levels();
    signal(SIGINT, intHandler);
    spdlog::debug("started.");
    /* 
    int sockfd, n, rec_len;
    char recvline[4096], sendline[4096];
    char buf[4000];
    struct sockaddr_in servaddr;
 */
    if (argc != 3)
    {
        std::cout << "usage: " << argv[0] << " IP_ADDR PORT" << std::endl;
        exit(0);
    }
    int socket_fd = connect_server(argv[1], std::stoul(argv[2], 0, 10));
    struct timeval timeout = {3, 0}; //3s
    setsockopt(socket_fd, SOL_SOCKET, SO_SNDTIMEO, (const char *)&timeout, sizeof(timeout));
    setsockopt(socket_fd, SOL_SOCKET, SO_RCVTIMEO, (const char *)&timeout, sizeof(timeout));
    /// RECV
    int recv_buff_size = 4000;
    char recv_buff[recv_buff_size];
    int recv_len = recv(socket_fd, recv_buff, recv_buff_size, 0);
    if (recv_len <= 0)
    {
        spdlog::error("over: maybe server or session is closed.");
        close(socket_fd);

        exit(1);
    }
    recv_buff[recv_len] = '\0';
    printf("%s", recv_buff);

    while (keep_running)
    {
        int send_buff_size = 4000;
        char *send_buff = NULL;
        size_t send_len;
        int gret = getline(&send_buff, &send_len, stdin);
        if (gret == -1)
        {
            spdlog::info("invalid line");
            close(socket_fd);

            exit(1);
        }
        send_len = strlen(send_buff);
        spdlog::debug("sending...");
        //fgets(send_buff, 4096, stdin);
        int status = send(socket_fd, send_buff, send_len, 0);
        spdlog::debug("status = {}", status);
        if (status < 0)
        {
            spdlog::info("maybe server or session is closed.");
            printf("send msg error: %s(errno: %d)\n", strerror(errno), errno);
            close(socket_fd);

            exit(0);
        }
        spdlog::debug("send {} bytes to server.", send_len);

        /// RECV
        recv_len = recv(socket_fd, recv_buff, recv_buff_size, 0);
        if (recv_len <= 0)
        {
            spdlog::error("over: maybe server or session is closed.");
            close(socket_fd);

            exit(1);
        }
        recv_buff[recv_len] = '\0';
        printf("%s", recv_buff);

        // 判断退出
        if (strcmp(send_buff, "quit\n") == 0)
        {
            spdlog::debug("quiting...", send_len);
            keep_running = false;
            sent_quit = true;
        }
    }
    if (!sent_quit)
    {
        send(socket_fd, "quit\n", 6, 0);
        spdlog::debug("over.");
        sent_quit = true;
    }
    close(socket_fd);
}