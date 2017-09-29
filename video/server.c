/*************************************************************************
	> File Name: server.c
	> Author: 
	> Mail: 
	> Created Time: Wed 12 Oct 2016 08:51:02 AM CST
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sqlite3.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/wait.h>
#include <pthread.h>

#include "camera.h"

#define SIZE 1024


//定义用户信息结构体
typedef struct
{
    int flag;
    char username[20];
    char password[20];
}USE;

//发送信息到Q客户端
int send_Qtclient(int connfd, USE *user)
{
    send(connfd, user, sizeof(USE), 0);
    bzero(user, sizeof(USE));

    return 0;
}

//注册函数
int do_register(USE *user)
{
    char buf[SIZE];
    char *errmsg;

    sqlite3 *db;
    if(0 != sqlite3_open("my.db", &db)) {
        fprintf(stderr, "sqlite3_open: %s\n", sqlite3_errmsg(db));
        exit(-1);
    }
    printf("sqlite3_open success!\n");
    
    sprintf(buf, "insert into user values('%s', '%s');", user->username, user->password);
    if(0 != sqlite3_exec(db, buf, NULL, NULL, &errmsg)) {
        fprintf(stderr, "sqlite3_exec: %s\n", errmsg);
        strcpy(user->username, "N");
    }
    else {
        strcpy(user->username, "R");
    }
    
    if(0 != sqlite3_close(db)) {
        fprintf(stderr, "sqlite_close: %s\n", sqlite3_errmsg(db));
        exit(-1);
    }

    return 0;
}

//登录函数
int do_login(USE *user)
{
    char buf[SIZE];
    char *errmsg;
    char **result;
    int nrow, ncolum;

    printf("---------login----------\n");
    printf("%s\n", user->username);
    printf("%s\n", user->password);
    
    sqlite3 *db;
    if(0 != sqlite3_open("my.db", &db)) {
        fprintf(stderr, "sqlite3_open: %s\n", sqlite3_errmsg(db));
        exit(-1);
    }
    printf("sqlite3_open success!\n");

    sprintf(buf, "select * from user where username='%s' and password='%s';", user->username, user->password);
    if(0 != sqlite3_get_table(db, buf, &result, &nrow, &ncolum, &errmsg)) {
        fprintf(stderr, "sqlite3_exec: %s\n", errmsg);
        exit(-1);
    } 
    
    if(nrow == 0) {
        strcpy(user->username, "N");
    } else {
        strcpy(user->username, "L");
    }

    if(0 != sqlite3_close(db)) {
        fprintf(stderr, "sqlite_close: %s\n", sqlite3_errmsg(db));
        exit(-1);
    }

    return 0;
}

//发送图片到Qt客户端
int do_video(int connfd)
{
    char picsize[20];
    while(1) {
        pthread_cond_wait(&jpeg_cond, &jpeg_mutex);
        bzero(picsize, sizeof(picsize));
        sprintf(picsize, "%ld", pic->len);
        send(connfd, picsize, sizeof(picsize), 0);
        send(connfd, pic->picbuf, pic->len, 0);  
        pthread_mutex_unlock(&jpeg_mutex);
    }
    return 0;
}

//关闭Qt线程
int do_closeQtclient(int tid)
{
#if 1
    pthread_cancel(tid);

    return 0;
#endif
}


//Qt客户端线程处理函数
void *Qtclient(void *arg)
{
    USE user;
    int connfd = *(int *)arg;
    int ret;
   
    //获得线程id
    pthread_t tid;
    tid = pthread_self();
    printf("tid=%ld\n", tid);

    while(1) {
        bzero(&user, sizeof(USE));
        ret = recv(connfd, &user, sizeof(USE), 0);
        if(ret == 0) {
            close(connfd);
            exit(0);
        }
        printf("username: %s\n", user.username);
        printf("password: %s\n", user.password);
        printf("user.flag: %d\n", user.flag);
        
        switch(user.flag) {
            case 1: do_login(&user); //登录
                    send_Qtclient(connfd, &user);
                break;
            case 2: do_register(&user); //注册
                    send_Qtclient(connfd, &user);
                break;
            case 3: do_video(connfd); //发送图片
                break;
            case 4: do_closeQtclient(tid); //关闭Qt线程
                break;
            default:
                break;
        }      
    }

    close(connfd);
}

//摄像头线程处理函数
void *pthread_cam(void)
{
    camera_open_device(); //打开摄像头
    camera_get_capability(); 
    camera_get_format(); //获得摄像头采集格式
    camera_set_format(); //设置摄像头采集格式
    camera_request_buf(); //申请内存
    camera_query_mmap_buf(); //映射内存
    camera_capture() ;
    while(1){
        camera_yuyv_to_rgb(); //yuyv转rgb
        camera_rgb_to_jpeg(); //rgb转jpeg
    }
    uninit_device(); 
    camera_off();
}

void *pthread_web()
{
    webserver();    
}

int main()
{
    //定义线程号
    pthread_t tid1, tid2, tid3;
    
    //开辟空间
    pic = (struct PIC *)malloc(60*1024);    
    web = (struct WEB *)malloc(60*1024);

    //摄像头线程
    if(0 != pthread_create(&tid1, NULL, (void *)pthread_cam, NULL)) {
        perror("pthread_create");
        exit(-1);
    }
    pthread_detach(tid1);//分离线程
    
    //web服务器线程
    if(0 != pthread_create(&tid2, NULL, (void *)pthread_web, NULL)) {
        perror("pthread_create");
        exit(-1);
    }
    pthread_detach(tid2);

    //创建套接字
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd < 0) {
        perror("sockfd");
        exit(-1);
    }
    printf("socket success!\n");

    struct sockaddr_in myaddr;
    bzero(&myaddr, sizeof(myaddr));
    myaddr.sin_family = AF_INET;
    myaddr.sin_port = htons(6666);
    myaddr.sin_addr.s_addr = htonl(INADDR_ANY);

    //端口复用
    int on = 1;
    if (0 > setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on))) {
        perror("setsockopt");
        exit(-1);    
    }

    //绑定ip和端口
    if(0 > bind(sockfd, (struct sockaddr *)&myaddr, sizeof(myaddr))) {
        perror("bind");
        exit(-1);
    }
    printf("bind-------------\n");


    //监听套接字
    if(0 > listen(sockfd, 1024)) {
        perror("listen");
        exit(-1);
    }
    printf("listen............\n");
    
    while(1) {
        struct sockaddr_in cliaddr;
        bzero(&cliaddr, sizeof(cliaddr));
        int addrlen = sizeof(cliaddr);
        
        //通信套接字
        int connfd = accept(sockfd, (struct sockaddr *)&cliaddr, &addrlen);
        if(connfd < 0) {
            perror("accept");
            exit(-1);
        }
        printf("accept........\n");
        
        //Qt客户端
        if(0 != pthread_create(&tid3, NULL, (void *)Qtclient, (void *)&connfd)) {
            perror("pthread_create");
            exit(1);
        }
        pthread_detach(tid3);
        
    }
    

    close(sockfd);
    return 0;
}
