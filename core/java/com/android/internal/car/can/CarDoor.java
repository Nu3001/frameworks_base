
package com.android.internal.car.can;

import android.os.Bundle;

public class CarDoor {
    public static final String BUNDLE_NAME = "car_door_data";
    public static final String KEY_DOOR_FRONT_LEFT = "front_left";
    public static final String KEY_DOOR_FRONT_RIGHT = "front_right";
    public static final String KEY_DOOR_REAR_LEFT = "rear_left";
    public static final String KEY_DOOR_REAR_RIGHT = "rear_right";
    public static final String KEY_DOOR_REAR_CENTER = "rear_center";

    private boolean mFrontLeftOpened;
    private boolean mFrontRightOpened;
    private boolean mRearLeftOpened;
    private boolean mRearRightOpened;
    private boolean mRearCenterOpened;

    public void setFrontLeft(boolean opened) {
        mFrontLeftOpened = opened;
    }

    public boolean getFrontLeft() {
        return mFrontLeftOpened;
    }

    public void setFrontRight(boolean opened) {
        mFrontRightOpened = opened;
    }

    public boolean getFrontRight() {
        return mFrontRightOpened;
    }

    public void setRearLeft(boolean opened) {
        mRearLeftOpened = opened;
    }

    public boolean getRearLeft() {
        return mRearLeftOpened;
    }

    public void setRearRight(boolean opened) {
        mRearRightOpened = opened;
    }

    public boolean getRearRight() {
        return mRearRightOpened;
    }

    public void setRearCenter(boolean opened) {
        mRearCenterOpened = opened;
    }

    public boolean getRearCenter() {
        return mRearCenterOpened;
    }

    public boolean isAllClosed() {
        return !(mFrontLeftOpened || mFrontRightOpened
                || mRearLeftOpened || mRearRightOpened
                || mRearCenterOpened);
    }

    public static CarDoor bundleToCarDoor(final Bundle bundle) {
        final boolean frontLeftOpened = bundle.getBoolean(CarDoor.KEY_DOOR_FRONT_LEFT, false);
        final boolean frontRightOpened = bundle.getBoolean(CarDoor.KEY_DOOR_FRONT_RIGHT, false);
        final boolean rearLeftOpened = bundle.getBoolean(CarDoor.KEY_DOOR_REAR_LEFT, false);
        final boolean rearRightOpened = bundle.getBoolean(CarDoor.KEY_DOOR_REAR_RIGHT, false);
        final boolean rearCenterOpened = bundle.getBoolean(CarDoor.KEY_DOOR_REAR_CENTER, false);
        final CarDoor carDoor = new CarDoor();
        carDoor.setFrontLeft(frontLeftOpened);
        carDoor.setFrontRight(frontRightOpened);
        carDoor.setRearLeft(rearLeftOpened);
        carDoor.setRearRight(rearRightOpened);
        carDoor.setRearCenter(rearCenterOpened);
        return carDoor;
    }

    public static Bundle carDoorToBundle(final CarDoor carDoor) {
        final Bundle bundle = new Bundle();
        bundle.putBoolean(CarDoor.KEY_DOOR_FRONT_LEFT, carDoor.getFrontLeft());
        bundle.putBoolean(CarDoor.KEY_DOOR_FRONT_RIGHT, carDoor.getFrontRight());
        bundle.putBoolean(CarDoor.KEY_DOOR_REAR_LEFT, carDoor.getRearLeft());
        bundle.putBoolean(CarDoor.KEY_DOOR_REAR_RIGHT, carDoor.getRearRight());
        bundle.putBoolean(CarDoor.KEY_DOOR_REAR_CENTER, carDoor.getRearCenter());
        return bundle;
    }

    public String toString() {
        return ("Front left:" + mFrontLeftOpened + "  Front right:" + mFrontRightOpened
            + "  Rear left:" + mRearLeftOpened + "  Rear right:" + mRearRightOpened
            + "  Rear center:" + mRearCenterOpened);
    }
}
