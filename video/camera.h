#ifndef _CARMERA_H
#define _CARMERA_H
#define CAMERA_DEVICE "/dev/video0" 

#define CLEAR(x) memset (&(x), 0, sizeof (x))  
#define PIC_WIDTH 640//640
#define PIC_HEIGHT 480//480  
#define BUFFER_COUNT 10
#define QUALITY 75
#define JPEG_FILE "cam.jpeg"
#define RGB_FILE "cam.rgb"

#define YCbCrtoR(Y,Cb,Cr) (1000000*Y + 1370705*(Cr-128))/1000000

#define YCbCrtoG(Y,Cb,Cr) (1000000*Y - 337633*(Cb-128) - 698001*(Cr-128))/1000000

#define YCbCrtoB(Y,Cb,Cr) (1000000*Y + 1732446*(Cb-128))/1000000

#define PHSIZE 1024*60

//Qt图片信息
struct PIC {
    unsigned long len;  //图片长度
    char picbuf[PHSIZE]; //图片大小
}*pic;

//web图片信息
struct WEB {
    unsigned long len;
    char picbuf[PHSIZE];
}*web;

//pthread_rwlock_t jpeg_mutex;
pthread_mutex_t jpeg_mutex, web_mutex;
pthread_cond_t jpeg_cond, web_cond;

void camera_open_device();//打开设备
void camera_get_capability();//查看设备信息
void camera_get_format() ;//get the format
void camera_set_format() ;//set the format 
int camera_capture() ;//视频采集
void camera_request_buf();// 申请内存；
void camera_query_mmap_buf() ;//获得内存，实现内存映射；
int camera_yuyv_to_rgb() ;//yuyv格式转为rgb格式
int camera_rgb_to_jpeg() ;//rgb格式压缩为jpeg格式
void camera_video_save() ;//保存视频；
void uninit_device() ;//撤销内存映射
void camera_off() ; //关闭设备
#endif
