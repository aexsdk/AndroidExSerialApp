#include <jni.h>
#include <stdio.h>      /*标准输入输出定义*/
#include <stdlib.h>
#include <android/log.h>
#include "com_androidex_plugins_uevent.h"
#include "uevent.h"


UEVENT_HANDLE s_uevent = NULL;

/*提供给回调使用*/
jclass ueventProvider=NULL;
jmethodID javaueventEvent=NULL;

/*
 *This function loads a locally-defined class.
 *这个函数加载一个本地定义的类
 * */
static jclass getProvider(JNIEnv *env)
{
	return (*env)->FindClass(env,"com/androidex/plugins/kkuevent");
}


/*
 *Returns the method ID for an instance (nonstatic) method of a class or interface.

 *返回类或接口实例（非静态）方法的方法 ID。方法可在某个 clazz 的超类中定义，也可从 clazz 继承。该方法由其名称和签名决定。

 *GetMethodID() 可使未初始化的类初始化。
 * */
static jmethodID getMethod(JNIEnv *env, char *func,char *result)
{
	if(ueventProvider==NULL)
		ueventProvider = getProvider(env);
	if(ueventProvider)
	{
		return (*env)->GetMethodID(env, ueventProvider, func,result);
	}
    return NULL;
}

/**
 * 调用了Java对应打印机处理方法的函数，此函数会用于打印机的事件处理
 */
static int jni_uevent_event(JNIEnv *env, jobject obj,int code,char *msg)
{
	JNIEnv* jniEnv = (JNIEnv*)env;
	jobject javaObject = (jobject)obj;

    if(jniEnv == NULL && javaObject == NULL) {
        return 0;
    }

	if(javaueventEvent==NULL){
		javaueventEvent = getMethod(env,"OnUEvent","(ILjava/lang/String;)V");
	}
	if(ueventProvider && javaueventEvent){
		jstring strmsg = (*jniEnv)->NewStringUTF(jniEnv, (const char *)msg);
		/*调用一个由methodID定义的实例的Java方法，可选择传递参数（args）的数组到这个方法。*/
		(*jniEnv)->CallVoidMethod(jniEnv, javaObject, javaueventEvent,(jint)code,strmsg);
		return 0;
	}
	else
		return -1;
	return 0;
}

jint Uevent_OnLoad(JavaVM* vm, void* reserved)
{
	jint res;
	JNIEnv* env;

	res = (*vm)->GetEnv(vm, (void **)&env, JNI_VERSION_1_4);
	s_uevent = kkuevent_open(env,NULL,"");
	return JNI_VERSION_1_4;
}

void Uevent_OnUnload(JavaVM* vm, void* reserved)
{
	jint res;
	JNIEnv* env;

	res = (*vm)->GetEnv(vm, (void **)&env, JNI_VERSION_1_4);
	if(s_uevent != NULL){
		kkuevent_close(s_uevent,NULL,NULL);
		s_uevent = NULL;
	}
}

JNIEXPORT jint JNICALL Java_com_androidex_plugins_kkuevent_native_1uevent_1open
(JNIEnv *env, jobject this, jstring strarg)
{
	char *charg = (char *)(*env)->GetStringUTFChars(env, strarg, 0);

	kkuevent_set_event(jni_uevent_event);
	if(!s_uevent){
		s_uevent = kkuevent_open(env,this,charg);
	}

	(*env)->ReleaseStringUTFChars(env, strarg, charg);
	if(s_uevent != NULL){
		return 1;
	}else{
		return 0;
	}
}

JNIEXPORT jint JNICALL Java_com_androidex_plugins_kkuevent_native_1uevent_1read
(JNIEnv *env, jobject this, jstring strarg)
{
	char *charg = (char *)(*env)->GetStringUTFChars(env, strarg, 0);
	int ret = -1;

	if(s_uevent == NULL){
		s_uevent = kkuevent_open(env,this,charg);
	}

	if(s_uevent){
		ret = kkuevent_read(s_uevent,env,this,charg);
	}
	(*env)->ReleaseStringUTFChars(env, strarg, charg);
	return ret;
}

JNIEXPORT jint JNICALL Java_com_androidex_plugins_kkuevent_native_1uevent_1close
  (JNIEnv *env, jobject this)
{
	int iret = -1;

	if(s_uevent){
		iret = kkuevent_close(s_uevent,env,this);
		s_uevent=NULL;
	}
	return iret;
}
