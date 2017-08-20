#include <jni.h>
#include <stdio.h>      /*标准输入输出定义*/
#include <stdlib.h>
#include <android/log.h>
#include "com_androidex_plugins_kkserial.h"
#include "serial.h"
#include "../include/utils.h"

jint Serial_OnLoad(JavaVM* vm, void* reserved)
{
	return JNI_VERSION_1_4;
}

void Serial_OnUnload(JavaVM* vm, void* reserved)
{
}

/*
 *This function loads a locally-defined class.
 *这个函数加载一个本地定义的类
 * */
static jclass getProvider(JNIEnv *env)
{
    return (*env)->FindClass(env,"com/androidex/plugins/kkserial");
}

/*
 *Returns the method ID for an instance (nonstatic) method of a class or interface.

 *返回类或接口实例（非静态）方法的方法 ID。方法可在某个 clazz 的超类中定义，也可从 clazz 继承。该方法由其名称和签名决定。

 *GetMethodID() 可使未初始化的类初始化。
 * */
static jmethodID getMethod(JNIEnv *env, char *func,char *result)
{
    jclass lProvider = getProvider(env);

    if(lProvider)
    {
        return (*env)->GetMethodID(env, lProvider, func,result);
    }
    return NULL;
}

/**
 * 调用了Java对应打印机处理方法的函数，此函数会用于打印机的事件处理
 */
int jni_log_event(JNIEnv *env, jobject obj,int fd,char *msg)
{
    jmethodID onLogEvent = NULL;
    JNIEnv* jniEnv = (JNIEnv*)env;
    jobject javaObject = (jobject)obj;

    if(jniEnv == NULL && javaObject == NULL) {
        return 0;
    }

    if(onLogEvent == NULL){
        onLogEvent = getMethod(env,"OnLogEvent","(ILjava/lang/String;)V");
    }
    if(onLogEvent){
        jstring strmsg = (*jniEnv)->NewStringUTF(jniEnv, (const char *)msg);
        /*调用一个由methodID定义的实例的Java方法，可选择传递参数（args）的数组到这个方法。*/
        (*jniEnv)->CallVoidMethod(jniEnv, javaObject, onLogEvent,(jint)fd,strmsg);
        return 0;
    }
    return -1;
}

/**
 * 调用了Java对应打印机处理方法的函数，此函数会用于打印机的事件处理
 */
int jni_data_recive_event(JNIEnv *env, jobject obj,int fd,char *data,int len)
{
    jmethodID onDataReciveEvent = NULL;
    JNIEnv* jniEnv = (JNIEnv*)env;
    jobject javaObject = (jobject)obj;

    if(jniEnv == NULL && javaObject == NULL) {
        return 0;
    }

    if(onDataReciveEvent == NULL){
        onDataReciveEvent = getMethod(env,"OnDataReciveEvent","(ILjava/lang/[B/lang;)V");
    }
    if(onDataReciveEvent){
        jbyte* byte = (jbyte*)data;
        jbyteArray jarray = (*env)->NewByteArray(env,len);
        (*env)->SetByteArrayRegion(env,jarray, 0, len, byte);
        /*调用一个由methodID定义的实例的Java方法，可选择传递参数（args）的数组到这个方法。*/
        (*jniEnv)->CallVoidMethod(jniEnv, javaObject, onDataReciveEvent,(jint)fd,jarray,len);
        return 0;
    }
    return -1;
}

JNIEXPORT jint JNICALL Java_com_androidex_plugins_kkserial_native_1serial_1open
(JNIEnv *env, jobject this, jstring strarg)
{
    int fd = 0;
    char *charg = (char *)(*env)->GetStringUTFChars(env, strarg, 0);

    __android_log_print(0,tag, "%s:%s", __func__,charg);
    fd = kkserial_open(env,this,charg);
    (*env)->ReleaseStringUTFChars(env, strarg, charg);
    return fd;
}

/*
 * Class:     com_eztor_plugins_serial
 * Method:    serial_select
 * Signature: (III)I
 */
