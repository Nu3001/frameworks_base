
package com.android.internal.car.can;

public class CanBusManager {
    private static final String TAG = "CanBusManager";
    private static final boolean D = false;

    public static final String ACTION_SEND = "com.android.internal.car.can.action.SEND";
    public static final String ACTION_RECEINVED = "com.android.internal.car.can.action.RECEIVED";
    public static final String ACTION_SWITCH_SOURCE = "com.android.internal.car.can.action.SWITCH_SOURCE";

    public static final String CATEGORY_AIRCONDITIONING = "com.android.internal.car.can.AIRCONDITIONING";
    public static final String CATEGORY_RADAR = "com.android.internal.car.can.RADAR";
    public static final String CATEGORY_RADIO = "com.android.internal.car.can.Radio";
    public static final String EXTRA_SEND_DATA = "extra_data";

    public static final String EXTRA_TARGET_SOURCE = "target_source";
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
