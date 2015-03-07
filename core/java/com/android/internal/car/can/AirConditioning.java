
package com.android.internal.car.can;

import android.content.SharedPreferences;
import android.os.Bundle;

public class AirConditioning {
    public static final String BUNDLE_NAME = "air_condition_bundle";
    public static final String ACTION_NAME = "com.newsmy.car.airconditioner.event";
    public static final int EXTERNAL_CYCLE = 0;
    public static final int INTERNAL_CYCLE = 1;
    public static final boolean SWITCH_ON = true;
    public static final boolean SWITCH_OFF = false;
    public static final int SEAT_HEATING_NONE = 0;
    public static final int SEAT_HEATING_LEVEL_1 = 1;
    public static final int SEAT_HEATING_LEVEL_2 = 2;
    public static final int SEAT_HEATING_LEVEL_3 = 3;
    public static final float DEFAULT_TEMP = 0.0f;
    public static final int MIN_WIND_LEVEL = 0;
    public static final int MAX_WIND_LEVEL = 7;
    public static final int MAX_SEAT_HEATING = 3;

    public static final String EXTRA_AIR_CONDITIONING = "extra_air_condition";
    public static final String EXTRA_AC = "ac";
    public static final String EXTRA_CYCLE = "cycle";
    public static final String EXTRA_AUTO_STRONG_WIND = "strong_wind";
    public static final String EXTRA_AUTO_SOFT_WIND = "soft_wind";
    public static final String EXTRA_AUTO_WIND = "auto_wind";
    public static final String EXTRA_DUAL = "dual";
    public static final String EXTRA_MAXFORNT = "maxfornt";
    public static final String EXTRA_REAR = "rear";
//    public static final String EXTRA_UP_WIND = "up_wind";
//    public static final String EXTRA_HORIZONTAL_WIND = "horizontal_wind";
    public static final String EXTRA_WIND_DIRECTION = "wind_direction";
//    public static final String EXTRA_DOWN_WIND = "down_wind";
    public static final String EXTRA_DISPLAY = "display";
    public static final String EXTRA_WIND_LEVEL = "wind_level";
    public static final String EXTRA_LEFT_TEMP = "left_temp";
    public static final String EXTRA_RIGHT_TEMP = "right_temp";
    public static final String EXTRA_AQS_CYCLE = "aqs_cycle";
    public static final String EXTRA_LEFT_SEAT_HEATING = "left_seat_heating";
    public static final String EXTRA_REAR_LOCK = "rear_lock";
    public static final String EXTRA_AC_MAX = "ac_max";
    public static final String EXTRA_RIGHT_SEAT_HEATING = "right_seat_heating";

    
    // air conditioning condition
    private boolean AcDisplaySwitch;
    private boolean AirConditioningSwitch;
    private boolean ACSwitch;
    private int Cycle;
    private boolean AUTOStrongWindSwitch;
    private boolean AUTOSoftWindSiwtch;
    private boolean AUTOSwitch;
    private boolean DUALSwitch;
    private boolean MAXFORNTSwitch;
    private boolean REARSwitch;

    // wind
    private boolean UpWindSwitch;
    private boolean HorizontalWindSwitch;
    private boolean DownWindSwitch;
    private int WindDirection;
    private int WindLevel;

    // left temp
    private float LeftTemp;

    // right temp
    private float RightTemp;

    // status info
    private boolean AQSInternalCycleSwitch;
    private int LeftSeatHeatingLevel;
    private boolean REARLockSwitch;

    public void setWindDirection(int windDirection) {
        WindDirection = windDirection;
    }
    
    public int getWindDirection() {
        return WindDirection;
    }
    public boolean getAirConditioningSwitch() {
        return AirConditioningSwitch;
    }

    public void setAirConditioningSwitch(boolean airConditioningSwitch) {
        AirConditioningSwitch = airConditioningSwitch;
    }

    public boolean getACSwitch() {
        return ACSwitch;
    }

    public void setACSwitch(boolean ACSwitch) {
        this.ACSwitch = ACSwitch;
    }

