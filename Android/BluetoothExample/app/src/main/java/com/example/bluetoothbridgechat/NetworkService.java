package com.example.bluetoothbridgechat;

import android.content.Context;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.util.Log;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.InetSocketAddress;
import java.net.Socket;

/**
 * Created by xujun on 2017/6/28.
 */

public class NetworkService {
    private static final String TAG = NetworkService.class.getSimpleName();

    public static final int STATE_NONE = 0;
    public static final int STATE_CONNECTING = 1;
    public static final int STATE_CONNECTED = 2;


    private int mState;
    private int mNewState;

    private final Handler mHandler;

    private Socket mSocket;


    private ConnectThread mConnectThread;
    private ConnectedThread mConnectedThread;


    public NetworkService(Context context, Handler handler) {
        mHandler = handler;
        mState = STATE_NONE;
        mNewState = mState;
    }

    private synchronized void updateStateToFrame() {
        mState = getState();
        Log.d(TAG, "updateStateToFrame: " + mNewState + "->" + mState);
        mNewState = mState;
        mHandler.obtainMessage(Constants.MESSAGE_STATE_CHANGE, mNewState, -1).sendToTarget();
    }

    public synchronized int getState() {
        return mState;
    }

    public synchronized void start() {
        Log.d(TAG, "start: ");

        if(mConnectThread != null) {
            mConnectThread.cancel();
            mConnectThread =  null;
        }
        if(mConnectedThread != null) {
            mConnectedThread.cancel();
            mConnectedThread = null;
        }

        updateStateToFrame();

    }

    public synchronized void connectTo(String address, Integer port) {
        Log.d(TAG, "connectTo: " + address + "(" + port + ")");

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

        mConnectThread = new ConnectThread(address, port);
        mConnectThread.start();
        updateStateToFrame();

    }

    public synchronized void connected(Socket socket) {
        Log.d(TAG, "connected: ");

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

        updateStateToFrame();
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
        updateStateToFrame();
    }


    public void write(byte[] bytes) {
        ConnectedThread r;
        synchronized (this) {
            if(mState != STATE_CONNECTED)
            {
                return;
            }
            r = mConnectedThread;
        }
        r.write(bytes);
    }

    private void connectionFailed() {
        Message msg = mHandler.obtainMessage(Constants.MESSAGE_TOAST);
        Bundle bundle = new Bundle();
        bundle.putString(Constants.TOAST, "未能连接到服务");
        msg.setData(bundle);
        mHandler.sendMessage(msg);

        mState = STATE_NONE;
        updateStateToFrame();

    }

    private void connectionLost() {
        Message msg = mHandler.obtainMessage(Constants.MESSAGE_TOAST);
        Bundle bundle = new Bundle();
        bundle.putString(Constants.TOAST, "失去到服务的连接");
        msg.setData(bundle);
        mHandler.sendMessage(msg);

        mState = STATE_NONE;
        updateStateToFrame();
    }



    private class ConnectThread extends Thread {
        private final Socket mmSocket;
        private final String mmAddress;
        private final Integer mmPort;
        public ConnectThread(String address, Integer port) {

            mmAddress = address;
            mmPort = port;
            mmSocket = new Socket();
            mState = STATE_CONNECTING;

        }

        public void run() {
            Log.i(TAG, "Begin mConnectThread");
            setName("ConnectThread");

            try {
                mmSocket.connect(new InetSocketAddress(mmAddress, mmPort));
            } catch (IOException e) {
                try {
                    mmSocket.close();
                } catch (IOException e1) {
                    Log.e(TAG, "unable to close() during connection failure", e1);

                }
                connectionFailed();
                return;
            }

            synchronized (NetworkService.this) {
                mConnectThread = null;
            }

            connected(mmSocket);

        }

        public void cancel() {
            Log.d(TAG, "connect cancle");

            try {
                mmSocket.close();
            } catch (IOException e) {
                Log.e(TAG, "close of connect failed", e );

            }
        }
    }

    private class ConnectedThread extends Thread {
        private final Socket mmSocket;
        private final InputStream mmInStream;
        private final OutputStream mmOutStream;

        public ConnectedThread(Socket socket) {
            Log.d(TAG, "create ConnectedThread ");
            mmSocket = socket;

            InputStream tmpIn = null;
            OutputStream tmpOut = null;

            try {
                tmpIn = socket.getInputStream();
                tmpOut = socket.getOutputStream();
            } catch (IOException e) {
                Log.e(TAG, "tmp sockets not created", e );
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
                    mHandler.obtainMessage(Constants.MESSAGE_READ, bytes, -1, buffer)
                            .sendToTarget();
                } catch (IOException e) {
                    Log.e(TAG, "diconnected", e );
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
                Log.e(TAG, "close() of connect socket failed", e );
            }
        }

        public void write(byte[] bytes) {
            Log.d(TAG, "connected write");

            try {
                mmOutStream.write(bytes);
                mHandler.obtainMessage(Constants.MESSAGE_WRITE, -1, -1, bytes)
                        .sendToTarget();
            } catch (IOException e) {
                Log.e(TAG, "Exception during write", e );
            }
        }
    }
}
