/*
 * Copyright (C) 2006 The Android Open Source Project
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

package com.android.server;

import android.content.pm.PackageManager;
import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.AsyncTask;
import android.os.Binder;
import android.os.Handler;
import android.os.UserHandle;
import android.os.ServiceManager;
import android.provider.Settings;
import android.util.EventLog;
import android.util.Slog;

import com.android.internal.car.can.CanBusManager;
import com.android.internal.car.can.CarDoor;
import com.android.internal.car.can.Radar;
import com.android.internal.car.can.AirConditioning;

import java.io.IOException;
import java.io.PrintWriter;
import java.io.FileDescriptor;

class CanBusService extends Binder {
    private static final String TAG = CanBusService.class.getSimpleName();
    private static final boolean LOCAL_LOGV = false;
    private static final boolean LOCAL_LOGD = false;
    private static final boolean IF_NOTIFICATION = false;

    private final Context mContext;
    private Handler mHandler;
    private Radar mRadar;
    private AirConditioning mAirConditioningCache;
    private CarDoor mCarDoor;
    private SendAirConditioningRunnable mSendAirConditioningRunnable;
    private SendRadarRunnable mSendRadarRunnable;
    private SendCarDoorRunnable mSendCarDoorRunnable;

    private class SendAirConditioningRunnable implements Runnable {
        @Override
        public void run() {
            if (IF_NOTIFICATION) {
                final Intent intent = new Intent(CanBusManager.ACTION_RECEINVED);
                intent.addCategory(CanBusManager.CATEGORY_AIRCONDITIONING);
                intent.putExtra(AirConditioning.BUNDLE_NAME,
                        AirConditioning.airCondition2Bundle(mAirConditioningCache));
                mContext.sendBroadcastAsUser(intent, UserHandle.CURRENT);
            } else {
                final Intent activityIntent = new Intent();
                activityIntent.setClassName("com.newsmy.car.airconditioner",
                        "com.newsmy.car.airconditioner.NewsmyAirConditionerActivity");
                activityIntent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
                activityIntent.putExtra(AirConditioning.BUNDLE_NAME,
                        AirConditioning.airCondition2Bundle(mAirConditioningCache));
                mContext.startActivityAsUser(activityIntent, UserHandle.CURRENT);
            }
            if (LOCAL_LOGD) {
                Slog.d(TAG, "broadcast air condition!!");
                Slog.d(TAG, "info :\n" + mAirConditioningCache.toString());
            }
        }
    }

    private class SendCarDoorRunnable implements Runnable {
        @Override
        public void run() {
            if (IF_NOTIFICATION) {
                // final Intent intent = new
                // Intent(CanBusManager.ACTION_RECEINVED);
                // intent.addCategory(CanBusManager.CATEGORY_AIRCONDITIONING);
                // intent.putExtra(AirConditioning.BUNDLE_NAME,
                // AirConditioning.airCondition2Bundle(mAirConditioningCache));
                // mContext.sendBroadcastAsUser(intent, UserHandle.CURRENT);
            } else {
                final Intent activityIntent = new Intent();
                activityIntent.setClassName("com.newsmy.car.cardoor",
                        "com.newsmy.car.cardoor.MainActivity");
                activityIntent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
                activityIntent.putExtra(CarDoor.BUNDLE_NAME, CarDoor.carDoorToBundle(mCarDoor));
                mContext.startActivityAsUser(activityIntent, UserHandle.CURRENT);
            }
            if (LOCAL_LOGD) {
                Slog.d(TAG, "broadcast car door!!");
                Slog.d(TAG, "info :\n" + mCarDoor.toString());
            }
        }
    }

    private class SendRadarRunnable implements Runnable {
        @Override
        public void run() {
            if (IF_NOTIFICATION) {
                final Intent intent = new Intent(CanBusManager.ACTION_RECEINVED);
                intent.addCategory(CanBusManager.CATEGORY_RADAR);
                intent.putExtra(Radar.BUNDLE_NAME, mRadar.toBundle());
                mContext.sendBroadcastAsUser(intent, UserHandle.CURRENT);
            } else {
                final Intent activityIntent = new Intent();
                activityIntent.setClassName("com.newsmy.car.radar",
                        "com.newsmy.car.radar.NewsmyCarRadarActivity");
                activityIntent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
                activityIntent.putExtra(Radar.BUNDLE_NAME, mRadar.toBundle());
                mContext.startActivityAsUser(activityIntent, UserHandle.CURRENT);
            }
            if (LOCAL_LOGD){
                //Slog.d(TAG, "broadcast SendRadarRunnable!!");
                Slog.d(TAG, "broadcast SendRadarRunnable. info :\n" + mRadar.toString());
            }
        }

    }

    private CanRequestReceiver mCanRequestReceiver;

    private class CanRequestReceiver extends BroadcastReceiver {
        @Override
        public void onReceive(Context context, Intent intent) {
            final byte[] data = intent.getByteArrayExtra(CanBusManager.EXTRA_SEND_DATA);
            if (data == null)
                return;
            if (LOCAL_LOGD) {
                Slog.d(TAG, "send can request : " + new String(data));
            }
            new SendToCanTask().execute(data);
        }
    }

    private class SendToCanTask extends AsyncTask<byte[], Void, Void> {
        @Override
        protected Void doInBackground(byte[]... params) {
            final byte[] data = params[0];
            if (data != null)
                native_sendCommand(data);
            return null;
        }
    }

    private BroadcastReceiver mRadarShowReceiver;
    public CanBusService(Context context) {
        mContext = context;
        mHandler = new Handler();
        mSendAirConditioningRunnable = new SendAirConditioningRunnable();
        mSendRadarRunnable = new SendRadarRunnable();
        mAirConditioningCache = new AirConditioning();
        mSendCarDoorRunnable = new SendCarDoorRunnable();
        mRadar = new Radar();
        mCarDoor = new CarDoor();
        mCanRequestReceiver = new CanRequestReceiver();
        final IntentFilter intentFilter = new IntentFilter();
        intentFilter.addAction(CanBusManager.ACTION_SEND);
        intentFilter.addCategory(CanBusManager.CATEGORY_AIRCONDITIONING);
        intentFilter.addCategory(CanBusManager.CATEGORY_RADAR);
        intentFilter.addCategory(CanBusManager.CATEGORY_RADIO); 
        context.registerReceiver(mCanRequestReceiver, intentFilter);
        
        mRadarShowReceiver = new RadarShowReceiver();
        final IntentFilter radarShowFilter = new IntentFilter();
        radarShowFilter.addAction(Radar.ACTION_SHOW);
        context.registerReceiver(mRadarShowReceiver, radarShowFilter);
        native_start();
    }
    
    private boolean mRadarShowing;
    private class RadarShowReceiver extends BroadcastReceiver {
        @Override
        public void onReceive(Context context, Intent intent) {
            mRadarShowing = intent.getBooleanExtra(Radar.EXTRA_SHOW, false);
        }
    }

    private native boolean native_start();

    private native int native_sendCommand(byte[] data);

    void systemReady() {
        // can opened?
    }

    // ******************************************
    // * about Radar of CanBus
    // ******************************************
    private Radar getMemberRadar() {
        return mRadar;
    }

    private CarDoor getMemberCarDoor() {
        return mCarDoor;
    }

    private void reportRadarInfo() {
        mHandler.post(mSendRadarRunnable);
    }

    private void reportCarDoor() {
        mHandler.post(mSendCarDoorRunnable);
    }

    // ***************************************************
    // * about air condition
    // ***************************************************
    private AirConditioning getMemberAirCondition() {
        return mAirConditioningCache;
    }

    private void reportAirConditioning() {
        if (LOCAL_LOGD)
            Slog.d(TAG, "Can bus , report air conditioning!!!!");
        if (!mRadarShowing /*&& mAirConditioningCache.getAirConditioningDisplaySiwtch()*/)
            mHandler.post(mSendAirConditioningRunnable);
    }

    @Override
    protected void dump(FileDescriptor fd, PrintWriter pw, String[] args) {
        if (mContext.checkCallingOrSelfPermission(android.Manifest.permission.DUMP)
                != PackageManager.PERMISSION_GRANTED) {

            pw.println("Permission Denial: can't dump Battery service from from pid="
                    + Binder.getCallingPid()
                    + ", uid=" + Binder.getCallingUid());
            return;
        }
        pw.println("[can bus]:");
        pw.println("[air condition]:" + mAirConditioningCache.toString());
        pw.println("[radar]:" + mRadar.toString());
    }
//
//    // Air Conditioning
//    private boolean mAirConditioningSwitch;
//    private boolean mACSwitch;
//    private int mCycle;
//    private boolean mAUTOStrongWindSwitch;
//    private boolean mAUTOSoftWindSiwtch;
//    private boolean mDUALSwitch;
//    private boolean mMAXFORNTSwitch;
//    private boolean mREARSwitch;
//    private boolean mUpWindSwitch;
//    private boolean mHorizontalWindSwitch;
//    private boolean mDownWindSwitch;
//    private boolean mAirConditioningDisplaySiwtch;
//    private int mWindLevel;
//    private float mLeftTemp;
//    private float mRightTemp;
//    private boolean mAQSInternalCycleSwitch;
//    private int mLeftSeatHeatingLevel;
//    private boolean mREARLockSwitch;
}
