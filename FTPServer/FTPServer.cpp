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