    public int getCycle() {
        return Cycle;
    }

    public void setCycle(int cycle) {
        Cycle = cycle;
    }

    public boolean getAUTOStrongWindSwitch() {
        return AUTOStrongWindSwitch;
    }

    public void setAUTOStrongWindSwitch(boolean AUTOStrongWindSwitch) {
        this.AUTOStrongWindSwitch = AUTOStrongWindSwitch;
    }

    public boolean getAUTOSoftWindSiwtch() {
        return AUTOSoftWindSiwtch;
    }

    public void setAUTOSoftWindSiwtch(boolean AUTOSoftWindSiwtch) {
        this.AUTOSoftWindSiwtch = AUTOSoftWindSiwtch;
    }

    public boolean getAUTOSwitch() {
        return AUTOSwitch;
    }

    public void setAUTOSwitch(boolean AUTOSwitch) {
        this.AUTOSwitch = AUTOSwitch;
    }

    public boolean getDUALSwitch() {
        return DUALSwitch;
    }

    public void setDUALSwitch(boolean DUALSwitch) {
        this.DUALSwitch = DUALSwitch;
    }

    public boolean getMAXFORNTSwitch() {
        return MAXFORNTSwitch;
    }

    public void setMAXFORNTSwitch(boolean MAXFORNTSwitch) {
        this.MAXFORNTSwitch = MAXFORNTSwitch;
    }

    public boolean getREARSwitch() {
        return REARSwitch;
    }

    public void setREARSwitch(boolean REARSwitch) {
        this.REARSwitch = REARSwitch;
    }

//    public boolean getUpWindSwitch() {
//        return UpWindSwitch;
//    }
//
//    public void setUpWindSwitch(boolean upWindSwitch) {
//        UpWindSwitch = upWindSwitch;
//    }
//
//    public boolean getHorizontalWindSwitch() {
//        return HorizontalWindSwitch;
//    }
//
//    public void setHorizontalWindSwitch(boolean horizontalWindSwitch) {
//        HorizontalWindSwitch = horizontalWindSwitch;
//    }

//    public boolean getDownWindSwitch() {
//        return DownWindSwitch;
//    }
//
//    public void setDownWindSwitch(boolean downWindSwitch) {
//        DownWindSwitch = downWindSwitch;
//    }

    public boolean getAirConditioningDisplaySiwtch() {
        return AcDisplaySwitch;
    }

    public void setAirConditioningDisplaySiwtch(boolean airConditioningDisplaySiwtch) {
        AcDisplaySwitch = airConditioningDisplaySiwtch;
    }

    public int getWindLevel() {
        return WindLevel;
    }

    public void setWindLevel(int windLevel) {
        WindLevel = windLevel;
    }

    public float getLeftTemp() {
        return LeftTemp;
    }

    public void setLeftTemp(float leftTemp) {
        LeftTemp = leftTemp;
    }

    public float getRightTemp() {
        return RightTemp;
    }

    public void setRightTemp(float rightTemp) {
        RightTemp = rightTemp;
    }

    public boolean getAQSInternalCycleSwitch() {
        return AQSInternalCycleSwitch;
    }

    public void setAQSInternalCycleSwitch(boolean AQSInternalCycleSwitch) {
        this.AQSInternalCycleSwitch = AQSInternalCycleSwitch;
    }

    public int getLeftSeatHeatingLevel() {
        return LeftSeatHeatingLevel > MAX_SEAT_HEATING ? MAX_SEAT_HEATING : LeftSeatHeatingLevel;
    }

    public void setLeftSeatHeatingLevel(int leftSeatHeatingLevel) {
        LeftSeatHeatingLevel = leftSeatHeatingLevel;
    }

    public boolean getREARLockSwitch() {
        return REARLockSwitch;
    }

    public void setREARLockSwitch(boolean REARLockSwitch) {
        this.REARLockSwitch = REARLockSwitch;
    }

    public boolean getACMAXSwitch() {
        return ACMAXSwitch;
    }