JNIEXPORT jint JNICALL Java_com_eztor_plugins_kkserial_native_1serial_1select
        (JNIEnv *env, jobject this, jint fd, jint usec)
{
    int r = kkserial_select(env,this,fd,usec);
    return r;
}

JNIEXPORT jbyteArray JNICALL Java_com_androidex_plugins_kkserial_native_1serial_1read
        (JNIEnv *env, jobject this, jint fd, jint length, jint timeout)
{
    char *buf;

    if(length <= 0 || length > MAX_READLEN)
        length = MAX_READLEN;
    buf = malloc(length);
    if(buf){
        memset(buf,0,sizeof(buf));
        int len = kkserial_read(env,this,fd, length, buf,timeout);
        if(len > 0){
            jbyte* byte = (jbyte*)buf;
            jbyteArray jarray = (*env)->NewByteArray(env,len);
            (*env)->SetByteArrayRegion(env,jarray, 0, len, byte);
            free(buf);
            return jarray;
        }
        free(buf);
    }
    return NULL;
}

JNIEXPORT jint JNICALL Java_com_androidex_plugins_kkserial_native_1serial_1write
        (JNIEnv *env, jobject this, jint fd, jbyteArray data,jint len)
{
    jbyte * arrayBody = (*env)->GetByteArrayElements(env,data,0);
    char* szdata = (char*)arrayBody;

    int r = kkserial_write(env,this,fd,szdata,len);
    (*env)->ReleaseByteArrayElements(env,data, arrayBody,0);
    return r;
}

JNIEXPORT jint JNICALL Java_com_androidex_plugins_kkserial_native_1serial_1close
    (JNIEnv *env, jobject this,jint fd)
{
    kkserial_close(env,this,fd);
	return 0;
}

JNIEXPORT jstring JNICALL Java_com_androidex_plugins_kkserial_native_1serial_1readHex
        (JNIEnv *env, jobject this, jint fd, jint length, jint timeout)
{
    char *buf;
    jstring strmsg = NULL;

    if(length <= 0 || length > MAX_READLEN)
        length = MAX_READLEN;
    buf = malloc(length);
    if(buf){

        memset(buf,0,sizeof(buf));
        int len = kkserial_read(env,this,fd, length, buf,timeout);
        if(len > 0){
            jbyte* byte = (jbyte*)buf;
            jbyteArray jarray = (*env)->NewByteArray(env,len);
            (*env)->SetByteArrayRegion(env,jarray, 0, len, byte);
            int dlen = len*2 + 1;
            char *hexbuf = malloc(dlen);

            if(hexbuf){
                memset(hexbuf,0,dlen);
                HexEncode((const unsigned char *)buf,len,hexbuf,&dlen);
                strmsg = (*env)->NewStringUTF(env, (const char *)hexbuf);
                free(hexbuf);
            }
        }
        free(buf);
    }
    return strmsg;
}


JNIEXPORT jint JNICALL Java_com_androidex_plugins_kkserial_native_1serial_1writeHex
        (JNIEnv *env, jobject this, jint fd, jstring data)
{
    char* szdata = (char*)(*env)->GetStringUTFChars(env, data, 0);;
    int len = strlen(szdata);
    int dlen = len;
    char *buf = malloc(dlen);
    int r = 0;

    if(buf){
        memset(buf,0,dlen);
        HexDecode(szdata,len,buf,&dlen);
        __android_log_print(ANDROID_LOG_DEBUG,"kkserial","writeHex([%d=>%d]%s)",len,dlen,szdata);
        r = kkserial_write(env,this,fd,buf,dlen);
        free(buf);
    }else {
        __android_log_print(ANDROID_LOG_DEBUG,"kkserial","writeHex error: malloc buf error.dlen=%d",dlen);
    }

    (*env)->ReleaseStringUTFChars(env, data, szdata);
    return r;
}

JNIEXPORT jbyteArray JNICALL Java_com_androidex_plugins_kkserial_native_1serial_1readloop
        (JNIEnv *env, jobject this, jint fd, jint length)
{
    if(length <= 0 || length > MAX_READLEN)
        length = MAX_READLEN;
    return kkserial_readloop(env,this,fd, length);
}
