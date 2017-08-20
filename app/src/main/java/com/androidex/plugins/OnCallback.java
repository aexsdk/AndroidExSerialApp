package com.androidex.plugins;

public interface OnCallback {
    void onLogEvent(int fd,String msg);
    void onDataRecive(int fd,byte[] data,int len);
}
