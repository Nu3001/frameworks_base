package com.android.internal.car.can;

import android.os.Bundle;
import android.util.Slog;

public class Sonata8 {
    public static final String ACTION_CANBUS_SONATA8 = "com.bonovo.canbus.Sonata8";
    public static final String ACTION_SHOW = "com.android.internal.car.can";
    public static final String BUNDLE_NAME = "sonata8_bundle";
    public static final String EXTRA_SHOW = "show";
    public static final String KEY_EQ_BASS = "eq_bass";
    public static final String KEY_EQ_MID = "eq_mid";
    public static final String KEY_EQ_TREBLE = "eq_treble";
    public static final String KEY_S8_VOLUME = "s8_volume";
    public static final String KEY_VOLUME_FRONT_AND_REAR = "volume_front_rear";
    public static final String KEY_VOLUME_LEFT_AND_RIGHT = "volume_left_right";
    private static final boolean LOCAL_LOGD = true;
    private static final String TAG = "Sonata8";
    private int mBalanceFrontAndRear = 0;
    private int mBalanceLeftAndRight = 0;
    private int mVolumeEQBass = 0;
    private int mVolumeEQMid = 0;
    private int mVolumeEQTreble = 0;
    private int mVolume = 0;
    
    public Sonata8() {
    }
    
    public Sonata8(int frontRear, int leftRight, int eqBass, int eqMid, int eqTreble, int volume) {
        mBalanceFrontAndRear = frontRear;
        mBalanceLeftAndRight = leftRight;
        mVolumeEQBass = eqBass;
        mVolumeEQMid = eqMid;
        mVolumeEQTreble = eqTreble;
        mVolume = volume;
    }
    
    public int getBalanceFrontAndRear() {
        return mBalanceFrontAndRear;
    }
    
    public int getBalanceLeftAndRight() {
        return mBalanceLeftAndRight;
    }
    
    public int getVolumeEQBass() {
        return mVolumeEQBass;
    }
    
    public int getVolumeEQMid() {
        return mVolumeEQMid;
    }
    
    public int getVolumeEQTreble() {
        return mVolumeEQTreble;
    }
    
    public int getVolume() {
        return mVolume;
    }
    
    public void setBalanceFrontAndRear(int volume) {
        mBalanceFrontAndRear = volume;
        Slog.v("Sonata8", "myu Class-->Sonata8 setBalanceFrontAndRear() mBalanceFrontAndRear =" + mBalanceFrontAndRear);
    }
    
    public void setBalanceLeftAndRight(int volume) {
        mBalanceLeftAndRight = volume;
        Slog.v("Sonata8", "myu Class-->Sonata8 setBalanceLeftAndRight() mBalanceLeftAndRight =" + mBalanceLeftAndRight);
    }
    
    public void setVolumeEQBass(int volume) {
        mVolumeEQBass = volume;
        Slog.v("Sonata8", "myu Class-->Sonata8 setVolumeEQBass()mVolumeEQBass =" + mVolumeEQBass);
    }
    
    public void setVolumeEQMid(int volume) {
        mVolumeEQMid = volume;
        Slog.v("Sonata8", "myu Class-->Sonata8 setVolumeEQMid() mVolumeEQMid=" + mVolumeEQMid);
    }
    
    public void setVolumeEQTreble(int volume) {
        mVolumeEQTreble = volume;
        Slog.v("Sonata8", "myu Class-->Sonata8 setVolumeEQTreble() mVolumeEQTreble =" + mVolumeEQTreble);
    }
    
    public void setVolume(int volume) {
        mVolume = volume;
        Slog.v("Sonata8", "myu Class-->Sonata8 setVolume() mVolume =" + mVolume);
    }
    
    public String toString() {
        return "FrontAndRear:" + mBalanceFrontAndRear + " LeftAndRightt:" + mBalanceLeftAndRight + " EQBass:" + mVolumeEQBass + " EQMid:" + mVolumeEQMid + " VolumeEQTreblet:" + mVolumeEQTreble + " S8Volume:" + mVolume;
    }
    
    public Bundle toBundle() {
        Bundle bundle = new Bundle();
        Slog.v("Sonata8", "myu Class-->Sonata8 toBundle()");
        bundle.putInt("volume_front_rear", mBalanceFrontAndRear);
        bundle.putInt("volume_left_right", mBalanceLeftAndRight);
        bundle.putInt("eq_bass", mVolumeEQBass);
        bundle.putInt("eq_mid", mVolumeEQMid);
        bundle.putInt("eq_treble", mVolumeEQTreble);
        bundle.putInt("s8_volume", mVolume);
        return bundle;
    }
    
    public static Sonata8 bundle2Radar(Bundle bundle) {
        return new Sonata8(getInt("volume_front_rear"), getInt("volume_left_right"), getInt("eq_bass"), getInt("eq_mid"), getInt("eq_treble"), getInt("s8_volume"));
    }
}
