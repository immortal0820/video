/*************************************************************************
	> File Name: camera.c
	> Author:LLCC
	> Mail:  lluoc@hqyj.com
	> Created Time: 2016年12月14日 星期三 15时49分59秒
 ************************************************************************/

#include<stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>  
#include <string.h>   
#include <assert.h>  
#include <getopt.h>             
#include <fcntl.h>              
#include <unistd.h>  
#include <errno.h>  
#include <malloc.h>  
#include <sys/stat.h>  
#include <sys/types.h>  
#include <sys/time.h>  
#include <sys/mman.h>  
#include <sys/ioctl.h>  
#include <linux/fb.h>
#include <asm/types.h>          
#include <linux/videodev2.h>  
#include <signal.h>
#include <setjmp.h>
#include <jpeglib.h>
#include"camera.h"


//MMAP ，定义一个结构体来映射每个缓冲帧;
typedef struct VideoBuffer { 
    void *start;    //视频缓冲区的起始地址
    size_t length;  
} VideoBuffer;


struct pic
{
    unsigned char pic_buf[PIC_WIDTH*PIC_HEIGHT*3];//视频文件缓冲区
    unsigned int pic_len;//视频文件大小

}PIC;
/************************************************************
struct jpeg{
    unsigned char* outbuffer  ;// 输出图像数据缓冲区
    unsigned long buffersize  ;// 输出图像缩后的图像大小
}JPEG;
*************************************************************/
VideoBuffer *buffers ; 
struct v4l2_capability cap; //查询视频能力
struct v4l2_fmtdesc fmt;
struct v4l2_format tv4l2_format;
struct v4l2_requestbuffers req ;
struct v4l2_buffer buf ;
int fd ;

void camera_open_device()//打开设备；
{
    fd = open(CAMERA_DEVICE, O_RDWR);//打开设备文件,阻塞模式
    if (fd < 0) {
                printf("Open %s failed\n", CAMERA_DEVICE);
                return;
    }
    printf("open device ok\n");

}


void camera_get_capability()//查看设备信息
{
    int ret = ioctl(fd, VIDIOC_QUERYCAP, &cap);//查看设备功能
    if (ret < 0) {
        printf("VIDIOC_QUERYCAP failed (%d)\n", ret);
        return;

    }
    printf("Capability Informations:\n");
    printf(" driver: %s\n", cap.driver);
    printf(" card: %s\n", cap.card);
    printf(" bus_info: %s\n", cap.bus_info);
    printf(" version: %08X\n", cap.version);
    printf(" capabilities: %08x\n\n", cap.capabilities);
}

void camera_get_format(){//获取视频设备支持的视频格式；
 int ret ;
 fmt.index = 0 ;
 fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE ;
 printf("Support format:/n");
 while(ioctl(fd,VIDIOC_ENUM_FMT,&fmt)!=-1){

    printf("/t%d.%s/n\n",fmt.index+1,fmt.description);
    fmt.index++;

 }
}

void camera_set_format(){
        int ret ;
        CLEAR(tv4l2_format) ;
        tv4l2_format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        tv4l2_format.fmt.pix.width = PIC_WIDTH ;//帧宽，单位像素
        tv4l2_format.fmt.pix.height = PIC_HEIGHT;// 帧高，单位像素
        tv4l2_format.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;//帧格式
        tv4l2_format.fmt.pix.field = V4L2_FIELD_INTERLACED;
        
        ret = ioctl(fd,VIDIOC_S_FMT,&tv4l2_format) ;
        if(ret < 0){
                printf("Set format of video failed1\n") ;
                perror("set format") ;
                return ;   
        }
        printf(" type: %d\n", tv4l2_format.type);
        printf(" width: %d\n", tv4l2_format.fmt.pix.width);
        printf(" height: %d\n", tv4l2_format.fmt.pix.height);
}


