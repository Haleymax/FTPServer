#pragma once
#include "cstring"
#include "cstdio"
#include "unistd.h"
#include "sys/socket.h"
#include "arpa/inet.h"
#include "sys/stat.h"
#include "fcntl.h"
#include "cerrno"

#define PORT 8082
#define BUFFSIZE 64
#define MAXBUFF 128

using namespace std;

class Server
{
private:
    struct sockaddr_in ser_addr;
    int listenfd;    //服务器监听套接字
public:
    Server() : listenfd(-1){}
    

    //初始化服务器
    int init(int sock_opt);

    //处理客户端的PUT请求
    int do_get(int sockfd , char *file);

    //处理客户端GET请求
    int do_get(int sockfd , char *file);

    //处理客户端具体请求
    void handle_client(int connfd);

    //处理客户端LS请求
    void  do_ls(int sockfd , char *path);
};


