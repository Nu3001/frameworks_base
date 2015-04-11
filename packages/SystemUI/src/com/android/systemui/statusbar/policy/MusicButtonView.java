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
import android.util.AttributeSet;
import android.util.Log;
import com.android.systemui.R;

public class MusicButtonView extends ExtensibleIntentButtonView {
    private static final String TAG = "StatusBar.MusicButtonView";
    private static final boolean DEBUG = false;

    Context mContext;

    public MusicButtonView(Context context, AttributeSet attrs) {
        this(context, attrs, 0);
    }

    public MusicButtonView(Context context, AttributeSet attrs, int defStyle) {
        super(context, attrs);
    }

    public Intent getActionIntent() {
        Intent intent = new Intent();
        intent.setAction(Intent.ACTION_MAIN);
        intent.addCategory(Intent.CATEGORY_APP_MUSIC);
        intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        return intent;
    }
}



