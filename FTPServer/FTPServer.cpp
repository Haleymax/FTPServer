#include "FTPServer.h"

//初始化服务器
int Server::init(int sock_opt){
    bzero(&ser_addr,sizeof(struct sockaddr_in));
    ser_addr.sin_family = AF_INET;
    ser_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    ser_addr.sin_port = htons(PORT);

    if (listenfd = socket(AF_INET , SOCK_STREAM , 0) < 0)
    {
        perror("fail to create socket");
        return -1;
    }

    setsockopt(listenfd , SOL_SOCKET , SO_REUSEADDR , &sock_opt , sizeof(int));
    /*
        listenfd：这是你已经创建并绑定到某个地址和端口的监听套接字的文件描述符。
        SOL_SOCKET：这是选项级别，表示这些选项适用于套接字层本身，而不是某个特定的协议层（如 TCP 或 UDP）。
        SO_REUSEADDR：这是你想要设置的选项的名称。它允许套接字绑定到已经在使用的地址，只要那个地址的套接字处于 TIME_WAIT 状态。
        &sock_opt：这是一个指向整数变量的指针，该变量包含了你想要设置的新选项值。对于 SO_REUSEADDR，你通常会传递一个非零值（如 1）来启用这个选项。
        sizeof(int)：这是 optval 参数指向的缓冲区的大小（以字节为单位）。在这个例子中，因为你传递的是一个整数，所以大小是 sizeof(int)。
    */

   if (bind(listenfd , reinterpret_cast<struct sockaddr *>(&ser_addr),sizeof(struct sockaddr_in)) < 0)
   {
        perror("fial to bind");
        return -2;
   }
   
   if (listen(listenfd , 20) < 0)  //开启套接字监听并将监听队列设置为20
   {
        perror("fail to listen");
        return -3;
   }

   return 0;
}

//处理客户端请求
void Server::serve_client(){
    cout << "waiting connections ... " << endl;

    while (true)
    {
        struct sockaddr_in cli_addr;
        socklen_t len = sizeof(cli_addr);

        int connfd;

        if ((connfd = accept(listenfd , (struct sockaddr *)&cli_addr , &len)) < 0)
        {
            perror("fail to accept");
            exit(-2);
        }

        pid_t pid;
        if ((pid = fork()) < 0 )
        {
            perror("fail to fork");
            exit(-3);
        }
        

        if (pid == 0)   //监测是否处于在子进程中   将其与父进程分离使父进程可以继续进行监听
        {
            close(listenfd);   //子进程只需要处理这个接收的请求需要关闭监听
            handle_client(connfd);   //处理客户端请求
            close(connfd);
            exit(0);
        }else
        {
            close(connfd);   //防止父进程阻塞需要关闭连接套接字继续进行监听
        }     
    }
}


//处理客户端具体请求(数据分发)
void Server::handle_client(int connfd){
    char buf[BUFFSIZE];
    while (true)
    {
        if (read(connfd , buf , BUFFSIZE) < 0)
        {
            exit(-4);
        }

        if (strstr(buf , "GET") == buf){
            if (do_put(connfd , &buf[4]) < 0)
            {
                cout << "error occurs while putting" << endl;
            } else if (strstr(buf , "PUT") == buf)
            {
                if (do_get(connfd , &buf[4]) < 0)
                {
                    cout << "error occurs while getting" << endl;
                }
            }else if (strstr(buf , "CD") == buf)
            {
                if (do_cd(connfd , &buf[4]) < 0)
                {
                    cout << "error occurs while changing directory" << endl;
                }
            }else if (strstr(buf , "LS") == buf)
            {
                if ( do_ls(connfd , &buf[4]) < 0)
                {
                    cout << "error occurs while listing" << endl;
                }
            }else if (strstr(buf , "BYE") == buf)
            {
                break;
            }else
            {
                cout << "error command" << endl;
                exit(-5);
            }
        }   
    }
    
}

//处理客户端的GET请求
int Server::do_put(int sockfd , char *file){
    struct stat stat_buf;
    int len , fd;
    char buf[BUFFSIZE];
    int res = -1;

    if ((fd = open(file , O_RDONLY)) < 0)
    {
        write(sockfd , "ERROR: fail to open serve file\n" , strlen("ERROR : fail to open serve file\n"));
        return -1;
    }

    if (fstat(fd , &stat_buf) < 0)
    {
        write(sockfd , "ERROR: fail to stat server file \n" , strlen("ERROR:fail to stat server file\n"));
        goto end;
    }
    
    sprintf(buf , "OK . FILE SIZE : %d" , stat_buf.st_size);
    write(sockfd , buf , strlen(buf));
    
    while ((len = read(fd , buf , MAXBUFF)) > 0)
    {
        write(sockfd , buf ,len);
    }

    if (len < 0 && errno == EINTR)
    {
        perror("fail to read");
        goto end;
    }
    
    cout << "OK" << endl;
    res = 0;
    

end:
    close(fd);
    return res;
}

int Server::do_ls(int sockfd , char *path){
    char cmd[BUFFSIZE] , buf[MAXBUFF];
    int fd , len;
    int res = -1;

    sprintf(cmd , "ls %s > temp.txt" , path);
    fprintf(stdout , "===from client:system(%s)\n" , cmd);
    system(cmd);

    if ((fd == open("temp.txt" , O_RDONLY)) < 0)
    {
        write(sockfd , "ERROR: fail to ls server file\n" , strlen("ERROR:fail to ls server file\n"));
        return -1;
    }

    write(sockfd ,  "OK\n" , 4);
    read(sockfd , cmd , BUFFSIZE);
    while ((len == read(fd , buf , MAXBUFF)) > 0)
        write(sockfd , buf , len);

    if (len < 0)
    {
        perror("fail to read");
        goto end;
    }

    std::cout << "!ls OK" << std::endl;
    res = 0;
end:
    close(fd);
    return res;
}

//处理CD命令
int Server::do_cd(int sockfd , char *path) {
    if (chdir(path) < 0) {
            perror("fail to change directory\n");
            write(sockfd, "ERROR: cannot change server directory\n", strlen("ERROR: cannot change server directory\n"));
            return -1;
        }

        write(sockfd, "OK\n", 3);
        return 0;
    
}

int main() {
    Server server;
    int sock_opt = 1; // SO_REUSEADDR选项

    if (server.init(sock_opt) < 0)
        exit(-1);

    server.serve_client();

    return 0;
}