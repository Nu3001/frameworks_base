/*
 * Copyright (C) 2014 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.android.internal.car.can;

import android.content.Intent;
import android.content.Context;
import android.os.Handler;
import android.os.Bundle;
import android.os.UserHandle;
import android.util.Slog;

public class Sonata8 {
    private static final String TAG = "Sonata8";
    private static final boolean LOCAL_LOGD = true;
    public static final String BUNDLE_NAME = "sonata8_bundle";
    public static final String ACTION_SHOW = "com.android.internal.car.can";
    public static final String EXTRA_SHOW = "show";
    /*
     * @
     */
    public static final String ACTION_CANBUS_SONATA8 = "com.bonovo.canbus.Sonata8";

    /*
     * @ The key that get the balance volume of front and rear through
     * intent
     */
    public static final String KEY_VOLUME_FRONT_AND_REAR = "volume_front_rear";
    
    /*
     * @ The key that get the balance volume of left and right through
     * intent
     */
    public static final String KEY_VOLUME_LEFT_AND_RIGHT = "volume_left_right";

    /*
     * @ The key that get the EQ of bass volume through
     * intent
     */
    public static final String KEY_EQ_BASS = "eq_bass";

    /*
     * @ The key that get the EQ of mid volume through
     * intent
     */
    public static final String KEY_EQ_MID = "eq_mid";

    /*
     * @ The key that get the EQ of treble volume through
     * intent
     */
    public static final String KEY_EQ_TREBLE = "eq_treble";
    
    /*
     * @ The key that get the S8 volume through
     * intent
     */
    public static final String KEY_S8_VOLUME = "s8_volume";

    private int mBalanceFrontAndRear = 0; // balance volume of front and rear
    
    private int mBalanceLeftAndRight = 0; // balance volume of left and right
    
    private int mVolumeEQBass = 0; // EQ of bass volume
    
    private int mVolumeEQMid = 0; // EQ of mid volume
    
    private int mVolumeEQTreble = 0; // EQ of treble volume
    
    private int mVolume = 0; // s8 volume

    public Sonata8() {
    }

    public Sonata8(int frontRear, int leftRight, int eqBass, int eqMid,
            int eqTreble, int volume) {
        mBalanceFrontAndRear = frontRear;
        mBalanceLeftAndRight = leftRight;
        mVolumeEQBass = eqBass;
        mVolumeEQMid = eqMid;
        mVolumeEQTreble = eqTreble;
        mVolume = volume;
    }

    // get functions
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

    // set functions
    public void setBalanceFrontAndRear(int volume) {
        mBalanceFrontAndRear = volume;
        if(LOCAL_LOGD)Slog.v(TAG,"myu Class-->Sonata8 setBalanceFrontAndRear() mBalanceFrontAndRear ="+mBalanceFrontAndRear);
    }

    public void setBalanceLeftAndRight(int volume) {
        mBalanceLeftAndRight = volume;
        if(LOCAL_LOGD)Slog.v(TAG,"myu Class-->Sonata8 setBalanceLeftAndRight() mBalanceLeftAndRight ="+mBalanceLeftAndRight);
    }

    public void setVolumeEQBass(int volume) {
        mVolumeEQBass = volume;
        if(LOCAL_LOGD)Slog.v(TAG,"myu Class-->Sonata8 setVolumeEQBass()mVolumeEQBass ="+mVolumeEQBass);
    }

    public void setVolumeEQMid(int volume) {
        mVolumeEQMid = volume;
        if(LOCAL_LOGD)Slog.v(TAG,"myu Class-->Sonata8 setVolumeEQMid() mVolumeEQMid="+mVolumeEQMid);
    }

    public void setVolumeEQTreble(int volume) {
        mVolumeEQTreble = volume;
        if(LOCAL_LOGD)Slog.v(TAG,"myu Class-->Sonata8 setVolumeEQTreble() mVolumeEQTreble ="+mVolumeEQTreble);
    }
    
    public void setVolume(int volume) {
    	mVolume = volume;
        if(LOCAL_LOGD)Slog.v(TAG,"myu Class-->Sonata8 setVolume() mVolume ="+mVolume);
    }

    public String toString() {
        return ("FrontAndRear:" + mBalanceFrontAndRear + " LeftAndRightt:" + mBalanceLeftAndRight
                + " EQBass:" + mVolumeEQBass + " EQMid:" + mVolumeEQMid
                + " VolumeEQTreblet:" + mVolumeEQTreble + " S8Volume:"+ mVolume);
    }

    public Bundle toBundle() {
        final Bundle bundle = new Bundle();
        if(LOCAL_LOGD)Slog.v(TAG,"myu Class-->Sonata8 toBundle()");
        bundle.putInt(KEY_VOLUME_FRONT_AND_REAR, mBalanceFrontAndRear);
        bundle.putInt(KEY_VOLUME_LEFT_AND_RIGHT, mBalanceLeftAndRight);
        bundle.putInt(KEY_EQ_BASS, mVolumeEQBass);
        bundle.putInt(KEY_EQ_MID, mVolumeEQMid);
        bundle.putInt(KEY_EQ_TREBLE, mVolumeEQTreble);
        bundle.putInt(KEY_S8_VOLUME, mVolume);
        return bundle;
    }

    public static Sonata8 bundle2Radar(final Bundle bundle) {
        return new Sonata8(
                bundle.getInt(KEY_VOLUME_FRONT_AND_REAR),
                bundle.getInt(KEY_VOLUME_LEFT_AND_RIGHT),
                bundle.getInt(KEY_EQ_BASS),
                bundle.getInt(KEY_EQ_MID),
                bundle.getInt(KEY_EQ_TREBLE),
                bundle.getInt(KEY_S8_VOLUME));
    }
}
