#include "FTPClient.h"

using namespace std;

//构造函数
FTPClient::FTPClient(/* args */)
{
    //初始化套接字
    sock_fd = -1;
    memset(&serv_addr , 0 , sizeof(serv_addr));   //服务器信息
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
}

FTPClient::~FTPClient()
{
    if (sock_fd != -1)
    {
        close(sock_fd);
    }
    
}

void FTPClient::run(){
    char cline[MAXBUFF];   //存储用户输入的命令行
    struct str_command command;   //命令

    cout << "myftp$: ";
    fflush(stdout);

    while (fgets(cline , MAXBUFF , stdin) != NULL)
    {
        if (split(&command , cline) < 0)
        {
            exit(EXIT_FAILURE);
        }
        
        //根据命令调用相应的函数
        if (strcasecmp(command.name , "get") == 0)
        {
            if (do_get(command.argv[1] , command.argv[2] , sock_fd) < 0)
            {
                exit(-2);
            }
        } else if (strcasecmp(command.name , "put") == 0)
        {
            if (do_put(command.argv[1] , command.argv[2] , sock_fd) < 0)
            {
                exit(-3);
            }
        } else if (strcasecmp(command.name , "cd") == 0)
        {
            if (do_cd(command.argv[1]) < 0)
            {
                exit(-4);
            }
        } else if (strcasecmp(command.name , "ls") == 0)
        {
            if (do_ls(command.argv[1]) < 0)
            {
                exit(-5);
            }
        } else if (strcasecmp(command.name , "connect") == 0)
        {
            if (do_connect(command.argv[1] , &serv_addr , &sock_fd) < 0)
            {
                exit(-6);
            }
        } else if (strcasecmp(command.name , "!ls") == 0)
        {
            if (do_ser_ls(command.argv[1],sock_fd) < 0)
            {
                exit(-9);
            }
        } else if (strcasecmp(command.name , "!cd") == 0)
        {
            if (do_ser_cd(command.argv[1],sock_fd) < 0)
            {
                exit(-10);
            }
        } else if (strcasecmp(command.name , "quit") == 0)
        {
            if (do_quit(sock_fd) < 0)
            {
                exit(-8);
            }
        } else {
            cout << "ERROR : wrong command" << endl;
            cout << "Usage : command argv 1 argv 2 , ... " << endl;
        }

        //清空命令
        memset(&command , 0 , sizeof(struct str_command));
        cout << "myftp$: ";
        fflush(stdout);
    }

    //关闭套接字
    if(sock_fd != -1){
        if(close(sock_fd) < 0){
            perror("fail to close");
            exit(-7);
        }
    }

}