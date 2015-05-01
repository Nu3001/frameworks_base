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
import android.content.SharedPreferences;
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
import com.android.internal.car.can.Sonata8;

import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.PrintWriter;
import java.io.FileDescriptor;


class CanBusService extends Binder {
    private static final String TAG = CanBusService.class.getSimpleName();
    private static final boolean LOCAL_LOGV = false;
    private static final boolean LOCAL_LOGD = false;
    private static final boolean IF_NOTIFICATION = false;
    private final int DEFAULT_CARTYPE = 0;//cartype default = 0(Volkswagen)
    private final int DEFAULT_SERIALTYPE = 0;//cartype default = 0(Non)
    private final int SERIALTYPE_OBD = 1;	//serial typ-->OBD
    private final int SERIALTYPE_CAN = 2;	//serial typ-->CAN
    private final int SERIALTYPE_NON = 0;	//serial typ-->NON
    private final int CARTYPE_VOLKEWAGEN = 0;	//car typ-->Volkswagen
    private final int CARTYPE_SONATA8 = 0;	//car typ-->Sonata8

    private int mCarType = 0; // the car type which is being used. 0:Volkswagen 1:Sonata8  default = 0(Volkswagen)
    private int mSerialType = 0;//the serial type which is being used. 1:OBD 2:CAN 0:Non	default = 0(Non)
    private int mVolumeS8 = 0;
    File carTypeFile = new File("/data/system/cartype.txt");		//save of the car type file
    File serialTypeFile = new File("/data/system/serial.txt");		//save of the car type file
    
    private int frontRear = 0;//Sonata8 Volume Balance of front and rear
    private int leftRight = 0;//Sonata8 Volume Balance of left and right
    private int eqBass = 0;//Sonata8 Volume EQ of Bass
    private int eqMid = 0;//Sonata8 Volume EQ of Mid
    private int eqTreble = 0;//Sonata8 Volume EQ of Treble
    private int s8Volume = 0;//Sonata8 Volume

    private final Context mContext;
    private SharedPreferences preferences;
    private Handler mHandler;
    private Radar mRadar;
    private AirConditioning mAirConditioningCache;
    private CarDoor mCarDoor;
    private Sonata8 mSonata8; 
    private SendSonata8Runnable mSendSonata8Runnable;
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
    
    private class SendSonata8Runnable implements Runnable {
        @Override
        public void run() {
//            if (true) {
//                final Intent intent = new Intent(CanBusManager.ACTION_RECEINVED);	//SEND S8 ACTION_RECEINVED Broadcast
//                intent.addCategory(CanBusManager.CATEGORY_SONATA8);
//                intent.putExtra(Sonata8.BUNDLE_NAME, mSonata8.toBundle());
//                mContext.sendBroadcastAsUser(intent, UserHandle.CURRENT);
//            } else {
//                final Intent activityIntent = new Intent();
//                activityIntent.setClassName("com.bonovo.soundbalance",
//                        "com.bonovo.soundbalance.SonataActivity");
//                activityIntent.setFlags(Intent.FLAG_ACTIVITY_SINGLE_TOP);
//                activityIntent.putExtra(Sonata8.BUNDLE_NAME, mSonata8.toBundle());
//                mContext.startActivityAsUser(activityIntent, UserHandle.CURRENT);
//            }
//            if (LOCAL_LOGD){
//                //Slog.d(TAG, "broadcast SendSonata8Runnable. info :\n" + mSonata8.toString());
//            }
        }

    }

    private CanRequestReceiver mCanRequestReceiver;

