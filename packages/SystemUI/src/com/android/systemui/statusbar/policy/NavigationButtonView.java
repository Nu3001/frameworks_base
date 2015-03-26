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
import android.net.Uri;
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

public class NavigationButtonView extends KeyButtonView {
    private static final String TAG = "StatusBar.NavigationButtonView";
    private static final boolean DEBUG = false;

    private static final String NAVIGATION_INTENT = Intent.CATEGORY_APP_MAPS;
    Context mContext;


    Runnable mCheckLongPress = new Runnable() {
        public void run() {
            if (isPressed()) {
                // Log.d("MusicButtonView", "longpressed: " + this);

                // Just an old-fashioned ImageView
                performLongClick();
            }
        }
    };

    public NavigationButtonView(Context context, AttributeSet attrs) {
        this(context, attrs, 0);
    }

    public NavigationButtonView(Context context, AttributeSet attrs, int defStyle) {
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
                if (mCode != 0) {
                    sendEvent(KeyEvent.ACTION_DOWN, 0, mDownTime);
                } else {
                    // Provide the same haptic feedback that the system offers for virtual keys.
                    performHapticFeedback(HapticFeedbackConstants.VIRTUAL_KEY);
                }
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
                if (mCode != 0) {
                    sendEvent(KeyEvent.ACTION_UP, KeyEvent.FLAG_CANCELED);
                }
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
        Intent intent = new Intent(android.content.Intent.ACTION_VIEW,
                Uri.parse("geo:"));
        intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        mContext.startActivity(intent);
        return true;
    }
    public boolean performLongClick() {
        Intent intent = new Intent(android.content.Intent.ACTION_VIEW,
                Uri.parse("geo:"));
        intent.putExtra(Intent.EXTRA_TEXT,mContext.getResources().getString(R.string.navigation_chooser_text));
        intent = Intent.createChooser(intent,getResources().getString(R.string.navigation_chooser_text));
        intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        mContext.startActivity(intent);
        return true;
    }
}



