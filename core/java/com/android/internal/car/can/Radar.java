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

public class Radar {
    private static final String TAG = "Radar";
    private static final boolean LOCAL_LOGD = false;
    public static final String BUNDLE_NAME = "radar_bundle";
    public static final String ACTION_SHOW = "com.android.internal.car.can";
    public static final String EXTRA_SHOW = "show";
    /*
     * @
     */
    public static final String ACTION_CANBUS_RADAR = "com.bonovo.canbus.Radar";

    /*
     * @ The key that get the distance of headstock left and obstacle through
     * intent
     */
    public static final String KEY_HEAD_LEFT = "head_left";

    /*
     * @ The key that get the distance of headstock right and obstacle through
     * intent
     */
    public static final String KEY_HEAD_RIGHT = "head_right";

    /*
     * @ The key that get the distance of tailstock left and obstacle through
     * intent
     */
    public static final String KEY_TAIL_LEFT = "tail_left";

    /*
     * @ The key that get the distance of tailstock right and obstacle through
     * intent
     */
    public static final String KEY_TAIL_RIGHT = "tail_right";

    /*
     * @ The key that get the distance of headstock centre-left and obstacle
     * through intent
     */
    public static final String KEY_HEAD_CENTRE_LEFT = "head_centre_left";

    /*
     * @ The key that get the distance of headstock centre-right and obstacle
     * through intent
     */
    public static final String KEY_HEAD_CENTRE_RIGHT = "head_centre_right";

    /*
     * @ The key that get the distance of tailstock centre-left and obstacle
     * through intent
     */
    public static final String KEY_TAIL_CENTRE_LEFT = "tail_centre_left";

    /*
     * @ The key that get the distance of tailstock centre-right and obstacle
     * through intent
     */
    public static final String KEY_TAIL_CENTRE_RIGHT = "tail_centre_right";

    private int mDistanceHeadstockLeft = 0; // The distance of headstock left
                                            // and obstacle
    private int mDistanceHeadstockRight = 0; // The distance of headstock right
                                             // and obstacle
    private int mDistanceTailstockLeft = 0; // The distance of tailstock left
                                            // and obstacle
    private int mDistanceTailstockRight = 0; // The distance of tailstock right
                                             // and obstacle
    private int mDistanceHeadstockCentreLeft = 0; // The distance of headstock
                                                  // centre-left and obstacle
    private int mDistanceHeadstockCentreRight = 0; // The distance of headstock
                                                   // centre-right and obstacle
    private int mDistanceTailstockCentreLeft = 0; // The distance of tailstock
                                                  // centre-left and obstacle
    private int mDistanceTailstockCentreRight = 0; // The distance of tailstock
                                                   // centre-right and obstacle

    public Radar() {
    }

    public Radar(int headLeft, int headRight, int tailLeft, int tailRight,
            int headCentreLeft, int headCentreRight, int tailCentreLeft, int tailCentreRight) {
        mDistanceHeadstockLeft = headLeft;
        mDistanceHeadstockRight = headRight;
        mDistanceTailstockLeft = tailLeft;
        mDistanceTailstockRight = tailRight;
        mDistanceHeadstockCentreLeft = headCentreLeft;
        mDistanceHeadstockCentreRight = headCentreRight;
        mDistanceTailstockCentreLeft = tailCentreLeft;
        mDistanceTailstockCentreRight = tailCentreRight;
    }

    // get distance functions
    public int getDistanceHeadstockLeft() {
        return mDistanceHeadstockLeft;
    }

    public int getDistanceHeadstockRight() {
        return mDistanceHeadstockRight;
    }

    public int getDistanceTailstockLeft() {
        return mDistanceTailstockLeft;
    }

    public int getDistanceTailstockRight() {
        return mDistanceTailstockRight;
    }

    public int getDistanceHeadstockCentreLeft() {
        return mDistanceHeadstockCentreLeft;
    }

    public int getDistanceHeadstockCentreRight() {
        return mDistanceHeadstockCentreRight;
    }

    public int getDistanceTailstockCentreLeft() {
        return mDistanceTailstockCentreLeft;
    }

    public int getDistanceTailstockCentreRight() {
        return mDistanceTailstockCentreRight;
    }

    // set distance functions
    public void setDistanceHeadstockLeft(int distance) {
        mDistanceHeadstockLeft = distance;
    }

    public void setDistanceHeadstockRight(int distance) {
        mDistanceHeadstockRight = distance;
    }

    public void setDistanceTailstockLeft(int distance) {
        mDistanceTailstockLeft = distance;
    }

    public void setDistanceTailstockRight(int distance) {
        mDistanceTailstockRight = distance;
    }

    public void setDistanceHeadstockCentreLeft(int distance) {
        mDistanceHeadstockCentreLeft = distance;
    }

    public void setDistanceHeadstockCentreRight(int distance) {
        mDistanceHeadstockCentreRight = distance;
    }

    public void setDistanceTailstockCentreLeft(int distance) {
        mDistanceTailstockCentreLeft = distance;
    }

    public void setDistanceTailstockCentreRight(int distance) {
        mDistanceTailstockCentreRight = distance;
    }

    public String toString() {
        return ("HeadLeft:" + mDistanceHeadstockLeft + " HeadRight:" + mDistanceHeadstockRight
                + " TailLeft:" + mDistanceTailstockLeft + " TailRight:" + mDistanceTailstockRight
                + " HeadCetreLeft:" + mDistanceHeadstockCentreLeft + " HeadCetreRight:"
                + mDistanceHeadstockCentreRight
                + " TailCetreLeft:" + mDistanceTailstockCentreLeft + " TailCetreRight:" + mDistanceTailstockCentreRight);
    }

    public Bundle toBundle() {
        final Bundle bundle = new Bundle();
        bundle.putInt(KEY_HEAD_LEFT, mDistanceHeadstockLeft);
        bundle.putInt(KEY_HEAD_RIGHT, mDistanceHeadstockRight);
        bundle.putInt(KEY_TAIL_LEFT, mDistanceTailstockLeft);
        bundle.putInt(KEY_TAIL_RIGHT, mDistanceTailstockRight);
        bundle.putInt(KEY_HEAD_CENTRE_LEFT, mDistanceHeadstockCentreLeft);
        bundle.putInt(KEY_HEAD_CENTRE_RIGHT, mDistanceHeadstockCentreRight);
        bundle.putInt(KEY_TAIL_CENTRE_LEFT, mDistanceTailstockCentreLeft);
        bundle.putInt(KEY_TAIL_CENTRE_RIGHT, mDistanceTailstockCentreRight);
        return bundle;
    }

    public static Radar bundle2Radar(final Bundle bundle) {
        return new Radar(
                bundle.getInt(KEY_HEAD_LEFT),
                bundle.getInt(KEY_HEAD_RIGHT),
                bundle.getInt(KEY_TAIL_LEFT),
                bundle.getInt(KEY_TAIL_RIGHT),
                bundle.getInt(KEY_HEAD_CENTRE_LEFT),
                bundle.getInt(KEY_HEAD_CENTRE_RIGHT),
                bundle.getInt(KEY_TAIL_CENTRE_LEFT),
                bundle.getInt(KEY_TAIL_CENTRE_RIGHT));
    }
}