void camera_request_buf(){
    CLEAR(req) ;
    int ret ;
    req.count = BUFFER_COUNT;//申请缓冲区的个数
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;// 缓冲帧数据格式
    req.memory = V4L2_MEMORY_MMAP;// 区别内存映射or用户指针方式
    ret = ioctl(fd,VIDIOC_REQBUFS,&req) ;
    if(ret<0){
        printf("request buf failed!\n") ;
        perror("request buf") ;
        return ;
    }
    printf("request buf success!\n") ;
    return ;
}
/**************************映射****************************/
void camera_query_mmap_buf(){
    buffers = calloc(req.count, sizeof(*buffers)) ;
    CLEAR(buf) ;
    int ret ;
    int i ;
    for(i=0;i<req.count;i++){
        buf.index = i ;//要投放到内核空间视频缓冲区的编号
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP ;
        ret = ioctl(fd,VIDIOC_QUERYBUF,&buf) ;//取得内存缓冲区到信息
        if(ret<0){
            printf("querybuf failed!\n") ;
            perror("query buf") ;
            return ;
        }
        printf("---------------req buf----------------------\n") ;
        buffers[i].length = buf.length ;
        // 把内核空间缓冲区映射到用户空间缓冲区;
        buffers[i].start  = (char *)mmap(0,buf.length,PROT_READ|PROT_WRITE,MAP_SHARED,fd,buf.m.offset) ;

        if(buffers[i].start == MAP_FAILED){
            printf("mmap failed!\n") ;
            perror("mmap") ;
            return ;
        }
        ret = ioctl(fd,VIDIOC_QBUF,&buf) ;
        if(ret<0){
            printf("VIDIOC_QBUF failed!\n") ;
            perror("VIDIOC_QBUF") ;
            return ;
        }
    }
    printf("VIDIOC_QUERYBUF success!\n") ;
    printf("VIDIOC_QBUF success!\n") ;
    return ;
}

int camera_yuyv_to_rgb(){
    int i,j;
    int ret;
    unsigned char y1,y2,u,v;
    int r1,g1,b1,r2,g2,b2;

    CLEAR(PIC.pic_buf) ;
   ret = ioctl(fd , VIDIOC_DQBUF, &buf);//出队，获取数据 
    if(ret<0){
        printf("-----1------VIDIOC_DQBUF failed!---yuyv-to-rgb-------\n") ;
        perror("-----1------VIDIOC_DQBUF");
        return -1 ;
    }
    //printf("VIDIOC_DQBUF success!\n") ;

    char *ptr = buffers[buf.index].start ; //yuyv;
    ret = ioctl(fd,VIDIOC_QBUF,&buf) ;
    if(ret<0){
        printf("-------2------VIDIOC_QBUF failed!\n") ;
        perror("-------2------VIDIOC_QBUF");
        return -1;
    }

    char *rgb = PIC.pic_buf ;//rgb;

    for(i=0;i<PIC_WIDTH*PIC_HEIGHT/2;i++){
        //yuyv的4个字节；
        y1 = *ptr ++ ;
        u = *ptr ++ ;
        y2 = *ptr ++ ;
        v = *ptr ++ ;
        
        //rgb的4个字节；
        r1 = YCbCrtoR(y1,u,v);
        g1 = YCbCrtoG(y1,u,v);
        b1 = YCbCrtoB(y1,u,v);
        
        r2 = YCbCrtoR(y2,u,v); 
        g2 = YCbCrtoG(y2,u,v);
        b2 = YCbCrtoB(y2,u,v);
        if(r1>255)  
        r1 = 255;  
        else if(r1<0)  
        r1 = 0;  
        
        if(b1>255)  
        b1 = 255;  
        else if(b1<0)  
        b1 = 0;      
        
        if(g1>255)  
        g1 = 255;  
        else if(g1<0)  
        g1 = 0;      
        
        if(r2>255)  
        r2 = 255;  
        else if(r2<0)  
        r2 = 0;  
        
        if(b2>255)  
        b2 = 255;  
        else if(b2<0)  
        b2 = 0;      
        
        if(g2>255)  
        g2 = 255;  
        else if(g2<0)  
        g2 = 0;          
        
        *rgb++= (unsigned char)r1;  
        *rgb++= (unsigned char)g1;  
        *rgb++= (unsigned char)b1;  
        *rgb++ =(unsigned char)r2;  
        *rgb++= (unsigned char)g2;  
        *rgb++= (unsigned char)b2;
    }
    
    /*
    int picfd = open(RGB_FILE,O_RDWR|O_CREAT|O_TRUNC,0777);
    if(picfd<0){
        printf("open rgb file failed!\n") ;
        perror("open rgb file") ;
        return -1 ;
    }
    write(picfd,PIC.pic_buf,640*480*3) ;
    close(picfd) ;
    return TRUE ;
    */
}

