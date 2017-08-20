//
// Created by 杨军 on 16/3/26.
//
#include <jni.h>
#include <stdio.h>      /*标准输入输出定义*/
#include <stdlib.h>
#include <android/log.h>
#include "../serial/com_androidex_plugins_kkserial.h"

jint JNI_OnLoad(JavaVM* vm, void* reserved)
{
    jint res;

    res = Serial_OnLoad(vm,reserved);
    return JNI_VERSION_1_4;
}

JNIEXPORT void JNICALL JNI_OnUnload(JavaVM* vm, void* reserved)
{
    Serial_OnUnload(vm, reserved);
}
