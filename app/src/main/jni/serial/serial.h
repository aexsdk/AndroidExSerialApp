#ifndef __HSERIAL__
#define __HSERIAL__

#define MAX_READLEN         2048

typedef long                LONG;
typedef unsigned short      WORD;
typedef unsigned long       DWORD;


typedef enum udev_event{
    UE_START 		= 0x10000
    ,UE_ERROR 		= 0x10001
    ,UE_STATUS		= 0x10201
    ,UE_READ_END	= 0x10202
    ,UE_EVENT		= 0x0
}PRINT_EVENT;

typedef void* HKKP;
typedef int (*ON_SERIAL_EVENT)(HKKP env,HKKP obj,int code,char *msg);
int jni_data_recive_event(JNIEnv *env, jobject obj,int fd,char *data,int len);
int jni_log_event(JNIEnv *env, jobject obj,int fd,char *msg);
/**
 * 打开，返回句柄
 * @param arg 串口参数字符串，字符串格式为:	com=/dev/ttyUSB0(串口设备字符串),s=9600(波特率),p=N(奇偶校验),b=1(停止位),d=8(数据位数)
 */
int kkserial_open(HKKP env,HKKP obj,char *arg);
void kkserial_close(HKKP env,HKKP obj,int fd);
int kkserial_read(HKKP env,HKKP obj,int fd,int length, char *buf, int timeout);
int kkserial_write(HKKP env,HKKP obj,int fd,char *data,int len);
int kkserial_select(HKKP env,HKKP obj,int fd, int usec);
int kkserial_readloop(HKKP env,HKKP obj,int fd,int length);

#endif