    public void setACMAXSwitch(boolean ACMAXSwitch) {
        this.ACMAXSwitch = ACMAXSwitch;
    }

    public int getRightSeatHeatingLevel() {
        return RightSeatHeatingLevel > MAX_SEAT_HEATING ? MAX_SEAT_HEATING : RightSeatHeatingLevel;
    }

    public void setRightSeatHeatingLevel(int rightSeatHeatingLevel) {
        RightSeatHeatingLevel = rightSeatHeatingLevel;
    }

    private boolean ACMAXSwitch;
    private int RightSeatHeatingLevel;

    public static Bundle airCondition2Bundle(final AirConditioning airConditioning) {
        final Bundle airConditionBundle = new Bundle();
        airConditionBundle.putBoolean(EXTRA_AIR_CONDITIONING,
                airConditioning.getAirConditioningSwitch());
        airConditionBundle.putBoolean(EXTRA_AC, airConditioning.getACSwitch());
        airConditionBundle.putInt(EXTRA_CYCLE, airConditioning.getCycle());
        airConditionBundle.putBoolean(EXTRA_AUTO_STRONG_WIND,
                airConditioning.getAUTOStrongWindSwitch());
        airConditionBundle
                .putBoolean(EXTRA_AUTO_SOFT_WIND, airConditioning.getAUTOSoftWindSiwtch());
        airConditionBundle.putBoolean(EXTRA_AUTO_WIND, airConditioning.getAUTOSwitch());
        airConditionBundle.putBoolean(EXTRA_DUAL, airConditioning.getDUALSwitch());
        airConditionBundle.putBoolean(EXTRA_MAXFORNT, airConditioning.getMAXFORNTSwitch());
        airConditionBundle.putBoolean(EXTRA_REAR, airConditioning.getREARSwitch());
//        airConditionBundle.putBoolean(EXTRA_UP_WIND, airConditioning.getUpWindSwitch());
//        airConditionBundle.putBoolean(EXTRA_HORIZONTAL_WIND,
//                airConditioning.getHorizontalWindSwitch());
//        airConditionBundle.putBoolean(EXTRA_DOWN_WIND, airConditioning.getDownWindSwitch());
        airConditionBundle.putInt(EXTRA_WIND_DIRECTION, airConditioning.getWindDirection());
        airConditionBundle.putBoolean(EXTRA_DISPLAY,
                airConditioning.getAirConditioningDisplaySiwtch());
        airConditionBundle.putInt(EXTRA_WIND_LEVEL, airConditioning.getWindLevel());
        airConditionBundle.putFloat(EXTRA_LEFT_TEMP, airConditioning.getLeftTemp());
        airConditionBundle.putFloat(EXTRA_RIGHT_TEMP, airConditioning.getRightTemp());
        airConditionBundle.putBoolean(EXTRA_AQS_CYCLE, airConditioning.getAQSInternalCycleSwitch());
        airConditionBundle.putInt(EXTRA_LEFT_SEAT_HEATING,
                airConditioning.getLeftSeatHeatingLevel());
        airConditionBundle.putBoolean(EXTRA_REAR_LOCK, airConditioning.getREARLockSwitch());
        airConditionBundle.putBoolean(EXTRA_AC_MAX, airConditioning.getACMAXSwitch());
        airConditionBundle.putInt(EXTRA_RIGHT_SEAT_HEATING,
                airConditioning.getRightSeatHeatingLevel());
        return airConditionBundle;
    }

