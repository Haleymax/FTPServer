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


//处理连接命令
int FTPClient::do_connect(char *ip , struct sockaddr_in *serv_addr , int *sock_fd){
    //清空地址结构体
    bzero(serv_addr , sizeof(struct sockaddr_in));
    serv_addr->sin_family = AF_INET;

    //将点分十进制转换为网络字节序
    inet_pton(AF_INET , ip , &(serv_addr->sin_addr));
    serv_addr->sin_port = htons(PORT);

    //创建套接字
    *sock_fd = socket(AF_INET , SOCK_STREAM , 0);
    if (*sock_fd < 0)
    {
        perror("fail to create socket");
        return -1;
    }

    //连接服务器
    if(connect(*sock_fd , (sockaddr *)serv_addr , sizeof(struct sockaddr_in)) < 0){
        perror("fail to connect");
        return -2;
    }

    return 0;
    
}

//处理获取文件命令
int FTPClient::do_get(const char *src , const char *dest , int sock_fd){
    char *dest_file;    //目标文件路径
    struct stat stat_buf;  //文件状态
    char *p , buf[MAXBUFF];
    int fd , len;
    int res = -1;

    //判断文件路径是普通文件还是目录
    if(src[strlen(src) - 1] == '/'){   //最后以  '/'结尾代表是一个目录
        cout << "ERROR: source is a directory" << endl;
        return -2;
    }

    //获取目标文件名
    if((p = (char *)strrchr(src , '/')) != NULL){
        dest_file = p + 1;  // '/' 后面的文件名
    }else{
        dest_file = (char *)src;  //表示直接就是文件名
    }

    //打开本地文件
    if((fd = open(dest , O_WRONLY | O_CREAT | O_TRUNC , 0644)) < 0){  //以只写方式（O_WRONLY）打开它，如果文件已存在则将其截断为空（O_TRUNC），如果文件不存在则创建它（O_CREAT）
        perror("fail to open file");
        return -3;
    }


    //发送GET命令到服务器
    sprintf(buf,"GET %s\r\n",src);
    if (write(sock_fd , buf , strlen(buf)) < 0)
    {
        perror("fail to write");
        return -4;
    }

    //读取服务器响应
    if ((len = read(sock_fd , buf , MAXBUFF)) < 0)
    {
        perror("fail to read");
        return -5;
    }
    buf[len] = '\0';
    
    //判断服务器返回的信息是不是OK
    if (strncmp(buf , "OK" , 2) != 0)
    {
        cout << "ERROR: " << buf << endl;
        return -6;
    }

    //读取文件内容并写入本地文件
    while ((len = read(sock_fd , buf , MAXBUFF)) > 0)
    {
        if (write(fd , buf ,len) < 0)
        {
            perror("fail to write");
            return -7;
        }
        
    }

    close(fd);

    cout<< "File \" " << src << "\" has been download as \" " << dest_file << "\"" << endl;

    return 0;
    
}

//处理上传文件命令
int FTPClient::do_put(const char *src , const char *dest , int sock_fd){
    struct stat stat_buf;   //存储文件信息

    char *p , buf[MAXBUFF];
    int fd , len;
    int res = -1;

    //检查文件目录是否存在
    if(src == NULL || dest == NULL){
        cout << "ERROR : wrong command" << endl;
        return -1;
    }

    //获取本地文件状态
    if(stat(src , &stat_buf) < 0){
        perror("fail to get file stat");
        return -2;
    }

    //如果源文件路径的最后一个字符是/，则说明源文件不是普通文件，而是目录
    if(S_ISDIR(stat_buf.st_mode)){
        cout << "ERROR: source is a directory" << endl;
        return -3;
    }

    //打开本地文件
    if((fd = open(src , O_RDONLY)) < 0){
        perror("fail to open file");
        return -4;
    }

    //发送PUT命令到服务器
    sprintf(buf , "PUT %s\r\n",dest);
    if (write(sock_fd , buf , strlen(buf)) < 0)
    {
        perror("fail to write");
        return -5;
    }

    //读取服务器响应
    if((len = read(sock_fd , buf , MAXBUFF)) < 0){
        perror("fail to read");
        return -6;
    }
    buf[len] = '\0';

    //如果服务器响应不是OK，则打印错误信息并返回
    if(strncmp(buf , "OK" , 2) != 0){
        cout << "ERROR: " << buf << endl;
        return -7;
    }

    //读取本地文件内容并写入服务器
    while ((len = read(fd , buf , MAXBUFF)) > 0){
        if (write(sock_fd , buf , len) < 0){
            perror("fail to write");
            return -8;
        }  
    }

    close(fd);
    cout << "file \"" << src << "\" has been uploaded as \"" << dest << "\"" << endl;

    return 0;
    
}