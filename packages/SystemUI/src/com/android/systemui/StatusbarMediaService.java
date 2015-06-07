package com.android.systemui;

import java.lang.reflect.Field;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;

import android.content.Context;
import android.content.Intent;
import android.media.AudioManager;
import android.media.RemoteController;
import android.media.RemoteController.MetadataEditor;
import android.os.Binder;
import android.os.IBinder;
import android.service.notification.NotificationListenerService;
import android.service.notification.StatusBarNotification;
import android.util.Log;

public class StatusbarMediaService extends NotificationListenerService implements RemoteController.OnClientUpdateListener {

    private final static String TAG = "StatusBarMedia";
    public final static String START_REMOTE_CONTROLLER_ACTION = "com.android.systemui.START_REMOTE_CONTROLLER";

    //dimensions in pixels for artwork
    private static final int BITMAP_HEIGHT = 1024;
    private static final int BITMAP_WIDTH = 1024;

    //Binder for our service.
    private IBinder mBinder = new RCBinder();

    private RemoteController mRemoteController;
    private Context mContext;

    //external callback provided by user.
    private RemoteController.OnClientUpdateListener mExternalClientUpdateListener;

    @Override
    public IBinder onBind(Intent intent) {
        Log.d(TAG,"onBind");
        if(intent.getAction().equals(START_REMOTE_CONTROLLER_ACTION)) {
            return mBinder;
        } else {
            return super.onBind(intent);
        }
    }

    @Override
    public void onNotificationPosted(StatusBarNotification notification) {
    }

    @Override
    public void onNotificationRemoved(StatusBarNotification notification) {
    }

    @Override
    public void onCreate() {
        //saving the context for further reuse
        mContext = getApplicationContext();
        mRemoteController = new RemoteController(mContext, this);
    }

    @Override
    public void onDestroy() {
        setRemoteControllerDisabled();
    }

    //Following method will be called by Activity usign IBinder

    /**
     * Enables the RemoteController thus allowing us to receive metadata updates.
     */
    public void setRemoteControllerEnabled() {
        if(!((AudioManager)mContext.getSystemService(Context.AUDIO_SERVICE)).registerRemoteController(mRemoteController)) {
            throw new RuntimeException("Error while registering RemoteController!");
        } else {
            mRemoteController.setArtworkConfiguration(BITMAP_WIDTH, BITMAP_HEIGHT);
        }
    }

    /**
     * Disables RemoteController.
     */
    public void setRemoteControllerDisabled() {
        ((AudioManager)mContext.getSystemService(Context.AUDIO_SERVICE)).unregisterRemoteController(mRemoteController);
    }

    /**
     * Sets up external callback for client update events.
     * @param listener External callback.
     */
    public void setClientUpdateListener(RemoteController.OnClientUpdateListener listener) {
        mExternalClientUpdateListener = listener;
    }

    //end of Binder methods.


    //the most simple Binder implementation
    public class RCBinder extends Binder {
        public StatusbarMediaService getService() {
            return StatusbarMediaService.this;
        }
    }

    //implementation of RemoteController.OnClientUpdateListener. Does nothing other than calling external callback.
    @Override
    public void onClientChange(boolean arg0) {
        if(mExternalClientUpdateListener != null) {
            mExternalClientUpdateListener.onClientChange(arg0);
        }
    }

    @Override
    public void onClientMetadataUpdate(MetadataEditor arg0) {
        if(mExternalClientUpdateListener != null) {
            mExternalClientUpdateListener.onClientMetadataUpdate(arg0);
        }
    }

    @Override
    public void onClientPlaybackStateUpdate(int arg0) {
        if(mExternalClientUpdateListener != null) {
            mExternalClientUpdateListener.onClientPlaybackStateUpdate(arg0);
        }
    }

    @Override
    public void onClientPlaybackStateUpdate(int arg0, long arg1, long arg2, float arg3) {
        if(mExternalClientUpdateListener != null) {
            mExternalClientUpdateListener.onClientPlaybackStateUpdate(arg0, arg1, arg2, arg3);
        }
    }

    @Override
    public void onClientTransportControlUpdate(int arg0) {
        if(mExternalClientUpdateListener != null) {
            mExternalClientUpdateListener.onClientTransportControlUpdate(arg0);
        }

    }



}