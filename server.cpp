#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <spdlog/spdlog.h>
#include "spdlog/cfg/env.h"

#include "unix.h"

#define BUGGY_DEBUG

#include "debug.h"

int new_socket()
{
    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd == -1)
    {
        printf("create socket error: %s(errno: %d)\n", strerror(errno), errno);
        exit(0);
    }
    return socket_fd;
}
int new_server(uint16_t port)
{

    int socket_fd = new_socket();

    struct sockaddr_in servaddr;

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY); //IP地址设置成INADDR_ANY,让系统自动获取本机的IP地址。
    servaddr.sin_port = htons(port);              //设置的端口为DEFAULT_PORT

    //将本地地址绑定到所创建的套接字上
    if (bind(socket_fd, (struct sockaddr *)&servaddr, sizeof(servaddr)) == -1)
    {
        printf("bind socket error: %s(errno: %d)\n", strerror(errno), errno);
        exit(0);
    }
    //开始监听是否有客户端连接
    if (listen(socket_fd, 10) == -1)
    {
        printf("listen socket error: %s(errno: %d)\n", strerror(errno), errno);
        exit(0);
    }
    return socket_fd;
}
static volatile int keep_running = 1;
static volatile int socket_fd;
void intHandler(int dummy)
{
    keep_running = 0;
    close(socket_fd);
    spdlog::info("interupt.");
    exit(1);
}

int main(int argc, char **argv)
{
    spdlog::cfg::load_env_levels();
    signal(SIGINT, intHandler);
    
    int port = 12333;
    if(argc == 2 && argv[1]){
        port = atoi(argv[1]);
    }
    socket_fd = new_server(port);
    spdlog::info("Listening on {}", port);
loadProg:
    int bg_infd, bg_outfd, bg_errfd;
    bgjob = spawn_background_command("./ecomm", &bg_infd, &bg_outfd, &bg_errfd);
    if (bgjob < 0)
    {
        spdlog::error("failed to start program.");
        exit(1);
    }
    usleep(300*1000);
    while (keep_running)
    {
        struct sockaddr client_addr;
        struct sockaddr_in *ipvv4_addr = (struct sockaddr_in *)&client_addr;
        socklen_t addr_len;
        int connect_fd = accept(socket_fd, &client_addr, &addr_len);
        if (connect_fd == -1)
        {
            spdlog::error("accept socket error: {}(errno: {})", strerror(errno), errno);
            exit(1);
        }
        spdlog::info("connected to {}:{}", inet_ntoa(ipvv4_addr->sin_addr), (int)ntohs(ipvv4_addr->sin_port));
        /// READ for welcome

        int bg_buff_size = 4000;
        char bg_ret[bg_buff_size];

        /// READ
        spdlog::debug("reading...");
        int bg_ret_size = read(bg_outfd, bg_ret, bg_buff_size);
        spdlog::debug("read {} bytes from bgjob.", bg_ret_size);
        if (bg_ret_size == 0)
        {
            //spdlog::error("bgjob over!");
            spdlog::info("session over.");
            close(connect_fd);
            goto loadProg;
        }
        /// SEND
        int send_ret = send(connect_fd, bg_ret, bg_ret_size, 0);
        if (send_ret == -1)
        {
            spdlog::error("send error");
            close(connect_fd);
            spdlog::info("disconnected from {}:{}", inet_ntoa(ipvv4_addr->sin_addr), (int)ntohs(ipvv4_addr->sin_port));
            //exit(0);
        }
        spdlog::debug("send {} bytes to client.", bg_ret_size);
        while (fd_is_valid(connect_fd))
        {

            /// RECV
            int recv_buff_size = 4000;
            char recv_buff[recv_buff_size];
            int recv_len = recv(connect_fd, recv_buff, recv_buff_size, 0);
            spdlog::debug("receive {} bytes from client.", recv_len);
            print_buffer(recv_buff, recv_len);
            recv_buff[recv_len] = '\0';

            /// WRITE
            write(bg_infd, recv_buff, recv_len);
            spdlog::debug("write {} bytes to bgjob.", recv_len);
            usleep(300 * 1000);
            /// READ
            spdlog::debug("reading...");
            int bg_ret_size = read(bg_outfd, bg_ret, bg_buff_size);
            spdlog::debug("read {} bytes from bgjob.", bg_ret_size);
            if (bg_ret_size == 0)
            {
                spdlog::info("session over.");
                close(connect_fd);
                goto loadProg;
            }
            /// SEND
            int send_ret = send(connect_fd, bg_ret, bg_ret_size, 0);
            if (send_ret == -1)
            {
                spdlog::error("send error");
                close(connect_fd);
                spdlog::info("disconnected from {}:{}", inet_ntoa(ipvv4_addr->sin_addr), (int)ntohs(ipvv4_addr->sin_port));
                //exit(0);
            }
            spdlog::debug("send {} bytes to client.", bg_ret_size);
        }
        close(connect_fd);
        spdlog::info("disconnected from {}:{}", inet_ntoa(ipvv4_addr->sin_addr), (int)ntohs(ipvv4_addr->sin_port));
    }
    close(socket_fd);
}