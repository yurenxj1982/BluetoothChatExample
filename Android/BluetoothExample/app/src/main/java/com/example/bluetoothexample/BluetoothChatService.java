package com.example.bluetoothexample;

import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothSocket;
import android.content.Context;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.util.Log;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.UUID;


/**
 * Created by xujun on 2017/6/24.
 */

public class BluetoothChatService {

    private static final String TAG = BluetoothChatService.class.getSimpleName();

    private static final String Service_Name = "BluetoothChat";
    private static final UUID Service_Uuid =
            UUID.fromString("e8e10f95-1a70-4b27-9ccf-02010264e9c8");

    private final BluetoothAdapter mAdapter;
    private final Handler mHandler;

    private ConnectThread mConnectThread;
    private ConnectedThread mConnectedThread;

    private int mNewState;
    private int mState;

    public static final int STATE_NONE = 0;
    public static final int STATE_LISTEN = 1;
    public static final int STATE_CONNECTING = 2;
    public static final int STATE_CONNECTED = 3;


    public BluetoothChatService(Context context, Handler handler) {
        mAdapter = BluetoothAdapter.getDefaultAdapter();
        mState = STATE_NONE;
        mNewState = mState;
        mHandler = handler;
    }

    private synchronized void updateUserInterfaceTitle() {
        mState = getState();
        Log.d(TAG, "updateUserInterfaceTitle: " + mNewState + "->" + mState);
        mNewState = mState;

        mHandler.obtainMessage(Constants.MESSAGE_STATE_CHANGE, mNewState, -1).sendToTarget();

    }

    public synchronized int getState() { return mState;}


    public synchronized void start() {
        Log.d(TAG, "start");

        if(mConnectThread != null) {
            mConnectThread.cancel();
            mConnectThread = null;
        }

        if(mConnectedThread != null) {
            mConnectedThread.cancel();
            mConnectedThread = null;
        }

        updateUserInterfaceTitle();

    }




    public synchronized void connectTo(BluetoothDevice device) {
        Log.d(TAG, "connectTo: " + device);

        if(mState == STATE_CONNECTING) {
            if(mConnectThread != null) {
                mConnectThread.cancel();
                mConnectThread = null;
            }
        }

        if(mConnectedThread != null) {
            mConnectedThread.cancel();
            mConnectedThread = null;
        }

        mConnectThread = new ConnectThread(device);
        mConnectThread.start();
        updateUserInterfaceTitle();
    }

    public synchronized void connected(BluetoothSocket socket, BluetoothDevice device) {

        Log.d(TAG, "connected");

        if(mConnectThread != null) {
            mConnectThread.cancel();
            mConnectThread = null;
        }

        if(mConnectedThread != null) {
            mConnectedThread.cancel();
            mConnectedThread = null;
        }

        mConnectedThread = new ConnectedThread(socket);
        mConnectedThread.start();

        Message msg = mHandler.obtainMessage(Constants.MESSAGE_DEVICE_NAME);
        Bundle bundle = new Bundle();
        bundle.putString(Constants.DEVICE_NAME, device.getName());
        msg.setData(bundle);
        mHandler.sendMessage(msg);
        updateUserInterfaceTitle();
    }

    public synchronized void stop() {

        if(mConnectThread != null) {
            mConnectThread.cancel();
            mConnectThread = null;
        }

        if(mConnectedThread != null) {
            mConnectedThread.cancel();
            mConnectedThread = null;
        }

        mState = STATE_NONE;
        updateUserInterfaceTitle();

    }

    public void write(byte[] send) {
        ConnectedThread r;
        synchronized (this) {
            if(mState != STATE_CONNECTED) return;
            r = mConnectedThread;
        }
        r.write(send);

    }

    private void connectionFailed() {
        Message msg = mHandler.obtainMessage(Constants.MESSAGE_TOAST);
        Bundle bundle = new Bundle();
        bundle.putString(Constants.TOAST, "未能连接设备");
        msg.setData(bundle);
        mHandler.sendMessage(msg);

        mState = STATE_NONE;

        updateUserInterfaceTitle();

        BluetoothChatService.this.start();
    }

    private void connectionLost() {

        Message msg = mHandler.obtainMessage(Constants.MESSAGE_TOAST);
        Bundle bundle = new Bundle();
        bundle.putString(Constants.TOAST, "已失去设备连接");
        msg.setData(bundle);
        mHandler.sendMessage(msg);

        mState = STATE_NONE;

        updateUserInterfaceTitle();

        BluetoothChatService.this.start();

    }

    private class ConnectThread extends Thread{
        private final BluetoothSocket mmSocket;
        private final BluetoothDevice mmDevice;

        public  ConnectThread(BluetoothDevice device) {
            mmDevice = device;
            BluetoothSocket tmp = null;

            try {
                tmp = device.createInsecureRfcommSocketToServiceRecord(Service_Uuid);
            }catch (IOException e) {
                Log.e(TAG, "Socket create() failed", e);
            }

            mmSocket = tmp;
            mState = STATE_CONNECTING;
        }

        public void run() {
            Log.i(TAG, "Begin mConnectThread");
            setName("ConnectThread");

            mAdapter.cancelDiscovery();
            try {
                mmSocket.connect();
            } catch (IOException e) {
                try {
                    mmSocket.close();
                } catch (IOException e1) {
                    Log.e(TAG, "unable to close() during connenction failure", e1);
                }
                connectionFailed();
                return;
            }

            synchronized (BluetoothChatService.this) {
                mConnectThread = null;
            }

            connected(mmSocket, mmDevice);
        }

        public void cancel() {
            Log.d(TAG, "connect cancel");
            try {
                mmSocket.close();
            } catch (IOException e) {
                Log.e(TAG, "close of connect failed", e);
            }
        }
    }

    private class ConnectedThread extends  Thread{
        private final BluetoothSocket mmSocket;
        private final InputStream mmInStream;
        private final OutputStream mmOutStream;

        public ConnectedThread(BluetoothSocket socket) {
            Log.d(TAG, "create ConnectedThread");
            mmSocket = socket;
            InputStream tmpIn = null;
            OutputStream tmpOut = null;

            try {
                tmpIn = socket.getInputStream();
                tmpOut = socket.getOutputStream();
            } catch (IOException e) {
                Log.e(TAG, "temp sockets not created", e);
            }

            mmInStream = tmpIn;
            mmOutStream = tmpOut;
            mState = STATE_CONNECTED;
        }

        public void run() {
            Log.i(TAG, "BEGIN mConnectedThread");
            byte[] buffer = new byte[1024];
            int bytes;

            while (mState == STATE_CONNECTED) {
                try {
                    bytes = mmInStream.read(buffer);

                    mHandler.obtainMessage(Constants.MESSAGE_READ, bytes, -1 ,buffer)
                            .sendToTarget();
                } catch (IOException e) {
                    Log.e(TAG, "disconnected", e);
                    connectionLost();
                    break;
                }
            }
        }

        public void cancel() {
            Log.d(TAG, "connected cancel");
            try {
                mmSocket.close();
            } catch (IOException e) {
                Log.e(TAG, "close() of connect socket failed", e);
            }
        }

        public void write(byte[] send) {
            Log.d(TAG, "connected write");
            try{
                mmOutStream.write(send);
                mHandler.obtainMessage(Constants.MESSAGE_WRITE, -1, -1, send)
                        .sendToTarget();
            } catch (IOException e) {
                Log.e(TAG, "Exception during write", e);
            }
        }
    }
}
