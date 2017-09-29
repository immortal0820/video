/********************************************************
*   Copyright (C) 2016 All rights reserved.
*   
*   Filename:server.c
*   Author  :M
*   Date    :2016-12-08
*   Describe:
*
********************************************************/
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <pthread.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include "camera.h"

#define BACKLOG 512
#define MAX_BUF_LEN 40960
#define BYTE 1024*4
#define SIZE 20
#define REPORT 7777

typedef struct {
        int flag;
        char username[SIZE];
        char password[SIZE];
}USER;
USER *user;
static int stop = 1;

void errormsg(int connfd, char *errnmsg)
{
    FILE *fp;
    int ret;
    int filesize;
    struct stat filestat;
    char EHTML[SIZE] = {0};
    char response[MAX_BUF_LEN];
    char status[] = "HTTP/1.0 200 OK\r\n";
    char response_header[] = "Server: fsc100\r\nContent-Type: text/html\r\n\r\n";
    char response_text[BYTE];
    if(strncmp(errnmsg, "REGISTER", 8)) {
        strcpy(EHTML, "errno0.html");	
    } else if (strncmp(errnmsg, "LOGIN", 5)) {	
        strcpy(EHTML, "errno1.html");
    }

    if((fp = fopen(EHTML, "rb")) == NULL) {        
        perror("fopen");
        return;
    }        
    if((ret = stat(EHTML, &filestat)) == -1){
        perror("server->stat");
        return;
    }
    filesize = filestat.st_size;               
    bzero(response_text,sizeof(response_text));
    ret = fread(response_text, filesize, 1, fp);
    if(1 != ret) {
        perror("fread");
        fclose(fp);
        return;
    }
    memset(response, 0, sizeof(response));
    strcat(response, status);
    strcat(response, response_header);
    strcat(response, response_text);
    ret = write(connfd, response, strlen(response));
    if (ret == -1) {
        perror("server->write");
        return;
    }

}

char *usermsg(char *request, char *repasswd) 
{
    int psdlen, i = 0;
    char renter[SIZE] = {0};                
    char *userdata;
    char *remsg = "TRUE";
    user =(USER *)malloc(sizeof(USER));
    memset(user, 0, sizeof(user));
    userdata = strstr(request, "username=");
    if(userdata == NULL) {
        printf("login failed\n");
        return NULL;
    }      
    userdata = userdata + 9;
    memset(user->username, 0, sizeof(user->username));
    while(*userdata != '&'){
        user->username[i ++] = *userdata;
        userdata ++;
    }
    i = 0;
    userdata = userdata + 10;
    memset(user->password, 0, sizeof(user->password));
    while(*userdata != '&'){
        user->password[i ++] = *userdata;
        userdata ++;
    }        
    if(strncmp(repasswd, "do_register", 11) == 0) {
        i = 0;
        userdata = userdata + 19;
        while(*userdata != '&'){
            renter[i ++] = *userdata;
            userdata ++;
        }                                
        psdlen = strlen(user->password);
        if(user->password != NULL && renter != NULL) {
            if(strncmp(user->password, renter, psdlen) != 0) {
                return NULL;
            } else {
                return remsg; 
            }
        }else {
            return NULL;
        }   
    }
}

void video_rgs(int connfd)
{    
    FILE *fp;
    int ret;
    int filesize;
    struct stat filestat;
    char response[MAX_BUF_LEN];
    char status[] = "HTTP/1.0 200 OK\r\n";
    char response_header[] = "Server: fsc100\r\nContent-Type: text/html\r\n\r\n";
    char response_text[BYTE];
    if((fp = fopen("regis.html", "rb")) == NULL) {        
        perror("fopen");
        return;
    }        
    if((ret = stat("regis.html", &filestat)) == -1){
        perror("server->stat");
        return;
    }
    filesize = filestat.st_size;               
    bzero(response_text,sizeof(response_text));
    ret = fread(response_text, filesize, 1, fp);
    if(1 != ret) {
        perror("fread");
        fclose(fp);
        return;
    }
    memset(response, 0, sizeof(response));
    strcat(response, status);
    strcat(response, response_header);
    strcat(response, response_text);
    ret = write(connfd, response, strlen(response));
    if (ret == -1) {
        perror("server->write");
        return;
    }
}

