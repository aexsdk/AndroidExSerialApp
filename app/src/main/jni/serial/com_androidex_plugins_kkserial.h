/* DO NOT EDIT THIS FILE - it is machine generated */
#include <jni.h>
/* Header for class com_androidex_plugins_kkserial */

#ifndef _Included_com_androidex_plugins_kkserial
#define _Included_com_androidex_plugins_kkserial
static char *const tag = "Serial";
#ifdef __cplusplus
extern "C" {
#endif

jint Serial_OnLoad(JavaVM* vm, void* reserved);
void Serial_OnUnload(JavaVM* vm, void* reserved);

/*
 * Class:     com_androidex_plugins_kkserial
 * Method:    native_serial_open
 * Signature: (Ljava/lang/String;)I
 */
JNIEXPORT jint JNICALL Java_com_androidex_plugins_kkserial_native_1serial_1open
  (JNIEnv *, jobject, jstring);

/*
 * Class:     com_androidex_plugins_kkserial
 * Method:    native_serial_close
 * Signature: (I)I
 */
JNIEXPORT jint JNICALL Java_com_androidex_plugins_kkserial_native_1serial_1close
  (JNIEnv *, jobject, jint);

/*
 * Class:     com_androidex_plugins_kkserial
 * Method:    native_serial_select
 * Signature: (II)I
 */
JNIEXPORT jint JNICALL Java_com_androidex_plugins_kkserial_native_1serial_1select
  (JNIEnv *, jobject, jint, jint);

/*
 * Class:     com_androidex_plugins_kkserial
 * Method:    native_serial_read
 * Signature: (III)[B
 */
JNIEXPORT jbyteArray JNICALL Java_com_androidex_plugins_kkserial_native_1serial_1read
  (JNIEnv *, jobject, jint, jint, jint);

/*
 * Class:     com_androidex_plugins_kkserial
 * Method:    native_serial_write
 * Signature: (I[BI)I
 */
JNIEXPORT jint JNICALL Java_com_androidex_plugins_kkserial_native_1serial_1write
  (JNIEnv *, jobject, jint, jbyteArray, jint);

/*
 * Class:     com_androidex_plugins_kkserial
 * Method:    native_serial_readHex
 * Signature: (III)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_androidex_plugins_kkserial_native_1serial_1readHex
  (JNIEnv *, jobject, jint, jint, jint);

/*
 * Class:     com_androidex_plugins_kkserial
 * Method:    native_serial_writeHex
 * Signature: (ILjava/lang/String;)I
 */
JNIEXPORT jint JNICALL Java_com_androidex_plugins_kkserial_native_1serial_1writeHex
  (JNIEnv *, jobject, jint, jstring);

#ifdef __cplusplus
}
#endif
#endif
