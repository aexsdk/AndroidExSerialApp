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

#include	"uevent.h"

#define UEVENT_ANDROIDEX_READ	    "/dev/hwread"
#define MAX_BUFF 2048

static ON_UEVENT_EVENT on_uevent_event=NULL;

/**
 * 设置回调函数，在JNI的代码里会调用它来设置处理事件的回调函数
 */
void kkuevent_set_event(ON_UEVENT_EVENT oke)
{
	on_uevent_event = oke;
}

/**
 * 事件的入口函数，静态函数只能在本文件中调用
 */
 int kkuevent_event(HKKP env,HKKP obj,int code,char *pszFormat,...)
{
	char pszDest[MAX_BUFF];
	va_list args;

	va_start(args, pszFormat);
	vsnprintf(pszDest, sizeof(pszDest), pszFormat, args);
	va_end(args);
	//只有设置了事件回调函数，此函数才会调用事件，否则什么也不做
	if(on_uevent_event){
		return on_uevent_event(env,obj,code,pszDest);
	}else{
	    return 0;
	}
}


/**
 * 打开，返回0失败   其他成功
 * @kkueventram arg 参数字符串
 */
UEVENT_HANDLE kkuevent_open(HKKP env,HKKP obj,char* arg)
{
    struct sockaddr_nl nladdr;
    int fd;
    int sz = 64 * 1024;
    int on = 1;

	fd = open(UEVENT_ANDROIDEX_READ,O_RDONLY);
	if(fd >= 0){
		UEVENT_HANDLE uevent = (UEVENT_HANDLE)malloc(sizeof(UEVENT_DATA));
		if(uevent){
			memset(uevent,0,sizeof(UEVENT_DATA));
			uevent->fd = fd;
			if(arg != NULL)
				strncpy(uevent->subsystem,arg,sizeof(uevent->subsystem)-1);
			__android_log_print(ANDROID_LOG_DEBUG,"UEVENT","Uevent open return %x\n", (unsigned int)uevent);
			return uevent;
		}else{
			__android_log_print(ANDROID_LOG_DEBUG,"UEVENT","Uevent malloc handler error: %s",strerror(errno));
			kkuevent_event(env,obj,UE_ERROR,"Uevent malloc handler error: %s",strerror(errno));
			close(fd);
			return NULL;
		}
	}else{
		__android_log_print(ANDROID_LOG_DEBUG,"UEVENT","Uevent open %s error: %s",UEVENT_ANDROIDEX_READ,strerror(errno));
		kkuevent_event(env,obj,UE_ERROR,"Uevent open %s error: %s",UEVENT_ANDROIDEX_READ,strerror(errno));
	}
	return NULL;
}

/**
 * 打开，返回0失败   其他成功
 * @kkueventram arg 参数字符串
 */
int kkuevent_read(UEVENT_HANDLE uevent,HKKP env,HKKP obj,char* arg)
{
    char data[1024];
	int	res,havesybsystem;
	int fdrd = 0;

	if(uevent == NULL){
		__android_log_print(ANDROID_LOG_DEBUG,"UEVENT","Uevent read error:uevent=%x\n", uevent);
		return 0;
	}
	__android_log_print(ANDROID_LOG_DEBUG,"UEVENT","Uevent read:uevent=%x\n", uevent);
	if(arg != NULL && strlen(arg)>0){
		strncpy(uevent->subsystem,arg,sizeof(uevent->subsystem));
	}
	havesybsystem = strlen(uevent->subsystem);
	while(1)
    {
	    int iret = 0;
		fd_set rfds;

		fdrd = uevent->fd;
		FD_CLR(fdrd,&rfds);
		FD_ZERO(&rfds);
		FD_SET(fdrd, &rfds);
		//timeout.tv_sec=5;
		iret = select(fdrd + 1, &rfds, NULL, NULL, NULL);
		if(iret <= 0){
			kkuevent_event(env,obj,UE_ERROR,"Uevent select error: %s", strerror(errno));
			break;
		}
		if(FD_ISSET(fdrd, &rfds)){
			memset(data,0,strlen(data));
			res = read(fdrd,data,sizeof(data));
			//__android_log_print(ANDROID_LOG_DEBUG,"UEVENT","select iret=%d ,fdrd =%d,recv(%d)=%s",iret,fdrd,res,data);
			if(res <= 0){
				kkuevent_event(env,obj,UE_ERROR,"Uevent read error: %s", strerror(errno));
				//kkuevent_close(uevent);
                break;
			}else{
				__android_log_print(ANDROID_LOG_DEBUG,"UEVENT","%s",data);
				if(havesybsystem){
					if(strstr(data,uevent->subsystem) != NULL){
						kkuevent_event(env,obj,UE_EVENT,data);
					}
				}else{
					kkuevent_event(env,obj,UE_EVENT,data);
				}
			}
		}
	}
	kkuevent_event(env,obj,UE_READ_END,"Uevent read end.");
	kkuevent_close(uevent,env,obj);
	return -1;
}

/**
 * 函数名：kkuevent_close
 * 参数：
 * 返回值：0，成功；其他为失败；
 * 说明：关闭打印机串口
 */
int kkuevent_close(UEVENT_HANDLE uevent,HKKP env,HKKP obj)
{
	if(uevent){
		if(uevent->fd){
			close(uevent->fd);
		}
		free(uevent);
		return 0;
	}
	return -1;
}