int camera_rgb_to_jpeg()
{
    char* rgb_buffer = PIC.pic_buf ;
    int quality = QUALITY ;
    int width = PIC_WIDTH ;
    int height = PIC_HEIGHT ;
    struct jpeg_compress_struct cinfo ;//定义压缩对象，处理主要的功能
    struct jpeg_error_mgr jerr;//用于错误信息的结构体
    JSAMPROW row_pointer[1] ;//指向一行图像数据的指针,一行位图
    int row_stride ;//每行需要的空间大小；每一行的字节数
    unsigned char* buffs = NULL ;
    unsigned long buffsize = 0 ;

    cinfo.err = jpeg_std_error(&jerr) ;//将错误处理结构对象绑定在JPEG对象上
    jpeg_create_compress(&cinfo) ;//初始化cinfo结构;
    
    //JPEG.outbuffer = NULL ;
    //JPEG.buffersize = 0 ;
    jpeg_mem_dest(&cinfo,&buffs,&buffsize); //mem为内存操作
    
    cinfo.image_width = width ;//图像宽度
    cinfo.image_height = height ;//图像高度
    cinfo.input_components = 3 ;//每个像素中的通道数（灰度为1，全彩色3）
    cinfo.in_color_space = JCS_RGB ;//图像颜色空间

    jpeg_set_defaults(&cinfo) ;//设定缺省设置,设定之前必须设定in_color_space
    jpeg_set_quality(&cinfo,QUALITY,TRUE) ;// 设定编码jpeg压缩质量
    jpeg_start_compress(&cinfo,TRUE) ;//开始压缩；
    row_stride = width*3 ;
    // 对每一行进行压缩
    while(cinfo.next_scanline<cinfo.image_height){
        row_pointer[0] = &rgb_buffer[cinfo.next_scanline*row_stride] ;
        jpeg_write_scanlines(&cinfo,row_pointer,1) ;
    }
    jpeg_finish_compress(&cinfo);//结束压缩
    pthread_mutex_lock(&jpeg_mutex);
    pic->len = buffsize;
    web->len = buffsize;
    memcpy(pic->picbuf,buffs,buffsize);
    memcpy(web->picbuf,buffs,buffsize);
    pthread_mutex_unlock(&jpeg_mutex);
    pthread_cond_signal(&jpeg_cond);
    jpeg_destroy_compress(&cinfo) ;
    return 0;
}

#if 0
void camera_video_save(){
    int ret ;
    ret = ioctl(fd,VIDIOC_DQBUF,&buf) ;
    int savefd = open(JPEG_FILE,O_RDWR|O_CREAT|O_TRUNC,0777) ;
    if(savefd<0){
        printf("savefd failed!\n") ;
        perror("savefd") ;
        return  ;
    }
    printf("open savefd success!\n") ;

    char videobuff[128*1024] = {0} ;
    ret = write(savefd,buffers[buf.index].start,buf.bytesused) ;
    if(ret<0){
        printf("write from savefd failed!\n") ;
        perror("write fd") ;
        return  ;
    }
    printf("write fd from savefd success!\n") ;
    close(savefd) ;
    printf("save video success!\n") ;
    return ;
}
#endif

int camera_capture(){
    int ret ;
    enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    ret = ioctl(fd,VIDIOC_STREAMON,&type) ; //帧缓存入队
    if(ret<0){
        printf("capture failed!\n") ;
        perror("capture ret") ;
        return -1 ;
    }
    printf("camera is capturing.................\n") ;
    return 0 ;
}

void uninit_device(){
    unsigned int i ;
    for(i=0;i<req.count;i++){
        if(-1 == munmap(buffers[i].start,buffers[i].length)){
            printf("munmap failed!\n") ;
            perror("munmap") ;
            return  ;
        }
        buffers = NULL ;
    }
    return ;
}

void camera_off(){
    int ret ;
    ret = ioctl(fd,VIDIOC_STREAMOFF,&buf) ;
    if(ret<0){
        printf("close camera failed!\n") ;
        perror("close camera") ;
        return  ;
    }
    close(fd) ;
    return ;
}


/********************************************************/
#if 0
int main(){
    camera_open_device();
    camera_get_capability();
    camera_get_format() ;
    camera_set_format() ;
    camera_request_buf() ;
    camera_query_mmap_buf() ;
    camera_capture() ;
    while(1){
        camera_yuyv_to_rgb() ;
        camera_rgb_to_jpeg() ;
    }
    uninit_device() ;
    camera_off() ;
}
#endif
