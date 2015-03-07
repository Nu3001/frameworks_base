package com.android.internal.car.can;

import android.content.Intent;
import android.content.Context;
import android.os.Handler;
import android.os.Bundle;
import android.os.UserHandle;
import android.util.Slog;

public class CanRadio {
    private static final String TAG = "Radio";
    private static final boolean LOCAL_LOGD = false;

    private static final String EXTRA_RADIO_BAND = "radio_type";
    private static final String EXTRA_RADIO_FREQ = "radio_freq";
    public static final int BAND_FM = 0x00;
    public static final int BAND_FM1 = 0x01;
    public static final int BAND_FM2 = 0x02;
    public static final int BAND_FM3 = 0x03;
    public static final int BAND_AM = 0x10;
    public static final int BAND_AM1 = 0x11;
    public static final int BAND_AM2 = 0x12;
    public static final int BAND_AM3 = 0x13;
    public static final int BAND_MW = 0x20;
    public static final int BAND_MW1 = 0x21;
    public static final int BAND_MW2 = 0x22;
    public static final int BAND_MW3 = 0x23;
    public static final int BAND_LW = 0x30;
    public static final int BAND_LW1 = 0x31;
    public static final int BAND_LW2 = 0x32;
    public static final int BAND_LW3 = 0x33;
    public static final int BAND_MIN = BAND_FM;
    public static final int BAND_MAX = BAND_LW3;
    
    private Context mContext;
    private Handler mHandler;
    private int mBand;
    private int mFreq;
    private RadioRunnable mRadioRunnable = null;

    private class RadioRunnable implements Runnable {
        @Override
        public void run() {
            Intent intent = new Intent(CanBusManager.ACTION_SEND);
            byte[] data = new byte[6];
            data[0] = (byte)0xC2;
            data[1] = (byte)0x04;
            data[2] = (byte)mBand;
            data[3] = (byte)(mFreq & 0xFF);
            data[4] = (byte)((mFreq & 0xFF00)>>8);
            data[5] = (byte)0x00;
            intent.putExtra(CanBusManager.EXTRA_SEND_DATA, data);
            mContext.sendBroadcastAsUser(intent, UserHandle.CURRENT);
            if (LOCAL_LOGD){
                Slog.d(TAG, "broadcast RadioRunnable. CanRadio --- band:" + mBand + " freq:" + mFreq);
            }
        }

    }

    public CanRadio(Context context) {
        mContext = context;
        mHandler = new Handler();
        mRadioRunnable = new RadioRunnable();
    }

    public CanRadio(Context context, int band, int freq){
        this(context);
        mBand = band;
        mFreq = freq;
    }

    public void setRadioBand(int band){
        mBand = band;
    }

    public int getRadioBand(){
        return mBand;
    }

    public void setRadioFreq(int freq){
        mFreq = freq;
    }

    public int getRadioFreq(){
        return mFreq;
    }

    public void sendRadioInfo(int band, int freq){
        if((band < BAND_MIN) || (band > BAND_MAX) || (mRadioRunnable == null)){
            return;
        }
        setRadioBand(band);
        setRadioFreq(freq);
        mHandler.post(mRadioRunnable);
    }

    public void controlRadioDisplay(boolean isDisplay) {
        Intent intent = new Intent(CanBusManager.ACTION_SEND);
        byte[] data = new byte[4];
        data[0] = (byte)0xC0;
        data[1] = (byte)0x02;
        if(isDisplay){
            data[2] = (byte)CanBusManager.SOURCE_TUNER;
            data[3] = (byte)CanBusManager.SOURCE_MEDIA_TYPE_TUNER;
        }else{
            data[2] = (byte)CanBusManager.SOURCE_OFF;
            data[3] = (byte)CanBusManager.SOURCE_MEDIA_TYPE_TUNER;
        }
        intent.putExtra(CanBusManager.EXTRA_SEND_DATA, data);
        mContext.sendBroadcastAsUser(intent, UserHandle.CURRENT);
        if (LOCAL_LOGD){
            Slog.d(TAG, "broadcast RadioRunnable. CanRadio --- isDisplay:" + isDisplay);
        }
    }

    public String toString(){
        return ("band : " + mBand + " freq : " + mFreq);
    }
}