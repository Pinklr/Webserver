#include <stdio.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

struct sockInfo {

    int fd;
    
    struct sockaddr_in addr;
    pthread_t tid;

    /* data */
};

struct sockInfo sockInfos[128];


void * working(void * arg) {
    //子线程和客户端通信 cfd 客户端信息 线程号
    struct sockInfo * info = (struct sockInfo *) arg;
    int cfd = info->fd;

     char cliIP[16];
            inet_ntop(AF_INET, &info->addr.sin_addr.s_addr, cliIP, sizeof(cliIP));
            unsigned short cliPort = ntohs(info->addr.sin_port);
            printf("client IP is %s , port is %d\n", cliIP, cliPort);

            //接受客户端发来的信息

            char recvBuf[1024];
            while(1) {
                int len = read(cfd, &recvBuf, sizeof(recvBuf));
                

                if(len == -1) {
                    perror("read");
                    exit(-1);
                }else if(len > 0) {
                    printf("recv client is %s\n", recvBuf);


                }else if(len == 0){
                    printf("client is closed...\n");
                    break;

                }

                write(cfd, recvBuf, strlen(recvBuf)+1);

            }

            close(cfd);
            exit(0);
        


}


int main() {
     // 创建socket
    int lfd = socket(PF_INET, SOCK_STREAM, 0);
    if(lfd == -1) {
        perror("socket");
        exit(-1);

    }
    struct sockaddr_in saddr;
    saddr.sin_addr.s_addr = INADDR_ANY;
    saddr.sin_port = htons(9999);
    saddr.sin_family = AF_INET;


    

    //绑定
    int ret = bind(lfd, (struct sockaddr *)&saddr, sizeof(saddr) );
    if(ret == -1) {
        perror("bind");
        exit(0);
    }


    //监听
    ret = listen(lfd, 128);
    if(ret == -1) {
        perror("listen");
        exit(-1);
    }

    //初始化sockInfo
    int max = sizeof(sockInfos) / sizeof(sockInfos[0]);
    for(int i = 0; i < max; i++) {
        sockInfos[i].fd = -1;
        sockInfos[i].tid = -1;
                
    }
    

    while(1) { //循环等待客户端的连接
        struct sockaddr_in cliaddr;
        int len = sizeof( cliaddr);
        
        //接受连接
        int cfd = accept(lfd, (struct sockaddr *)&cliaddr, &len);

        //创建子线程
        struct sockInfo * pinfo ;
        for(int i = 0; i < max; i++) {
            if(sockInfos[i].fd == -1) {
                pinfo = &sockInfos[i];
                break;
            }else if(i == max - 1){
                sleep(1);
                i = -1;
            }
        }

        
        pinfo->fd = cfd;
        memcpy(&pinfo->addr, &cliaddr, len);
    

        pthread_create(&pinfo->tid, NULL, working, pinfo);
        pthread_detach(pinfo->tid);


    }
}