	private class CanRequestReceiver extends BroadcastReceiver {
		@Override
		public void onReceive(Context context, Intent intent) {
			String action = intent.getAction();
			if (action.equals(CanBusManager.ACTION_SEND)) {
				final byte[] data = intent
						.getByteArrayExtra(CanBusManager.EXTRA_SEND_DATA);
				if (data == null)
					return;
				if (LOCAL_LOGD) {
					Slog.d(TAG, "send can request : " + new String(data));
				}
				new SendToCanTask().execute(data);
			} else if (action.equals(CanBusManager.ACTION_CAR_TYPE_REQUEST)) {		//CAR_TYPE_REQUEST
				try {
					FileInputStream inputStream = new FileInputStream(carTypeFile);
					DataInputStream dataInputStream = new DataInputStream(inputStream);
					mCarType = dataInputStream.readInt();
					if (LOCAL_LOGD)
						Slog.d(TAG, "CanBusService receiver REQUEST-->carTypeFile =" + mCarType);
					dataInputStream.close();
					inputStream.close();
				} catch (Exception e) {
					// TODO: handle exception
					Slog.d(TAG, "CanBusService receiver REQUEST-->carTypeFile nonono");
				}
				final Intent mintent = new Intent(
						CanBusManager.ACTION_CAR_TYPE_RESPONSE);
				mintent.addCategory(CanBusManager.CATEGORY_CAR_TYPE);
				mintent.putExtra(CanBusManager.EXTRA_CAR_TYPE, mCarType);
				mContext.sendBroadcastAsUser(mintent, UserHandle.CURRENT);
			} else if (action.equals(CanBusManager.ACTION_CAR_TYPE_CHANGED)) {		//CAR_TYPE_CHANGED
				mCarType = intent.getIntExtra(CanBusManager.EXTRA_CAR_TYPE,
						DEFAULT_CARTYPE);
//				mVolumeS8 = intent.getIntExtra("s8_volume",0);
				if (LOCAL_LOGD)Slog.d(TAG, "------------->CAR_TYPE_CHANGED");
				//send broadcast for HandlerService to tell carType changed
//				Intent sendForHandlerIntent = new Intent("android.intent.action.SEND_FOR_HANDLER_CAR_TYPE");
//				sendForHandlerIntent.putExtra(CanBusManager.HANDLER_CAR_TYPE, mCarType);
//				mContext.sendBroadcastAsUser(sendForHandlerIntent, UserHandle.CURRENT);
				
				native_setCarType(mCarType);
//				native_setVol((byte)mVolumeS8);
				if(mCarType == 0){
					//native_setPower((byte)0);
				}else if (mCarType == 1) {
					native_setPower((byte)1);
					try {
						Thread.sleep(2000);
					} catch (Exception e) {
						// TODO: handle exception
					}
//					native_readS8Info((byte)1);
					//when power on ,send broadcast to request S8Info()
					Intent requestS8InfoIntent = new Intent("request_s8_info");
					mContext.sendBroadcastAsUser(requestS8InfoIntent, UserHandle.CURRENT);
				}
				
//				try {
//					FileOutputStream outputStream = new FileOutputStream(
//							carTypeFile);
//					DataOutputStream dataOutputStream = new DataOutputStream(
//							outputStream);
//					if (LOCAL_LOGD)
//						Slog.d(TAG,
//								"CanBusService receiver CHANGED-->carTypeFile ="
//										+ mCarType);
//					dataOutputStream.writeInt(mCarType);
//					dataOutputStream.close();
//					outputStream.close();
//				} catch (Exception e) {
//					// TODO: handle exception
//					Slog.d(TAG,
//							"CanBusService receiver CHANGED-->carTypeFile nonono");
//				}

			} else if (action.equals(CanBusManager.ACTION_SERIAL_TYPE_REQUEST)) {	//SERIAL_TYPE_REQUEST
				try {
					FileInputStream inputStream = new FileInputStream(serialTypeFile);
					DataInputStream dataInputStream = new DataInputStream(inputStream);
					mSerialType = dataInputStream.readInt();
					if (LOCAL_LOGD)
						Slog.d(TAG, "CanBusService receiver REQUEST-->mSerialType ="+mSerialType);
					dataInputStream.close();
					inputStream.close();
				} catch (Exception e) {
					// TODO: handle exception
					Slog.d(TAG, "CanBusService receiver REQUEST-->mSerialType nonono");
				}
				final Intent mintent = new Intent(
						CanBusManager.ACTION_SERIAL_TYPE_RESPONSE);
				mintent.addCategory(CanBusManager.CATEGORY_SERIAL_TYPE);
				mintent.putExtra(CanBusManager.EXTRA_SERIAL_TYPE, mSerialType);
				mContext.sendBroadcastAsUser(mintent, UserHandle.CURRENT);
			}else if (action.equals(CanBusManager.ACTION_SERIAL_TYPE_CHANGED)) {	//SERIAL_TYPE_CHANGED
				mSerialType = intent.getIntExtra(CanBusManager.EXTRA_SERIAL_TYPE, DEFAULT_SERIALTYPE);
				if (LOCAL_LOGD)Slog.d(TAG, "----------->SERIAL_TYPE_CHANGED");
//				mVolumeS8 = intent.getIntExtra("s8_volume",0);
				
				native_setMcuUartFunc(mSerialType);
				
				if(mSerialType == SERIALTYPE_NON){
					native_setUART38400(0);
				}else if (mSerialType == SERIALTYPE_OBD) {
				}else if (mSerialType == SERIALTYPE_CAN) {
					native_setUART38400(384);
//					native_readS8Info((byte)1);
				}
				
//				try {
//					FileOutputStream outputStream = new FileOutputStream(
//							serialTypeFile);
//					DataOutputStream dataOutputStream = new DataOutputStream(
//							outputStream);
//					if (LOCAL_LOGD)
//						Slog.d(TAG, "CanBusService receiver CHANGED-->mSerialType ="+mSerialType);
//					dataOutputStream.writeInt(mSerialType);
//					dataOutputStream.close();
//					outputStream.close();
//					
//				} catch (Exception e) {
//					// TODO: handle exception
//					Slog.d(TAG, "CanBusService receiver CHANGED-->mSerialType nonono");
//				}
			}else if (action.equals("android.intent.action.BONOVO_SLEEP_KEY")) {
					if(mSerialType == SERIALTYPE_CAN){
						//native_setPower((byte)0);
					}
			}
		}
	}
	