    public static AirConditioning bundle2AirCondition(final Bundle bundle) {
        if (bundle == null)
            return null;
        final AirConditioning airConditioning = new AirConditioning();
        airConditioning.setAirConditioningSwitch(bundle.getBoolean(EXTRA_AIR_CONDITIONING,
                AirConditioning.SWITCH_OFF));
        airConditioning.setACSwitch(bundle.getBoolean(EXTRA_AC, AirConditioning.SWITCH_OFF));
        airConditioning.setCycle(bundle.getInt(EXTRA_CYCLE, AirConditioning.EXTERNAL_CYCLE));
        airConditioning.setAUTOStrongWindSwitch(bundle.getBoolean(EXTRA_AUTO_STRONG_WIND,
                AirConditioning.SWITCH_OFF));
        airConditioning.setAUTOSoftWindSiwtch(bundle.getBoolean(EXTRA_AUTO_SOFT_WIND,
                AirConditioning.SWITCH_OFF));
        airConditioning.setAUTOSwitch(bundle.getBoolean(EXTRA_AUTO_WIND, AirConditioning.SWITCH_OFF));
        airConditioning.setDUALSwitch(bundle.getBoolean(EXTRA_DUAL, AirConditioning.SWITCH_OFF));
        airConditioning.setMAXFORNTSwitch(bundle.getBoolean(EXTRA_MAXFORNT,
                AirConditioning.SWITCH_OFF));
        airConditioning.setREARSwitch(bundle.getBoolean(EXTRA_REAR, AirConditioning.SWITCH_OFF));
//        airConditioning.setUpWindSwitch(bundle
//                .getBoolean(EXTRA_UP_WIND, AirConditioning.SWITCH_OFF));
//        airConditioning.setHorizontalWindSwitch(bundle.getBoolean(EXTRA_HORIZONTAL_WIND,
//                AirConditioning.SWITCH_OFF));
//        airConditioning.setDownWindSwitch(bundle.getBoolean(EXTRA_DOWN_WIND,
//                AirConditioning.SWITCH_OFF));
        airConditioning.setWindDirection(bundle.getInt(EXTRA_WIND_DIRECTION, -1));
        airConditioning.setAirConditioningDisplaySiwtch(bundle.getBoolean(EXTRA_DISPLAY,
                AirConditioning.SWITCH_OFF));
        airConditioning.setWindLevel(bundle
                .getInt(EXTRA_WIND_LEVEL, AirConditioning.MIN_WIND_LEVEL));
        airConditioning.setLeftTemp(bundle.getFloat(EXTRA_LEFT_TEMP, AirConditioning.DEFAULT_TEMP));
        airConditioning.setRightTemp(bundle
                .getFloat(EXTRA_RIGHT_TEMP, AirConditioning.DEFAULT_TEMP));
        airConditioning.setAQSInternalCycleSwitch(bundle.getBoolean(EXTRA_AQS_CYCLE,
                AirConditioning.SWITCH_OFF));
        airConditioning.setLeftSeatHeatingLevel(bundle.getInt(EXTRA_LEFT_SEAT_HEATING,
                AirConditioning.SEAT_HEATING_NONE));
        airConditioning.setREARLockSwitch(bundle.getBoolean(EXTRA_REAR_LOCK,
                AirConditioning.SWITCH_OFF));
        airConditioning.setACMAXSwitch(bundle.getBoolean(EXTRA_AC_MAX, AirConditioning.SWITCH_OFF));
        airConditioning.setRightSeatHeatingLevel(bundle.getInt(EXTRA_RIGHT_SEAT_HEATING,
                AirConditioning.SEAT_HEATING_NONE));
        return airConditioning;
    }

