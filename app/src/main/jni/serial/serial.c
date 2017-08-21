#include     <stdio.h>      /*标准输入输出定义*/
#include     <unistd.h>     /*Unix标准函数定义*/
#include     <sys/types.h>  /**/
#include     <sys/stat.h>   /**/
#include     <fcntl.h>      /*文件控制定义*/
#include     <termios.h>    /*PPSIX终端控制定义*/
#include     <errno.h>      /*错误号定义*/
#include     <sys/ioctl.h>
#include     <ctype.h>
#include     <string.h>
#include	 <stdlib.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/un.h>

#include <jni.h>
#include <linux/netlink.h>
#include <android/log.h>
#include <dirent.h>
#include "../include/utils.h"
#include "serial.h"

#define MAX_BUFF 2048

/**
 * 事件的入口函数，静态函数只能在本文件中调用
 */
 int kkserial_event(HKKP env,HKKP obj,int fd,char *pszFormat,...)
{
	char pszDest[MAX_BUFF];
	va_list args;

    memset(pszDest,0,MAX_BUFF);
	va_start(args, pszFormat);
	vsnprintf(pszDest, sizeof(pszDest), pszFormat, args);
	va_end(args);
	//只有设置了事件回调函数，此函数才会调用事件，否则什么也不做
    //__android_log_print(ANDROID_LOG_DEBUG,"kkserial","%s",pszDest);
	jni_log_event(env,obj,fd,pszDest);
    return 0;
}

/**
 * 打开串口
 * @param arg 串口参数字符串，字符串格式为:	com=/dev/ttyUSB0(串口设备字符串),s=9600(波特率),p=N(奇偶校验),b=1(停止位),d=8(数据位数)
 */
int kkserial_open(HKKP env,HKKP obj,char *arg)
{
	int fd = com_open(arg);
	if(fd > 0){
		return fd;
	}else{
		return NULL;
	}
}

/**
 *关闭串口
 */
void kkserial_close(HKKP env,HKKP obj,int fd)
{
	if(fd >0){
        kkserial_event(env,obj,fd,"Close %d",fd);
		com_close(fd);
	}else{
        kkserial_event(env,obj,fd,"Close %d",fd);
    }
}

int kkserial_write(HKKP env,HKKP obj,int fd,char *data,int len)
{
	if(fd <= 0 )return FALSE;
    //__android_log_print(ANDROID_LOG_DEBUG,"kkserial","kkserial_write(data[0]=%02X,len=%d)",data[0],len);
    int r = com_write(fd, data, len);
	return r;
}

int kkserial_read(HKKP env,HKKP obj,int fd, int length, char *buf,int timeout)
{
	if(fd <= 0 )return FALSE;
	int r = 0;
	r =  com_recive(fd,buf,length,timeout);
	return r;
}

int kkserial_select(HKKP env,HKKP obj,int fd,int usec)
{
	if(fd <= 0 )return FALSE;
	int r = 0;
	r = com_select(fd,usec/1000000,usec%1000000);
	return r;
}


/**
 * 打开，返回0失败   其他成功
 * @kkueventram arg 参数字符串
 */
int kkserial_readloop(HKKP env,HKKP obj,int fd,int len)
{
	char data[1024];
	int	res,loop = 1;
	int revLen = len > 1023 ? 1023 : len-1;

	kkserial_event(env,obj,fd,"KKSERIAL read:fd=%d,length=%d\n", fd,len);
	while(loop)
	{
		int iret = 0;
		fd_set rfds;

		FD_CLR(fd,&rfds);
		FD_ZERO(&rfds);
		FD_SET(fd, &rfds);
        kkserial_event(env,obj,fd,"Start select ...");
		iret = select(fd + 1, &rfds, NULL, NULL, NULL);
        //kkserial_event(env,obj,fd,"   select ret=%d",iret);
		if(iret < 0){
			//发生错误
			kkserial_event(env,obj,fd,"KKSERIAL select error: %s", strerror(errno));
			tcflush(fd, TCIOFLUSH);
			loop = 0;		//退出循环
		}else if(iret == 0){
			//等待超时
			kkserial_event(env,obj,fd,"KKSERIAL select timeout");
		}else{
			if(FD_ISSET(fd, &rfds)){
				memset(data,0,1024);
				res = read(fd,data,revLen);
				if(res < 0){
					kkserial_event(env,obj,fd,"KKSERIAL read error: %s", strerror(errno));
					loop = 0;
				}else if(res == 0){
					//读取到文件尾
					kkserial_event(env,obj,fd,"KKSERIAL read empty");
				}else{
					//收到数据
					jni_data_recive_event(env,obj,fd,data,res);
                    //kkserial_event(env,obj,fd,"Recive from %d:%d:%s",fd,res,data);
				}
			}else{
                kkserial_event(env,obj,fd,"No fd_isset");
            }
		}
	}
	kkserial_event(env,obj,fd,"KKSERIAL read end.");
	return res;
}

