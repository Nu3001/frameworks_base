
package com.android.internal.car.can;

public class CanBusManager {
    private static final String TAG = "CanBusManager";
    private static final boolean D = false;

    public static final String ACTION_SEND = "com.android.internal.car.can.action.SEND";
    public static final String ACTION_RECEINVED = "com.android.internal.car.can.action.RECEIVED";
    public static final String ACTION_SWITCH_SOURCE = "com.android.internal.car.can.action.SWITCH_SOURCE";
    public static final String ACTION_BALANCE_LEFT_AND_RIGHT = "com.android.internal.car.can.action.ACTION_BALANCE_LEFT_AND_RIGHT";
    public static final String ACTION_CAR_TYPE_CHANGED = "com.android.internal.car.can.action.CAR_TYPE_CHANGED";
    public static final String ACTION_CAR_TYPE_REQUEST = "com.android.internal.car.can.action.CAR_TYPE_REQUEST";
    public static final String ACTION_CAR_TYPE_RESPONSE = "com.android.internal.car.can.action.CAR_TYPE_RESPONSE";
    public static final String ACTION_EQ_BASS = "com.android.internal.car.can.action.ACTION_EQ_BASS";
    public static final String ACTION_EQ_MID = "com.android.internal.car.can.action.ACTION_EQ_MID";
    public static final String ACTION_EQ_TREBLE = "com.android.internal.car.can.action.ACTION_EQ_TREBLE";
    public static final String ACTION_S8_READINFO = "com.android.internal.car.can.action.ACTION_S8_READINFO";
    public static final String ACTION_S8_VOLUME_CHANGED = "com.android.internal.car.can.action.ACTION_S8_VOLUME_CHANGED";
    public static final String ACTION_SERIAL_TYPE_CHANGED = "com.android.internal.car.can.action.SERIAL_TYPE_CHANGED";
    public static final String ACTION_SERIAL_TYPE_REQUEST = "com.android.internal.car.can.action.SERIAL_TYPE_REQUEST";
    public static final String ACTION_SERIAL_TYPE_RESPONSE = "com.android.internal.car.can.action.SERIAL_TYPE_RESPONSE";

    public static final String CATEGORY_AIRCONDITIONING = "com.android.internal.car.can.AIRCONDITIONING";
    public static final String CATEGORY_CAR_TYPE = "com.android.internal.car.can.Car";
    public static final String CATEGORY_RADAR = "com.android.internal.car.can.RADAR";
    public static final String CATEGORY_RADIO = "com.android.internal.car.can.Radio";
    public static final String CATEGORY_SERIAL_TYPE = "com.android.internal.car.can.Serial";
    public static final String CATEGORY_SONATA8 = "com.android.internal.car.can.Sonata8";
	
    public static final String EXTRA_CAR_TYPE = "car_type";
    public static final String EXTRA_EQ_BASS = "eq_bass";
    public static final String EXTRA_EQ_MID = "eq_mid";
    public static final String EXTRA_EQ_TREBLE = "eq_treble";
    public static final String EXTRA_S8_VOLUME = "s8_volume";
    public static final String EXTRA_SEND_DATA = "extra_data";
    public static final String EXTRA_SERIAL_TYPE = "serial_type";
    public static final String EXTRA_TARGET_SOURCE = "target_source";
    public static final String EXTRA_VOLUME_FRONT_AND_REAR = "front_rear";
    public static final String EXTRA_VOLUME_LEFT_AND_RIGHT = "left_right";
    public static final String HANDLER_CAR_TYPE = "handler_car_type";
	
    public static final int SOURCE_OFF = 0x00;
    public static final int SOURCE_TUNER = 0x01; // radio
    public static final int SOURCE_DISC = 0x02; // cd or dvd
    public static final int SOURCE_TV = 0x03;
    public static final int SOURCE_NAVI = 0x04;
    public static final int SOURCE_PHONE = 0x05;
    public static final int SOURCE_IPOD = 0x06;
    public static final int SOURCE_AUX = 0x07;
    public static final int SOURCE_USB = 0x08;
    public static final int SOURCE_SD = 0x09;
    public static final int SOURCE_DVBT = 0x0A;
    public static final int SOURCE_PHONE_A2DP = 0x0B;
    public static final int SOURCE_OTHER = 0x0C;
    public static final int SOURCE_CDC = 0x0D;

    public static final int SOURCE_MEDIA_TYPE_TUNER = 0x01;
    public static final int SOURCE_MEDIA_TYPE_SIMPLE = 0x10;
    public static final int SOURCE_MEDIA_TYPE_ENHANCED = 0x11;
    public static final int SOURCE_MEDIA_TYPE_IPOD = 0x12;
    public static final int SOURCE_MEDIA_TYPE_FILE_BASED_VIDEO = 0x20;
    public static final int SOURCE_MEDIA_TYPE_DVD_VIDEO = 0x21;
    public static final int SOURCE_MEDIA_TYPE_OTHER_VIDEO = 0x22;
    public static final int SOURCE_MEDIA_TYPE_NAVI_AUX = 0x30;
    public static final int SOURCE_MEDIA_TYPE_PHONE = 0x40;
}