    public static void saveAirCondition(SharedPreferences sp, final AirConditioning airConditioning) {
        final SharedPreferences.Editor editor = sp.edit();
        editor.putBoolean(EXTRA_AIR_CONDITIONING, airConditioning.getAirConditioningSwitch());
        editor.putBoolean(EXTRA_AC, airConditioning.getACSwitch());
        editor.putInt(EXTRA_CYCLE, airConditioning.getCycle());
        editor.putBoolean(EXTRA_AUTO_STRONG_WIND, airConditioning.getAUTOStrongWindSwitch());
        editor.putBoolean(EXTRA_AUTO_SOFT_WIND, airConditioning.getAUTOSoftWindSiwtch());
        editor.putBoolean(EXTRA_AUTO_WIND, airConditioning.getAUTOSwitch());
        editor.putBoolean(EXTRA_DUAL, airConditioning.getDUALSwitch());
        editor.putBoolean(EXTRA_MAXFORNT, airConditioning.getMAXFORNTSwitch());
        editor.putBoolean(EXTRA_REAR, airConditioning.getREARSwitch());
//        editor.putBoolean(EXTRA_UP_WIND, airConditioning.getUpWindSwitch());
//        editor.putBoolean(EXTRA_HORIZONTAL_WIND, airConditioning.getHorizontalWindSwitch());
//        editor.putBoolean(EXTRA_DOWN_WIND, airConditioning.getDownWindSwitch());
        editor.putInt(EXTRA_WIND_DIRECTION, airConditioning.getWindDirection());
        editor.putBoolean(EXTRA_DISPLAY, airConditioning.getAirConditioningDisplaySiwtch());
        editor.putInt(EXTRA_WIND_LEVEL, airConditioning.getWindLevel());
        editor.putFloat(EXTRA_LEFT_TEMP, airConditioning.getLeftTemp());
        editor.putFloat(EXTRA_RIGHT_TEMP, airConditioning.getRightTemp());
        editor.putBoolean(EXTRA_AQS_CYCLE, airConditioning.getAQSInternalCycleSwitch());
        editor.putInt(EXTRA_LEFT_SEAT_HEATING, airConditioning.getLeftSeatHeatingLevel());
        editor.putBoolean(EXTRA_REAR_LOCK, airConditioning.getREARLockSwitch());
        editor.putBoolean(EXTRA_AC_MAX, airConditioning.getACMAXSwitch());
        editor.putInt(EXTRA_RIGHT_SEAT_HEATING, airConditioning.getRightSeatHeatingLevel());
        editor.commit();
    }

    public static AirConditioning restoreAirCondition(final SharedPreferences sp) {
        AirConditioning airConditioning = new AirConditioning();
        airConditioning.setAirConditioningSwitch(sp.getBoolean(EXTRA_AIR_CONDITIONING,
                AirConditioning.SWITCH_OFF));
        airConditioning.setACSwitch(sp.getBoolean(EXTRA_AC, AirConditioning.SWITCH_OFF));
        airConditioning.setCycle(sp.getInt(EXTRA_CYCLE, AirConditioning.EXTERNAL_CYCLE));
        airConditioning.setAUTOStrongWindSwitch(sp.getBoolean(EXTRA_AUTO_STRONG_WIND,
                AirConditioning.SWITCH_OFF));
        airConditioning.setAUTOSoftWindSiwtch(sp.getBoolean(EXTRA_AUTO_SOFT_WIND,
                AirConditioning.SWITCH_OFF));
        airConditioning.setAUTOSwitch(sp.getBoolean(EXTRA_AUTO_WIND, AirConditioning.SWITCH_OFF));
        airConditioning.setDUALSwitch(sp.getBoolean(EXTRA_DUAL, AirConditioning.SWITCH_OFF));
        airConditioning
                .setMAXFORNTSwitch(sp.getBoolean(EXTRA_MAXFORNT, AirConditioning.SWITCH_OFF));
        airConditioning.setREARSwitch(sp.getBoolean(EXTRA_REAR, AirConditioning.SWITCH_OFF));
//        airConditioning.setUpWindSwitch(sp.getBoolean(EXTRA_UP_WIND, AirConditioning.SWITCH_OFF));
//        airConditioning.setHorizontalWindSwitch(sp.getBoolean(EXTRA_HORIZONTAL_WIND,
//                AirConditioning.SWITCH_OFF));
//        airConditioning.setDownWindSwitch(sp
//                .getBoolean(EXTRA_DOWN_WIND, AirConditioning.SWITCH_OFF));
        airConditioning.setWindDirection(sp.getInt(EXTRA_WIND_DIRECTION, 0));
        airConditioning.setAirConditioningDisplaySiwtch(sp.getBoolean(EXTRA_DISPLAY,
                AirConditioning.SWITCH_OFF));
        airConditioning.setWindLevel(sp.getInt(EXTRA_WIND_LEVEL, AirConditioning.MIN_WIND_LEVEL));
        airConditioning.setLeftTemp(sp.getFloat(EXTRA_LEFT_TEMP, AirConditioning.DEFAULT_TEMP));
        airConditioning.setRightTemp(sp.getFloat(EXTRA_RIGHT_TEMP, AirConditioning.DEFAULT_TEMP));
        airConditioning.setAQSInternalCycleSwitch(sp.getBoolean(EXTRA_AQS_CYCLE,
                AirConditioning.SWITCH_OFF));
        airConditioning.setLeftSeatHeatingLevel(sp.getInt(EXTRA_LEFT_SEAT_HEATING,
                AirConditioning.SEAT_HEATING_NONE));
        airConditioning.setREARLockSwitch(sp
                .getBoolean(EXTRA_REAR_LOCK, AirConditioning.SWITCH_OFF));
        airConditioning.setACMAXSwitch(sp.getBoolean(EXTRA_AC_MAX, AirConditioning.SWITCH_OFF));
        airConditioning.setRightSeatHeatingLevel(sp.getInt(EXTRA_RIGHT_SEAT_HEATING,
                AirConditioning.SEAT_HEATING_NONE));

        return airConditioning;
    }
    
