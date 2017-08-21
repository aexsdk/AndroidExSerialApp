package com.androidex.plugins;

/**
 * 串口操作插件,上层Java操作底层文件经常会发生权限等问题,使用此JNI插件来处理串口操作简单很多.操作方法也类似C语言的操作习惯.
 */

import android.content.Context;
import com.androidex.logger.Log;

import java.io.UnsupportedEncodingException;

public class kkserial {
    private static final String TAG = "serial";

    static
    {
        try {
            System.loadLibrary("appSerialLibs");
        } catch (UnsatisfiedLinkError e) {
            Log.d("Debug", "appSerialLibs.so library not found!");
        }
    }

    Context ctx=null;
    public kkserial(Context _ctx){
        ctx = _ctx;
    }

    public String byteToString(byte[] data){
        StringBuilder sMsg = new StringBuilder();
        if (data[0] == '$') {
            try {
                sMsg.append(new String(data, "UTF-8"));
            } catch (UnsupportedEncodingException e) {
                e.printStackTrace();
            }
        }else{
            try {
                sMsg.append(new String(data, "UTF-8"));
            } catch (UnsupportedEncodingException e) {
                e.printStackTrace();
            }
        }

        return sMsg.toString();
    }
    /**
     *      说明：该函数接收从jni层传送上来的事件，当JNI层函数在返回前需要Java层执行某些操作或者传送信息时，使用此事件函数。
     * 如果本函数没有处理则默认会交给生成对象时提供的Context对象去处理。
     * @param fd 消息代码，如果_code==0时，_msg为读取到的uevent数据
     * @param _msg  消息内容
     */
    public void OnLogEvent(int fd, String _msg) {
        //android.util.Log.i(TAG, String.format("OnLogEvent: fd=%d,msg=%s\n",fd,_msg));
        if ((ctx != null) && (ctx instanceof OnCallback)) {
            OnCallback listener = (OnCallback) ctx;
            listener.onLogEvent(fd, _msg);
        }
    }

    public void OnDataReciveEvent(int fd, byte[] data,int len) {
        //收到数据的事件
        // android.util.Log.i(TAG, String.format("OnLogEvent: fd=%d,msg=%s\n",fd,byteToString(data)));
        if ((ctx != null) && (ctx instanceof OnCallback)) {
            OnCallback listener = (OnCallback) ctx;
            listener.onDataRecive(fd, data,len);
        }
    }

   /**
	 * 打开串口端口
	 * @param sport  要打开串口的地址信息字符串
	 * @return  成功返回非0，否则返回0或者小于0
	 */
	public int serial_open(String sport){
		int r = native_serial_open(sport);
		return r;
	}

    /**
     * 关闭UEvent收取
     * @return 成功返回非0，否则返回0或者小于0
     */
    public int serial_close(int fd){
        int r = native_serial_close(fd);
        return r;
    }

    public int serial_select(int fd,int timeout)
    {
        return native_serial_select(fd,timeout);
    }

	/**
	 * 启动读取串口数据的线程
	 * @return 返回读取结束的方式，一般是调用close来控制结束
	 */
	public byte[] serial_read(int fd,int length, int timeout){
        return native_serial_read(fd,length,timeout);
	}

    public int serial_readloop(final int fd, final int length){
        Runnable run=new Runnable() {
            public void run() {
                OnLogEvent(fd,"开始读取串口数据");
                native_serial_readloop(fd,length);
                OnLogEvent(fd,"读取结束");
            }
        };
        Thread pthread = new Thread(run);
        pthread.start();
        return 1;
    }

    public String serial_readHex(int fd,int length, int timeout){
        return native_serial_readHex(fd,length,timeout);
    }

	/**
	 * 写数据到串口
	 * @param data  要写入的数据
	 * @return 返回写入的字节数或者错误值
	 */
	public int serial_write(int fd,byte[] data,int size){
		int r = native_serial_write(fd,data,size);
		return r;
	}

    public int serial_writeHex(int fd,String data){
        int r = native_serial_writeHex(fd,data);
        return r;
    }

	//jni 相关函数
	public native int       native_serial_open(String sport);
    public native int       native_serial_close(int fd);
    public native int       native_serial_select(int fd, int usec);
    public native byte[]    native_serial_read(int fd,int length,int timeout);
	public native int       native_serial_write(int fd,byte[] data,int size);
    public native String    native_serial_readHex(int fd,int length,int timeout);
    public native int       native_serial_writeHex(int fd,String data);
    public native int       native_serial_readloop(int fd,int length);
}
