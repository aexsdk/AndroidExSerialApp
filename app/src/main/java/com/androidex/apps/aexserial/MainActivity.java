package com.androidex.apps.aexserial;

/**
 * Created by cts on 17/3/31.
 */

import android.app.Activity;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.SharedPreferences;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.content.res.Configuration;
import android.os.BatteryManager;
import android.os.Bundle;
import android.os.Parcelable;
import android.text.InputType;
import android.text.method.KeyListener;
import android.text.method.NumberKeyListener;
import android.text.method.TextKeyListener;
import android.util.Base64;
import android.util.Log;
import android.view.KeyEvent;
import android.view.View;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.CompoundButton;
import android.widget.EditText;
import android.widget.RadioButton;
import android.widget.Spinner;
import android.widget.TextView;
import android.widget.Toast;
import android.widget.ToggleButton;

import com.androidex.apps.bean.*;
import com.androidex.plugins.OnCallback;
import com.androidex.plugins.kkserial;

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;
import java.io.UnsupportedEncodingException;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.LinkedList;
import java.util.List;
import java.util.Queue;
import android.content.BroadcastReceiver;


import static android.R.attr.versionName;

/**
 * serialport api和jni取自http://code.google.com/p/android-serialport-api/
 *
 * @author benjaminwan
 *         串口助手，支持4串口同时读写
 *         程序载入时自动搜索串口设备
 *         n,8,1，没得选
 */
public class MainActivity extends Activity implements View.OnClickListener,OnCallback {
    EditText editTextRecDisp, editTextLines, editTextCOMA;
    EditText editTextTimeCOMA;
    CheckBox checkBoxAutoClear, checkBoxAutoCOMA;
    Button ButtonClear, ButtonSendCOMA, btn_queryVersion, btn_parameter;
    Button btn_serialText, btn_queryType;
    ToggleButton toggleButton_startTimingA,toggleButtonCOMA;
    Spinner SpinnerCOMA;
    Spinner SpinnerBaudRateCOMA;
    RadioButton radioButtonTxt, radioButtonHex;
    DispQueueThread DispQueue;//刷新显示线程
    SerialPortFinder mSerialPortFinder;//串口设备搜索
    AssistBean AssistData;//用于界面数据序列化和反序列化
    int iRecLines = 0;//接收区行数

    private kkserial serial;        //串口测试对象
    private int mSerialFd = 0;      //打开的串口句柄，调用串口对象函数时需要
    private String mPort = "/dev/ttymxc2,115200,N,1,8";     //打开串口的参数
    private Thread pthread = null;
    public static final String bdSerialReciveData          = "bd.serial.recivedata";
    public static final String bdSerialReciveLog           = "bd.serial.recivelog";
    private NotifyReceiver mReceiver;

