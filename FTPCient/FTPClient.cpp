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


//将命令行分割为命令和参数
int FTPClient::split(struct str_command *command , char *cline){
    int i = 0 , p = 0;

    cline[strlen(cline) - 1] = '\0';   //去掉末尾的换行符号

    while (cline[p] != '\0' && (cline[p] == ' ' || cline[p] == '\t'))
    {
        p++;
    }

    while (cline[p] != '\0')
    {
        //分配内存存储参数
        if ((command->argv[i] == (char *)malloc(sizeof(char)*BUFFSIZE)) == NULL)
        {
            perror("fail to malloc");
            return -1;
        }

        //获取参数内容
        int j = 0;
        while (cline[p] != '\0' && cline[p] != ' ' && cline[p] != '\t')
        {
            command->argv[i][j++] = cline[p++];
        }
        command->argv[i][j] = '\0'; //添加字符串结束符号

        i++;

        while (cline[p] != '\0' && (cline[p] == ' ' || cline[p] == '\t'))
        {
            p++;
        }
        
    }
    
    command->argv[i] = NULL;  //参数的末尾以空结尾
    command->name = command->argv[0];   //字符串赋值

    return i;
}