    @Override
    public String toString() {
        StringBuilder sb = new StringBuilder();
        sb.append(EXTRA_AC);
        sb.append(":");
        sb.append(ACSwitch);
        sb.append("\n");
        sb.append(EXTRA_AC_MAX);
        sb.append(":");
        sb.append(ACMAXSwitch);
        sb.append("\n");
        sb.append(EXTRA_AIR_CONDITIONING);
        sb.append(":");
        sb.append(AirConditioningSwitch);
        sb.append("\n");
        sb.append(EXTRA_AQS_CYCLE);
        sb.append(":");
        sb.append(AQSInternalCycleSwitch);
        sb.append("\n");
        sb.append(EXTRA_AUTO_SOFT_WIND);
        sb.append(":");
        sb.append(AUTOSoftWindSiwtch);
        sb.append("\n");
        sb.append(EXTRA_AUTO_WIND);
        sb.append(":");
        sb.append(AUTOSwitch);
        sb.append("\n");
        sb.append(EXTRA_AUTO_STRONG_WIND);
        sb.append(":");
        sb.append(AUTOStrongWindSwitch);
        sb.append("\n");
        sb.append(EXTRA_CYCLE);
        sb.append(":");
        sb.append(Cycle);
        sb.append("\n");
        sb.append(EXTRA_DISPLAY);
        sb.append(":");
        sb.append(AcDisplaySwitch);
        sb.append("\n");
        sb.append(EXTRA_DUAL);
        sb.append(":");
        sb.append(DUALSwitch);
        sb.append("\n");
        sb.append(EXTRA_LEFT_SEAT_HEATING);
        sb.append(":");
        sb.append(LeftSeatHeatingLevel);
        sb.append("\n");
        sb.append(EXTRA_LEFT_TEMP);
        sb.append(":");
        sb.append(LeftTemp);
        sb.append("\n");
        sb.append(EXTRA_MAXFORNT);
        sb.append(":");
        sb.append(MAXFORNTSwitch);
        sb.append("\n");
        sb.append(EXTRA_REAR);
        sb.append(":");
        sb.append(REARSwitch);
        sb.append("\n");
        sb.append(EXTRA_REAR_LOCK);
        sb.append(":");
        sb.append(REARLockSwitch);
        sb.append("\n");
        sb.append(EXTRA_RIGHT_SEAT_HEATING);
        sb.append(":");
        sb.append(RightSeatHeatingLevel);
        sb.append("\n");
        sb.append(EXTRA_RIGHT_TEMP);
        sb.append(":");
        sb.append(RightTemp);
        sb.append("\n");
        sb.append(EXTRA_WIND_DIRECTION);
        sb.append(":");
        sb.append(WindDirection);
        sb.append("\n");        
        sb.append(EXTRA_WIND_LEVEL);
        sb.append(":");
        sb.append(WindLevel);
        sb.append("\n");
        
        return sb.toString();
    }
}
