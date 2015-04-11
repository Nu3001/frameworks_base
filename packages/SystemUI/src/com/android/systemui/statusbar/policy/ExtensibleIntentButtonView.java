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

import android.app.ActivityManager;
import android.app.ActivityManager.RunningTaskInfo;
import android.content.ActivityNotFoundException;
import android.content.Context;
import android.content.Intent;
import android.content.pm.ActivityInfo;
import android.content.pm.PackageManager;
import android.content.pm.ResolveInfo;
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
import java.util.List;
import com.android.systemui.R;

public class ExtensibleIntentButtonView extends KeyButtonView {
    private static final String TAG = "StatusBar.ExtensibleIntentButton";
    private static final boolean DEBUG = false;
    private static final int MAX_TASKS = 20; // get up to 20 tasks to search for apps

    int mTaskID = 0;
    Context mContext;
    List<ResolveInfo> mPackages;
    List<ActivityManager.RunningTaskInfo> mTasks;
    PackageManager mPackageManager;
    Intent mIntent;
    ActivityManager mActivityManager;

    Runnable mCheckLongPress = new Runnable() {
        public void run() {
            if (isPressed()) {
                setPressed(false);
                performLongClick();
            }
        }
    };

    public ExtensibleIntentButtonView(Context context, AttributeSet attrs) {
        this(context, attrs, 0);
    }

    public ExtensibleIntentButtonView(Context context, AttributeSet attrs, int defStyle) {
        super(context, attrs);
        mContext = context;
        mPackageManager = mContext.getPackageManager();
        mActivityManager = (ActivityManager) mContext.getSystemService(mContext.ACTIVITY_SERVICE);
    }


    public boolean onTouchEvent(MotionEvent ev) {
        final int action = ev.getAction();
        int x, y;

        switch (action) {
            case MotionEvent.ACTION_DOWN:
                mDownTime = SystemClock.uptimeMillis();
                setPressed(true);
                performPreload(); // preload certain intents and packages.
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

        if (mTaskID > 0) { // We found a task
            mActivityManager.moveTaskToFront(mTaskID,0);
        }else{ // No active tasks - launch default for intent
            try {
                mContext.startActivity(mIntent);
            } catch (ActivityNotFoundException e) {
            }
        }
        return true;
    }

    public boolean performLongClick() {
        mIntent = Intent.createChooser(mIntent,getResources().getString(R.string.chooser_text));
        mIntent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        mContext.startActivity(mIntent);
        return true;
    }

    public void performPreload() {
        Log.d(TAG,"starting PreLoad");
        mIntent = getActionIntent();
        mPackages = mPackageManager.queryIntentActivities(getActionIntent(),0);
        mTasks = mActivityManager.getRunningTasks(MAX_TASKS);
        mTaskID = getRunningTask();
    }

    public Intent getActionIntent() {
        Intent intent = new Intent();
        return intent;
    }

    public int getRunningTask() {
        Log.d(TAG,"Starting getRunningTask");
        int taskID = 0;
        String task = "";
        String packagename = "";
        boolean finished = false;
        Log.d(TAG,"Resolve Count:" + mPackages.size());
        Log.d(TAG,"Task Count:" + mTasks.size());
        for (ActivityManager.RunningTaskInfo ti : mTasks) { // loop through running tasks
            task = ti.baseActivity.getPackageName();
            Log.d(TAG,"TI:"+task);
            for (ResolveInfo ri : mPackages) { // Loop through avail packages
                packagename = ri.activityInfo.packageName;
                Log.d(TAG,"RI"+packagename);
                if (task.equals(packagename)) {
                    taskID = ti.id;
                    finished = true;
                    break;
                }
            }
            if (finished) { // if we found the task in innerloop, stop the outer loop too
                break;
            }
        }
        if (finished) { // did we find a match?
            return taskID;
        }else {
            return 0; //we did not find a match
        }
    }
}


