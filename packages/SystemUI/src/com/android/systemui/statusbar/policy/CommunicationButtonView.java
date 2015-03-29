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

import android.content.ActivityNotFoundException;
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

public class CommunicationButtonView extends KeyButtonView {
    private static final String TAG = "StatusBar.MusicButtonView";
    private static final boolean DEBUG = false;

    Context mContext;

    public CommunicationButtonView(Context context, AttributeSet attrs) {
        this(context, attrs, 0);
    }

    public CommunicationButtonView(Context context, AttributeSet attrs, int defStyle) {
        super(context, attrs);
        mContext = context;
    }

    public boolean performClick() {
        Intent intent = new Intent();
        intent.setAction(Intent.ACTION_DIAL);
        intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        try {
            mContext.startActivity(intent); }
        catch (ActivityNotFoundException e) {}
        return true;
    }

    public boolean performLongClick() {
        setPressed(false);
        Intent intent = new Intent();
        intent.setAction(Intent.ACTION_DIAL);
        intent.putExtra(Intent.EXTRA_TEXT,mContext.getResources().getString(R.string.communication_chooser_text));
        intent = Intent.createChooser(intent,getResources().getString(R.string.communication_chooser_text));
        intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        mContext.startActivity(intent);
        return true;
    }
}