    /**
     * Called when the activity is first created.
     */
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.main);
        serial = new kkserial(this);
        DispQueue = new DispQueueThread();
        DispQueue.start();
        AssistData = getAssistData();
        setControls();

        mReceiver = new NotifyReceiver();
        IntentFilter filter = new IntentFilter();
        filter.addAction(bdSerialReciveData);
        filter.addAction(bdSerialReciveLog);
        registerReceiver(mReceiver, filter);
    }

    @Override
    public void onDestroy() {
        unregisterReceiver(mReceiver);
        saveAssistData(AssistData);
        CloseComPort();
        super.onDestroy();
    }

    @Override
    public void onConfigurationChanged(Configuration newConfig) {
        super.onConfigurationChanged(newConfig);
        CloseComPort();
        setContentView(R.layout.main);
        setControls();
    }

    //----------------------------------------------------
    private void setControls() {
        String appName = getString(R.string.app_name);
        try {
            com.androidex.aexlibs.hwService mService = new com.androidex.aexlibs.hwService(this);
            setTitle(appName + ":" + mService.getSdkVersion());
        } catch (Exception e) {
            e.printStackTrace();
        }
        editTextRecDisp = (EditText) findViewById(R.id.editTextRecDisp);
        editTextLines = (EditText) findViewById(R.id.editTextLines);
        editTextCOMA = (EditText) findViewById(R.id.editTextCOMA);
        editTextTimeCOMA = (EditText) findViewById(R.id.editTextTimeCOMA);

        checkBoxAutoClear = (CheckBox) findViewById(R.id.checkBoxAutoClear);
        checkBoxAutoCOMA = (CheckBox) findViewById(R.id.checkBoxAutoCOMA);

        ButtonClear = (Button) findViewById(R.id.ButtonClear);
        ButtonSendCOMA = (Button) findViewById(R.id.ButtonSendCOMA);

        btn_serialText = (Button) findViewById(R.id.btn_serialText);
        btn_queryType = (Button) findViewById(R.id.btn_queryType);
        btn_queryVersion = (Button) findViewById(R.id.btn_queryVersion);
        btn_parameter = (Button) findViewById(R.id.btn_parameter);
        btn_serialText.setOnClickListener(this);
        btn_queryType.setOnClickListener(this);
        btn_queryVersion.setOnClickListener(this);
        btn_parameter.setOnClickListener(this);

        toggleButton_startTimingA = (ToggleButton) findViewById(R.id.toggleButton_startTimingA);
        toggleButtonCOMA = (ToggleButton) findViewById(R.id.toggleButtonCOMA);
        SpinnerCOMA = (Spinner) findViewById(R.id.SpinnerCOMA);
        SpinnerBaudRateCOMA = (Spinner) findViewById(R.id.SpinnerBaudRateCOMA);
        radioButtonTxt = (RadioButton) findViewById(R.id.radioButtonTxt);
        radioButtonHex = (RadioButton) findViewById(R.id.radioButtonHex);

        editTextCOMA.setOnEditorActionListener(new EditorActionEvent());
        editTextTimeCOMA.setOnEditorActionListener(new EditorActionEvent());
        editTextCOMA.setOnFocusChangeListener(new FocusChangeEvent());
        editTextTimeCOMA.setOnFocusChangeListener(new FocusChangeEvent());

        radioButtonTxt.setOnClickListener(new radioButtonClickEvent());
        radioButtonHex.setOnClickListener(new radioButtonClickEvent());
        ButtonClear.setOnClickListener(new ButtonClickEvent());
        ButtonSendCOMA.setOnClickListener(new ButtonClickEvent());
        toggleButton_startTimingA.setOnCheckedChangeListener(new ToggleButtonStartTimingListener());
        toggleButtonCOMA.setOnCheckedChangeListener(new ToggleButtonCheckedChangeEvent());
        checkBoxAutoCOMA.setOnCheckedChangeListener(new CheckBoxChangeEvent());

        ArrayAdapter<CharSequence> adapter = ArrayAdapter.createFromResource(this, R.array.baudrates_value, android.R.layout.simple_spinner_item);
        adapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        SpinnerBaudRateCOMA.setAdapter(adapter);
        SpinnerBaudRateCOMA.setSelection(12);

        mSerialPortFinder = new SerialPortFinder();
        String[] entryValues = mSerialPortFinder.getAllDevicesPath();
        List<String> allDevices = new ArrayList<String>();
        for (int i = 0; i < entryValues.length; i++) {
            allDevices.add(entryValues[i]);
        }
        ArrayAdapter<String> aspnDevices = new ArrayAdapter<String>(this, android.R.layout.simple_spinner_item, allDevices);
        aspnDevices.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        SpinnerCOMA.setAdapter(aspnDevices);
        if (allDevices.size() > 0) {
            SpinnerCOMA.setSelection(0);
        }
        SpinnerCOMA.setOnItemSelectedListener(new ItemSelectedEvent());
        SpinnerBaudRateCOMA.setOnItemSelectedListener(new ItemSelectedEvent());
        DispAssistData(AssistData);
    }

    /*********操作485指令************/
    public void onClick(View v) {
        switch (v.getId()) {
            case R.id.btn_serialText:
                sendData("$001,01&");
                break;
            case R.id.btn_queryType:
                sendData("$001,02&");
                break;
            case R.id.btn_queryVersion:
                sendData("$001,03&");
                break;
            case R.id.btn_parameter:
                sendData("$001,04&");
                break;

        }
    }

    @Override
    public void onLogEvent(int fd, String msg) {
        //收到的日志数据
        //log(String.format("fd=%d:%s", fd, msg));
        android.util.Log.i("SERIAL", String.format("OnLogEvent: fd=%d,msg=%s\n",fd,msg));
        Intent ds_intent = new Intent();
        ds_intent.setAction(bdSerialReciveLog);
        ds_intent.putExtra("data",msg);
        sendBroadcast(ds_intent);
    }

    @Override
    public void onDataRecive(int fd, byte[] data, int len) {
        //收到的串口数据
        //logData(data, len);
        android.util.Log.i("SERIAL", String.format("Rev from fd=%d(%d):%s\n",fd,len,serial.byteToString(data)));
        Intent ds_intent = new Intent();
        ds_intent.setAction(bdSerialReciveData);
        ds_intent.putExtra("data",data);
        sendBroadcast(ds_intent);
    }

    //----------------------------------------------------串口号或波特率变化时，关闭打开的串口
    class ItemSelectedEvent implements Spinner.OnItemSelectedListener {
        public void onItemSelected(AdapterView<?> arg0, View arg1, int arg2, long arg3) {
            if ((arg0 == SpinnerCOMA) || (arg0 == SpinnerBaudRateCOMA)) {
                CloseComPort();
                checkBoxAutoCOMA.setChecked(false);
                toggleButtonCOMA.setChecked(false);
            }
        }

        public void onNothingSelected(AdapterView<?> arg0) {

        }

    }

    //----------------------------------------------------编辑框焦点转移事件
    class FocusChangeEvent implements EditText.OnFocusChangeListener {
        public void onFocusChange(View v, boolean hasFocus) {
            if (v == editTextCOMA) {
                setSendData(editTextCOMA);
            } else if (v == editTextTimeCOMA) {
                setDelayTime(editTextTimeCOMA);
            }
        }
    }

    //----------------------------------------------------编辑框完成事件
    class EditorActionEvent implements EditText.OnEditorActionListener {
        public boolean onEditorAction(TextView v, int actionId, KeyEvent event) {
            if (v == editTextCOMA) {
                setSendData(editTextCOMA);
            } else if (v == editTextTimeCOMA) {
                setDelayTime(editTextTimeCOMA);
            }
            return false;
        }
    }

    //----------------------------------------------------Txt、Hex模式选择
    class radioButtonClickEvent implements RadioButton.OnClickListener {
        public void onClick(View v) {
            if (v == radioButtonTxt) {
                KeyListener TxtkeyListener = new TextKeyListener(TextKeyListener.Capitalize.NONE, false);
                editTextCOMA.setKeyListener(TxtkeyListener);
                AssistData.setTxtMode(true);
            } else if (v == radioButtonHex) {
                KeyListener HexkeyListener = new NumberKeyListener() {
                    public int getInputType() {
                        return InputType.TYPE_CLASS_TEXT;
                    }

                    @Override
                    protected char[] getAcceptedChars() {
                        return new char[]{'0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
                                'a', 'b', 'c', 'd', 'e', 'f', 'A', 'B', 'C', 'D', 'E', 'F'};
                    }
                };
                editTextCOMA.setKeyListener(HexkeyListener);
                AssistData.setTxtMode(false);
            }
            editTextCOMA.setText(AssistData.getSendA());
            setSendData(editTextCOMA);
        }
    }

    //----------------------------------------------------自动发送
    class CheckBoxChangeEvent implements CheckBox.OnCheckedChangeListener {
        public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
            if (buttonView == checkBoxAutoCOMA) {
                if (!toggleButtonCOMA.isChecked() && isChecked) {
                    buttonView.setChecked(false);
                    return;
                }
                SetLoopData(editTextCOMA.getText().toString());
                SetAutoSend(isChecked);
            }
        }
    }

    //----------------------------------------------------清除按钮、发送按钮
    class ButtonClickEvent implements View.OnClickListener {
        public void onClick(View v) {
            if (v == ButtonClear) {
                editTextRecDisp.setText("");
            } else if (v == ButtonSendCOMA) {
                sendData(editTextCOMA.getText().toString());
            }
        }
    }


    class ToggleButtonStartTimingListener implements ToggleButton.OnCheckedChangeListener {

        public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
            if (buttonView == toggleButton_startTimingA) {
                if (isChecked) {
                    Toast.makeText(MainActivity.this, "开启串口自动控制状态", Toast.LENGTH_LONG).show();
                    //ComA.startTime();
                    toggleButton_startTimingA.setChecked(true);
                } else {
                    Toast.makeText(MainActivity.this, "关闭串口自动控制状态", Toast.LENGTH_LONG).show();
                    //ComA.stopTime();
                    toggleButton_startTimingA.setChecked(false);
                }
            }
        }
    }

    //----------------------------------------------------打开关闭串口
    class ToggleButtonCheckedChangeEvent implements ToggleButton.OnCheckedChangeListener {
        public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
            if (buttonView == toggleButtonCOMA) {
                if (isChecked) {
                    if(mSerialFd <= 0) {
                        mPort = String.format("%s,%s,N,1,8", SpinnerCOMA.getSelectedItem().toString(),
                                SpinnerBaudRateCOMA.getSelectedItem().toString());
                        OpenComPort(mPort);
                    }
                } else {
                    if(mSerialFd > 0){
                        CloseComPort();
                    }
                }
                checkBoxAutoCOMA.setChecked(mSerialFd > 0);
            }
        }
    }

    //----------------------------------------------------刷新显示线程
    private class DispQueueThread extends Thread {
        private Queue<ComBean> QueueList = new LinkedList<ComBean>();

        @Override
        public void run() {
            super.run();
            while (!isInterrupted()) {
                final ComBean ComData;
                while ((ComData = QueueList.poll()) != null) {
                    runOnUiThread(new Runnable() {
                        public void run() {
                            DispRecData(ComData);
                        }
                    });
                    try {
                        Thread.sleep(100);//显示性能高的话，可以把此数值调小。
                    } catch (Exception e) {
                        e.printStackTrace();
                    }
                    break;
                }
            }
        }

        public synchronized void AddQueue(ComBean ComData) {
            QueueList.add(ComData);
        }
    }

    //----------------------------------------------------刷新界面数据
    private void DispAssistData(AssistBean AssistData) {
        editTextCOMA.setText(AssistData.getSendA());
        setSendData(editTextCOMA);
        if (AssistData.isTxt()) {
            radioButtonTxt.setChecked(true);
        } else {
            radioButtonHex.setChecked(true);
        }
        editTextTimeCOMA.setText(AssistData.sTimeA);
        setDelayTime(editTextTimeCOMA);
    }

    //----------------------------------------------------保存、获取界面数据
    private void saveAssistData(AssistBean AssistData) {
        AssistData.sTimeA = editTextTimeCOMA.getText().toString();
        SharedPreferences msharedPreferences = getSharedPreferences("comassistant", Context.MODE_PRIVATE);
        try {
            ByteArrayOutputStream baos = new ByteArrayOutputStream();
            ObjectOutputStream oos = new ObjectOutputStream(baos);
            oos.writeObject(AssistData);
            String sBase64 = new String(Base64.encode(baos.toByteArray(), 0));
            SharedPreferences.Editor editor = msharedPreferences.edit();
            editor.putString("AssistData", sBase64);
            editor.commit();
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    //----------------------------------------------------
    private AssistBean getAssistData() {
        SharedPreferences msharedPreferences = getSharedPreferences("comassistant", Context.MODE_PRIVATE);
        AssistBean AssistData = new AssistBean();
        try {
            String personBase64 = msharedPreferences.getString("AssistData", "");
            byte[] base64Bytes = Base64.decode(personBase64.getBytes(), 0);
            if (base64Bytes != null) {
                ByteArrayInputStream bais = new ByteArrayInputStream(base64Bytes);
                ObjectInputStream ois = new ObjectInputStream(bais);
                AssistData = (AssistBean) ois.readObject();
            }
            return AssistData;
        } catch (IOException e) {
            e.printStackTrace();
        } catch (Exception e) {
            e.printStackTrace();
        }
        return AssistData;
    }

    //----------------------------------------------------设置自动发送延时
    private void setDelayTime(TextView v) {
        if (v == editTextTimeCOMA) {
            AssistData.sTimeA = v.getText().toString();
            SetiDelayTime(v.getText().toString());
        }
    }

    //----------------------------------------------------设置自动发送数据
    private void setSendData(TextView v) {
        if (v == editTextCOMA) {
            AssistData.setSendA(v.getText().toString());
            SetLoopData(v.getText().toString());
        }
    }

    //----------------------------------------------------设置自动发送延时
    private void SetiDelayTime(String sTime) {
        //ComPort.setiDelay(Integer.parseInt(sTime));
    }

    //----------------------------------------------------设置自动发送数据
    private void SetLoopData(String sLoopData) {
        if (radioButtonTxt.isChecked()) {
            //ComPort.setTxtLoopData(sLoopData);
        } else if (radioButtonHex.isChecked()) {
            //ComPort.setHexLoopData(sLoopData);
        }
    }

    //----------------------------------------------------显示接收数据
    private void DispRecData(ComBean ComRecData) {
        StringBuilder sMsg = new StringBuilder();
        //sMsg.append(ComRecData.sRecTime);
        //sMsg.append("[");
        //sMsg.append(ComRecData.sComPort);
        //sMsg.append(String.format("](%d)",ComRecData.bRec.length));
        if (ComRecData.bRec[0] == 0X24) {
            try {
                //sMsg.append("[Str] ");
                sMsg.append(new String(ComRecData.bRec, "UTF-8"));
            } catch (UnsupportedEncodingException e) {
                e.printStackTrace();
            }
        } else {
            if (radioButtonTxt.isChecked()) {
                //sMsg.append("[Txt] ");
                try {
                    sMsg.append(new String(ComRecData.bRec, "UTF-8"));
                } catch (UnsupportedEncodingException e) {
                    e.printStackTrace();
                }
            } else if (radioButtonHex.isChecked()) {
                sMsg.append("[Hex] ");
                sMsg.append(MyFunc.ByteArrToHex(ComRecData.bRec));
            }
        }
        //sMsg.append("\r\n");
        editTextRecDisp.append(sMsg);
        //Log.e("xxx显示数据：", sMsg.toString());
        iRecLines++;
        editTextLines.setText(String.valueOf(iRecLines));
        if ((iRecLines > 500) && (checkBoxAutoClear.isChecked())) {//达到500项自动清除
            editTextRecDisp.setText("");
            editTextLines.setText("0");
            iRecLines = 0;
        }
    }

    private void logData(byte[] data,int len){
        StringBuilder sMsg = new StringBuilder();
        if (data[0] == 0x24) {
            try {
                //sMsg.append("[Str] ");
                sMsg.append(new String(data, "UTF-8"));
            } catch (UnsupportedEncodingException e) {
                e.printStackTrace();
            }
        } else {
            if (radioButtonTxt.isChecked()) {
                //sMsg.append("[Txt] ");
                try {
                    sMsg.append(new String(data, "UTF-8"));
                } catch (UnsupportedEncodingException e) {
                    e.printStackTrace();
                }
            } else if (radioButtonHex.isChecked()) {
                sMsg.append("[Hex] ");
                sMsg.append(MyFunc.ByteArrToHex(data));
            }
        }
        //sMsg.append("\r\n");
        editTextRecDisp.append(sMsg);
        //Log.e("xxx显示数据：", sMsg.toString());
        iRecLines++;
        editTextLines.setText(String.valueOf(iRecLines));
        if ((iRecLines > 500) && (checkBoxAutoClear.isChecked())) {//达到500项自动清除
            editTextRecDisp.setText("");
            editTextLines.setText("0");
            iRecLines = 0;
        }
    }

    private void log(String msg){
        editTextRecDisp.append(String.format("\r\n%s\r\n",msg));
        Log.i("TS",msg);
    }

    //----------------------------------------------------设置自动发送模式开关
    private void SetAutoSend(boolean isAutoSend) {
        if (isAutoSend) {
            //ComPort.startSend();
        } else {
            //ComPort.stopSend();
        }
    }

    //----------------------------------------------------串口发送
    private void sendData(String sOut) {
        if(mSerialFd > 0){
            if(radioButtonHex.isChecked()){
                byte[] data = sOut.getBytes();
                serial.serial_write(mSerialFd,data,data.length);
            }else{
                serial.serial_writeHex(mSerialFd,sOut);
            }
        }
    }

    //----------------------------------------------------关闭串口
    private void CloseComPort() {
        if(mSerialFd > 0){
            log(String.format("关闭串口%d",mSerialFd));
            serial.serial_close(mSerialFd);
            mSerialFd = 0;
        }
    }

    //----------------------------------------------------打开串口
    private void OpenComPort(String port) {
        mPort = port;
        if(mSerialFd > 0){
            //串口已经打开
        }else{
            mSerialFd = serial.serial_open(mPort);
            if (mSerialFd > 0) {
                log("打开串口成功！");
                serial.serial_readloop(mSerialFd,100);
            } else {
                log("打开串口失败！请查看串口是否存在");
            }
        }
    }

    //------------------------------------------显示消息
    private void ShowMessage(String sMsg) {
        //Toast.makeText(this, sMsg, Toast.LENGTH_SHORT).show();
        log(sMsg);
    }

    public class NotifyReceiver extends BroadcastReceiver {
        @Override
        public void onReceive(Context context, Intent intent) {
            if (intent.getAction().equals(bdSerialReciveLog)){
                String data = intent.getStringExtra("data");
                log(data);
            }else if(intent.getAction().equals(bdSerialReciveData)){
                byte[] data = intent.getByteArrayExtra("data");
                logData(data,data.length);
            }
        }
    }

}