void video_log(int connfd)
{
    int ret;               
    FILE *fpv;
    int filesize;
    char response[MAX_BUF_LEN];
    struct stat filestat;
    char status[] = "HTTP/1.0 200 OK\r\n"; 
    char response_header[] = "Server: fsc100\r\nContent-Type: text/html\r\n\r\n";
    char response_text[BYTE];                      
    if((fpv = fopen("login.html", "rb")) == NULL) {        
        perror("fopen");
        return;
    }        
    if((ret = stat("login.html", &filestat)) == -1){
        perror("server->stat");
        return;
    }
    filesize = filestat.st_size;               
    bzero(response_text,sizeof(response_text));
    ret = fread(response_text, filesize, 1, fpv);
    if(1 != ret) {
        perror("fread");
        fclose(fpv);
        return;
    }
    memset(response, 0, sizeof(response));
    strcat(response, status);
    strcat(response, response_header);
    strcat(response, response_text);
    ret = write(connfd, response, strlen(response));    
    if(-1 == ret) {
        perror("Server->write\n");
        fclose(fpv);
        return;
    }
    fclose(fpv);    	
}

void video_ind(int connfd)
{

    FILE *fp;
    int ret;
    int filesize;
    struct stat filestat;
    char response[MAX_BUF_LEN];
    char status[] = "HTTP/1.0 200 OK\r\n";
    char response_header[] = "Server: fsc100\r\nContent-Type: text/html\r\n\r\n";
    char response_text[BYTE];
    if((fp = fopen("index.html", "rb")) == NULL) {        
        perror("fopen");
        return;
    }        
    if((ret = stat("index.html", &filestat)) == -1){
        perror("server->stat");
        return;
    }
    filesize = filestat.st_size;               
    bzero(response_text,sizeof(response_text));
    ret = fread(response_text, filesize, 1, fp);
    if(1 != ret) {
        perror("fread");
        fclose(fp);
        return;
    }
    memset(response, 0, sizeof(response));
    strcat(response, status);
    strcat(response, response_header);
    strcat(response, response_text);
    ret = write(connfd, response, strlen(response));
    if (ret == -1) {
        perror("server->write");
        return;
    }
}

void video_sur(int connfd)
{        
    int ret = 0;
    int count;
    char response[MAX_BUF_LEN];
    char status[] = "HTTP/1.0 200 OK\r\n";
    char response_header[] = "Server: fsc100\r\nContent-Type: image/jpeg\r\nContent-Length: ";
    char length[32] = {0};              

    memset(response, 0, sizeof(response));
    strcat(response, status);
    strcat(response, response_header);
    snprintf(length, sizeof(length), "%ld", web->len);
    strcat(response, length);
    strcat(response, "\r\n\r\n");
    ret = write(connfd, response, strlen(response));
    if (ret < 0) {
        perror("server->write");
        return;
    }
    count = 0;
    pthread_mutex_lock( &web_mutex);	
    while (count < web->len) {
        ret = write(connfd, web->picbuf + count, web->len - count);
        if (ret < 0) {
            perror("server->write");
            return;
        } else if(ret == 0) {
            //sleep(0.05);
            continue;
        } else {
            count += ret;
        }
    }
    pthread_mutex_unlock(&web_mutex);
}

