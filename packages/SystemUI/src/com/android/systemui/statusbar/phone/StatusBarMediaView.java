/*  Orignal code by Dr Breen
*   https://github.com/DrBreen/RemoteControllerExample
*   Modified by Zaphod for embedding into StatusBar
   Licensed under the Apache License, Version 2.0 (the "License");
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
package com.android.systemui.statusbar.phone;

import android.media.MediaMetadataRetriever;
import android.media.RemoteControlClient;
import android.media.RemoteController;
import android.media.RemoteController.MetadataEditor;
import android.os.Bundle;
import android.os.Handler;
import android.os.IBinder;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.util.AttributeSet;
import android.util.Log;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.ImageButton;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.SeekBar;
import android.widget.SeekBar.OnSeekBarChangeListener;
import android.widget.TextView;
import com.android.systemui.StatusbarMediaService;
import com.android.systemui.R;

public class StatusBarMediaView extends LinearLayout {

    public final static String START_REMOTE_CONTROLLER_ACTION = "com.android.systemui.START_REMOTE_CONTROLLER";
    private final static String TAG = "StatusBarMedia";

    //Views in the Activity
    protected TextView mArtistText;
    protected TextView mTitleText;
    protected TextView mAlbumText;
    protected ImageView mArtwork;


    protected StatusbarMediaService mRCService;
    protected boolean mBound = false; //flag indicating if service is bound to Activity

    protected Handler mHandler = new Handler();

    protected boolean mIsPlaying = false; //flag indicating if music is playing
    protected long mSongDuration = 1;

    private Context mContext;

    private RemoteController.OnClientUpdateListener mClientUpdateListener = new RemoteController.OnClientUpdateListener() {

        @Override
        public void onClientPlaybackStateUpdate(int state, long stateChangeTimeMs, long currentPosMs, float speed) {
            switch(state) {
                case RemoteControlClient.PLAYSTATE_PLAYING:
                    mIsPlaying = true;
                    break;
                default:
                    mIsPlaying = false;
                    break;
            }
        }

        @Override
        public void onClientTransportControlUpdate(int transportControlFlags){
        }

        @Override
        public void onClientPlaybackStateUpdate(int state) {
            switch(state) {
                case RemoteControlClient.PLAYSTATE_PLAYING:
                    mIsPlaying = true;
                    break;
                default:
                    mIsPlaying = false;
                    break;
            }
        }

        @Override
        public void onClientMetadataUpdate(MetadataEditor editor) {

            Log.d (TAG, "MetadataUpdate");

            //some players write artist name to METADATA_KEY_ALBUMARTIST instead of METADATA_KEY_ARTIST, so we should double-check it
            mArtistText.setText(editor.getString(MediaMetadataRetriever.METADATA_KEY_ARTIST,
                    editor.getString(MediaMetadataRetriever.METADATA_KEY_ALBUMARTIST, mContext.getString(R.string.unknown))
            ));

            mTitleText.setText(editor.getString(MediaMetadataRetriever.METADATA_KEY_TITLE, mContext.getString(R.string.unknown)));
            mAlbumText.setText(editor.getString(MediaMetadataRetriever.METADATA_KEY_ALBUM, mContext.getString(R.string.unknown)));

            mArtwork.setImageBitmap(editor.getBitmap(MetadataEditor.BITMAP_KEY_ARTWORK, null));
        }

        @Override
        public void onClientChange(boolean clearing) {

        }
    };

    public StatusBarMediaView(Context context, AttributeSet attrs) {
        super(context, attrs);
        mContext = context;
        Log.d(TAG,"onCreate");
    }

    @Override
    public void onFinishInflate() {
        Log.d (TAG,"Enter onFinishInflate");
        mTitleText = (TextView)findViewById(R.id.title_text);
        mAlbumText = (TextView)findViewById(R.id.album_text);
        mArtistText = (TextView)findViewById(R.id.artist_text);

        mArtwork = (ImageView)findViewById(R.id.album_art);
        if (getVisibility() == View.VISIBLE){
            startService();
        }
    }

    @Override
    public void onVisibilityChanged(View changedView, int visibility) {
        Log.d(TAG,"Entering onVisibilityChanged");
        if (changedView == this) {
            switch (visibility) {
                case View.VISIBLE:
                    startService();
                    break;
                case View.INVISIBLE:
                    stopService();
                    break;
                case View.GONE:
                    stopService();
                    break;
            }
        }
    }

    private void startService() {
        Log.d(TAG,"StartService");
        Intent intent = new Intent(START_REMOTE_CONTROLLER_ACTION);
        mContext.bindService(intent, mConnection, Context.BIND_AUTO_CREATE);
    }

    private void stopService() {
        Log.d(TAG,"StopService");
        if(mBound) {
            mRCService.setRemoteControllerDisabled();
        }
        mContext.unbindService(mConnection);
    }

    private ServiceConnection mConnection = new ServiceConnection() {
        @Override
        public void onServiceConnected(ComponentName className, IBinder service) {
            Log.d(TAG,"onServiceConnected");
            //Getting the binder and activating RemoteController instantly
            StatusbarMediaService.RCBinder binder = (StatusbarMediaService.RCBinder) service;
            mRCService = binder.getService();
            mRCService.setRemoteControllerEnabled();
            mRCService.setClientUpdateListener(mClientUpdateListener);
            mBound = true;
        }

        @Override
        public void onServiceDisconnected(ComponentName name) {
            mBound = false;
        }
    };

}
