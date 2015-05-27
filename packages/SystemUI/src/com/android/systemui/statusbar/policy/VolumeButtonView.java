/*
 * Copyright (C) 2008 The Android Open Source Project
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

package com.android.systemui.statusbar.policy;

import android.content.Context;
import android.content.Intent;
import android.hardware.input.InputManager;
import android.os.SystemClock;
import android.util.AttributeSet;
import android.util.Log;
import android.view.HapticFeedbackConstants;
import android.view.InputDevice;
import android.view.KeyCharacterMap;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.SoundEffectConstants;
import android.view.View;
import android.view.ViewConfiguration;
import android.view.ViewDebug;
import android.view.accessibility.AccessibilityEvent;
import android.widget.ImageView;
import android.os.Handler;
import android.os.Message;
import com.android.systemui.R;

public class VolumeButtonView extends KeyButtonView {
    private static final String TAG = "StatusBar.VolumeButtonView";
    private static final boolean DEBUG = false;

    Context mContext;


    Runnable mCheckLongPress = new Runnable() {
        public void run() {
            if (isPressed()) {
                // Log.d("MusicButtonView", "longpressed: " + this);

                setPressed(false);
                performLongClick();
            }
        }
    };

    public VolumeButtonView(Context context, AttributeSet attrs) {
        this(context, attrs, 0);
    }

    public VolumeButtonView(Context context, AttributeSet attrs, int defStyle) {
        super(context, attrs);
        mContext = context;
    }


    public boolean onTouchEvent(MotionEvent ev) {
        final int action = ev.getAction();
        int x, y;

        switch (action) {
            case MotionEvent.ACTION_DOWN:
                //Log.d("KeyButtonView", "press");
                mDownTime = SystemClock.uptimeMillis();
                setPressed(true);
                if (mSupportsLongpress) {
                    removeCallbacks(mCheckLongPress);
                    postDelayed(mCheckLongPress, ViewConfiguration.getLongPressTimeout());
                }
                break;
            case MotionEvent.ACTION_MOVE:
                x = (int)ev.getX();
                y = (int)ev.getY();
                setPressed(x >= -mTouchSlop
                        && x < getWidth() + mTouchSlop
                        && y >= -mTouchSlop
                        && y < getHeight() + mTouchSlop);
                break;
            case MotionEvent.ACTION_CANCEL:
                setPressed(false);
                if (mSupportsLongpress) {
                    removeCallbacks(mCheckLongPress);
                }
                break;
            case MotionEvent.ACTION_UP:
                final boolean doIt = isPressed();
                setPressed(false);
                if (mSupportsLongpress) {
                    removeCallbacks(mCheckLongPress);
                }
                if (doIt) {
                        performClick();
                }
                break;
        }
        return true;
    }

    public boolean performClick() {
        if (mCode == 24) {
            Intent intent = new Intent("android.intent.action.BONOVO_VOLUMEADD_KEY");
            intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
            mContext.sendBroadcast(intent);
        } else if (mCode == 25) {
            Intent intent = new Intent("android.intent.action.BONOVO_VOLUMESUB_KEY");
            intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
            mContext.sendBroadcast(intent);
        } else {
            Intent intent = new Intent("android.intent.action.BONOVO_UPDATEVOLUME_KEY");
            intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
            mContext.sendBroadcast(intent);
        }
        return true;
    }
    public boolean performLongClick() {
        Intent intent = new Intent("android.intent.action.KEYCODE_BONOVO_SYSTEMMUTE_KEY");
        intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        mContext.sendBroadcast(intent);
        return true;
    }
}