void inter_http(int connfd)
{
    int ret;
    char *getmsg = NULL;
    char repasswd[SIZE] = {0};
    char ERRNO[SIZE] = {0};
    char request[MAX_BUF_LEN];
    char response[MAX_BUF_LEN];

    memset(request, 0, sizeof(request));
    ret = read(connfd, request, MAX_BUF_LEN);
    if (ret <= 0) {
        if (ret == 0)
        printf("server->read: end-of-file\n");
        else 
        perror("server->read");
        return; 
    }
    if(strstr(request, "GET /login.html") != NULL) {
        video_log(connfd);
    }
    #if 1       
    if(strstr(request, "register=REGISTER") != NULL) {
        video_rgs(connfd);                     
    }
    if(strstr(request, "do_register=LOGIN") != NULL) {  
        strcpy(repasswd, "do_register");
        getmsg =  usermsg(request, repasswd);
        if((user->username != NULL) && (user->password != NULL) && (getmsg == "TRUE")) {
            do_register(user);
            if(strncmp(user->username, "R", 1) == 0){
                printf("REGISTER SUCCESS!\n");
                video_ind(connfd);              
            } else if(strncmp(user->username, "N", 1) == 0) {
                strcpy(ERRNO, "REGISTER");
                errormsg(connfd, ERRNO);
            }
        } else {
            strcpy(ERRNO, "REGISTER");
            errormsg(connfd, ERRNO);
        }
        free(user);
    }		
    if(strstr(request, "login=LOGIN") != NULL) {
        strcpy(repasswd, "login");                
        usermsg(request, repasswd);                              
        if((user->username != NULL) && (user->password != NULL)) {
            do_login(user);        
            if(strncmp(user->username, "L", 1) == 0){
                printf("LOGIN SUCCESS!\n");
                video_ind(connfd);               
            } else if(strncmp(user->username, "N", 1) == 0) {
                strcpy(ERRNO, "LOGIN");
                errormsg(connfd, ERRNO);
            }
        } else {
            strcpy(ERRNO, "LOGIN");
            errormsg(connfd, ERRNO);
        }
        free(user);                                   
    }
    if(strstr(request, "return=RETURN") != NULL) {
        video_log(connfd);
    }
    if(strstr(request, "GET /?action=snapshot") != NULL) {
        if((stop%2) != 0) {
            video_sur(connfd);
        }
    }
    if(strstr(request, "POST /index.html?StopImg()") != NULL){                
        stop++;
    } 
#endif        
}

int sok_listen()                                                
{                                                  //socket & bind & listen
    int socket_fd; 
    unsigned short port;

    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(-1 == socket_fd) {
        perror("socket");
        exit(EXIT_FAILURE);
    }
    printf("socket success!\n"); 

    struct sockaddr_in my_addr;
    bzero(&my_addr, sizeof(my_addr));
    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(REPORT);
    my_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    int on = 1;
    if (0 > setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
 
    if(-1 == bind(socket_fd, (struct sockaddr*)&my_addr, 
        sizeof(my_addr))) {
        perror("bind");
        exit(EXIT_FAILURE);
    }
    printf("bind success\n");

    if(-1 == listen(socket_fd, BACKLOG)) {
        perror("listen");
        exit(EXIT_FAILURE);
    }
 printf("listen .......\n");
 return socket_fd;
}

int do_accept(int socket_fd)
{                                                        //accept
    int do_connfd;  
    char cli_ip[INET_ADDRSTRLEN] = "";  //用于保存客户端IP地址
    struct sockaddr_in cliaddr;
    bzero(&cliaddr, sizeof(cliaddr));
    int addrlen = sizeof(cliaddr);

    do_connfd = accept(socket_fd, (struct sockaddr *)&cliaddr, &addrlen);
    return do_connfd;
}
//===============================================================  
// 语法格式：    void main(void)  
// 实现功能：    主函数，建立一个TCP并发服务器  
// 入口参数：    无  
// 出口参数：    无  
//===============================================================
int webserver()                      
{      
    int sockfd = 0;
    int connfd = 0;

    sockfd = sok_listen();          //listen ......
    while(1) {                
        connfd = do_accept(sockfd);
        if(connfd < 0){                        
            printf("accept this time");  
            continue;                        
        } 
        if(connfd > 0) {
            inter_http(connfd);					
        }
        close(connfd);
    }

    close(sockfd);
    return 0;
} 
