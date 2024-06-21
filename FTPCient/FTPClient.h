#ifndef FTPCLIENT_H
#define FTPCLIENT_H

#include "iostream"
#include "cstring"
#include "cstdio"
#include "cstdlib"
#include "unistd.h"
#include "sys/socket.h"
#include "netinet/in.h"
#include "arpa/inet.h"
#include "sys/stat.h"
#include "fcntl.h"

#define PORT 8082
#define BUFFSIZE 32
#define MAXBUFF 128

//FTP客户端
class FTPClient
{
private:
    int sock_fd;
    struct sockaddr_in serv_addr;

    //存储命令的结构体
    struct str_command
    {
        char *name;     //命令名称
        char *argv[10]; //命令参数数组
    };
    
public:
    FTPClient();   //构造初始化客户端

    ~FTPClient();

    void run();     //启动FTP客户端


private:
    //将命令行分割为命令和其他参数
    int split(struct str_command *command , char *cline);   

    //处理连接
    int do_connect(char *ip , struct sockaddr_in *serv_addr , int *sock_fd);  

    //处理获取文件命令
    int do_get(const char *src , const char *dest , int sock_fd);

    //处理上传文件命令
    int do_put(const char *src , const char *dest , int sock_fd);
    
    //处理改变目录的命令
    int do_cd(char *path);

    //处理列出文件命令
    int do_ls(char *path);

    //处理服务器端列出目录内容命令
    int do_ser_ls(char *path , int sockfd);

    //处理服务端改变目录命令
    int do_ser_cd(char *path , int sockfd);

    //处理退出命令
    int do_quit(int sock_fd);
};

#endif