	private Sonata8RequestReceiver mSonata8RequestReceiver;
	
	private class Sonata8RequestReceiver extends BroadcastReceiver {
		@Override
		public void onReceive(Context context, Intent intent) {
			String action = intent.getAction();
			if(action.equals(CanBusManager.ACTION_BALANCE_FRONT_AND_REAR)){		//set BALANCE_FRONT_AND_REAR
				frontRear = intent.getIntExtra(CanBusManager.EXTRA_VOLUME_FRONT_AND_REAR, 0);
				native_setFadVal((byte)frontRear);
				if (LOCAL_LOGD)
					Slog.d(TAG, "Sonata8RequestReceiver  frontRear = "+frontRear);
			}else if (action.equals(CanBusManager.ACTION_BALANCE_LEFT_AND_RIGHT)) {	//set BALANCE_LEFT_AND_RIGHT
				leftRight = intent.getIntExtra(CanBusManager.EXTRA_VOLUME_LEFT_AND_RIGHT, 0);
				native_setBalVal((byte)leftRight);
				if (LOCAL_LOGD)
					Slog.d(TAG, "Sonata8RequestReceiver  leftRight = "+leftRight);
			}else if (action.equals(CanBusManager.ACTION_EQ_BASS)) {			//set EQ_BASS
				eqBass = intent.getIntExtra(CanBusManager.EXTRA_EQ_BASS, 0);
				native_setBass((byte)eqBass);
				if (LOCAL_LOGD)
					Slog.d(TAG, "Sonata8RequestReceiver  eqBass = "+eqBass);
			}else if (action.equals(CanBusManager.ACTION_EQ_MID)) {				//set EQ_MID
				eqMid = intent.getIntExtra(CanBusManager.EXTRA_EQ_MID, 0);
				native_setMid((byte)eqMid);
				if (LOCAL_LOGD)
					Slog.d(TAG, "Sonata8RequestReceiver  eqMid = "+eqMid);
			}else if (action.equals(CanBusManager.ACTION_EQ_TREBLE)) {			//set EQ_TREBLE
				eqTreble = intent.getIntExtra(CanBusManager.EXTRA_EQ_TREBLE, 0);
				native_setTre((byte)eqTreble);
				if (LOCAL_LOGD)
					Slog.d(TAG, "Sonata8RequestReceiver  eqTreble = "+eqTreble);
			}else if (action.equals(CanBusManager.ACTION_S8_READINFO)) {		//S8_READINFO
				native_readS8Info((byte)1);
			}else if (action.equals(CanBusManager.ACTION_S8_VOLUME_CHANGED)) {	//S8_VOLUME_CHANGED
				s8Volume = intent.getIntExtra(CanBusManager.EXTRA_S8_VOLUME,15);
				if (LOCAL_LOGD)
					Slog.d(TAG, "Receive-->Sonata8RequestReceiver s8Volume = "+s8Volume);
				native_setVol((byte)s8Volume);
			}
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
		//read the cartype.txt
//		try {
//			FileInputStream inputStream = new FileInputStream(carTypeFile);
//			DataInputStream dataInputStream = new DataInputStream(inputStream);
//			mCarType = dataInputStream.readInt();
//			if (LOCAL_LOGD)
//				Slog.d(TAG, "CanBusService constructor -->mCarType ="+mCarType);
//			dataInputStream.close();
//			inputStream.close();
//		} catch (Exception e) {
//			// TODO: handle exception
//			Slog.d(TAG, "CanBusService constructor nonono-->mCarType="+mCarType);
//		}
//		
//		//read the serialtype.txt
//		try {
//			FileInputStream inputStream = new FileInputStream(serialTypeFile);
//			DataInputStream dataInputStream = new DataInputStream(inputStream);
//			mSerialType = dataInputStream.readInt();
//			if (LOCAL_LOGD)
//				Slog.d(TAG, "CanBusService constructor -->mSerialType ="+mSerialType);
//			dataInputStream.close();
//			inputStream.close();
//		} catch (Exception e) {
//			// TODO: handle exception
//			Slog.d(TAG, "CanBusService constructor nonono-->mSerialType="+mSerialType);
//		}
		mSendAirConditioningRunnable = new SendAirConditioningRunnable();
		mSendRadarRunnable = new SendRadarRunnable();
		mSendSonata8Runnable = new SendSonata8Runnable();
		mAirConditioningCache = new AirConditioning();
		mSendCarDoorRunnable = new SendCarDoorRunnable();
		mRadar = new Radar();
		mCarDoor = new CarDoor();
		mSonata8 = new Sonata8();
		mCanRequestReceiver = new CanRequestReceiver();
		final IntentFilter intentFilter = new IntentFilter();
		intentFilter.addAction(CanBusManager.ACTION_SEND);
		intentFilter.addAction(CanBusManager.ACTION_CAR_TYPE_RESPONSE);
		intentFilter.addAction(CanBusManager.ACTION_CAR_TYPE_REQUEST);
		intentFilter.addAction(CanBusManager.ACTION_CAR_TYPE_CHANGED);
		intentFilter.addAction(CanBusManager.ACTION_SERIAL_TYPE_RESPONSE);
		intentFilter.addAction(CanBusManager.ACTION_SERIAL_TYPE_REQUEST);
		intentFilter.addAction(CanBusManager.ACTION_SERIAL_TYPE_CHANGED);
		intentFilter.addCategory(CanBusManager.CATEGORY_AIRCONDITIONING);
		intentFilter.addCategory(CanBusManager.CATEGORY_RADAR);
		intentFilter.addCategory(CanBusManager.CATEGORY_RADIO);
		intentFilter.addCategory(CanBusManager.CATEGORY_SONATA8);
		context.registerReceiver(mCanRequestReceiver, intentFilter);

		mRadarShowReceiver = new RadarShowReceiver();
		final IntentFilter radarShowFilter = new IntentFilter();
		radarShowFilter.addAction(Radar.ACTION_SHOW);
		context.registerReceiver(mRadarShowReceiver, radarShowFilter);
		
		mSonata8RequestReceiver = new Sonata8RequestReceiver();
		final IntentFilter sonata8Filter = new IntentFilter();
		sonata8Filter.addAction(CanBusManager.ACTION_BALANCE_FRONT_AND_REAR);
		sonata8Filter.addAction(CanBusManager.ACTION_BALANCE_LEFT_AND_RIGHT);
		sonata8Filter.addAction(CanBusManager.ACTION_EQ_BASS);
		sonata8Filter.addAction(CanBusManager.ACTION_EQ_MID);
		sonata8Filter.addAction(CanBusManager.ACTION_EQ_TREBLE);
		sonata8Filter.addAction(CanBusManager.ACTION_S8_READINFO);
		sonata8Filter.addAction(CanBusManager.ACTION_S8_VOLUME_CHANGED);
		context.registerReceiver(mSonata8RequestReceiver, sonata8Filter);

		native_start();
//		if(mSerialType == SERIALTYPE_NON){
//			//Slog.d(TAG, "CanBusService constructor UART38400 UART38400-->NON");
//			native_setMcuUartFunc(SERIALTYPE_NON); //2:CAN 1:OBD 0:NON
//			native_setUART38400(0);
//		}else if (mSerialType == SERIALTYPE_OBD) {
//			//Slog.d(TAG, "CanBusService constructor UART38400 UART38400-->OBD");
//			native_setMcuUartFunc(SERIALTYPE_OBD); //2:CAN 1:OBD 0:NON
//		}else if (mSerialType == SERIALTYPE_CAN) {
//			//Slog.d(TAG, "CanBusService constructor UART38400 UART38400-->CAN");
//			native_setMcuUartFunc(SERIALTYPE_CAN); //2:CAN 1:OBD 0:NON
//			native_setUART38400(384);
//			native_setPower((byte)1);
//		}
		
//		//if carType is Sonata8,set the Power-ON CMD
//		if(mCarType == 1){
//			native_setCarType(1);
//		}else if (mCarType == 0) {
//			native_setCarType(0);
//		}
		//*******************************
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
    private native int native_setMcuUartFunc(int mcu_uart_func_type);
    private native int native_setCarType(int carType);
    private native int native_setFadVal(byte fad_val);
    private native int native_setBalVal(byte bal_val);
    private native int native_setVol(byte volume);
    private native int native_setBass(byte bass_val);
    private native int native_setMid(byte mid_val);
    private native int native_setTre(byte tre_val);
    private native int native_setPower(byte on_off);
    private native int native_readS8Info(byte on_off);
    private native int native_setUART38400(int baud);
	

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
    
    private Sonata8 getMemberSonata8() {
        return mSonata8;
    }

    private void reportRadarInfo() {
        mHandler.post(mSendRadarRunnable);
    }

    private void reportCarDoor() {
        mHandler.post(mSendCarDoorRunnable);
    }
    
    private void reportSonata8() {
        mHandler.post(mSendSonata8Runnable);
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
