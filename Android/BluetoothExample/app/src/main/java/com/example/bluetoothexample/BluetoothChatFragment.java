package com.example.bluetoothexample;

import android.app.Activity;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.content.Intent;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.support.annotation.Nullable;
import android.support.v4.app.Fragment;
import android.support.v4.app.FragmentActivity;
import android.support.v4.app.NotificationCompat;
import android.app.ActionBar;
import android.util.Log;
import android.view.KeyEvent;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.View;
import android.view.ViewGroup;
import android.view.inputmethod.EditorInfo;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ListView;
import android.widget.TextView;
import android.widget.Toast;


public class BluetoothChatFragment extends Fragment {

    private static final String TAG = BluetoothChatFragment.class.getSimpleName();

    private static final int REQUEST_ENABLE_BLUETOOTH = 1;
    private static final int REQUEST_SEARCH_DEVICES = 2;

    private BluetoothAdapter mBluetoothAdapter = null;

    private BluetoothChatService mChatService = null;

    private ListView mMessageListView = null;
    private EditText mOutMessageEdit = null;
    private Button   mSendButton = null;

    private ArrayAdapter<String> mMessageListAdapter;
    private StringBuffer mOutStringBuffer;

    private String mConnectedDevcieName;


    public BluetoothChatFragment() {

    }

    @Override
    public void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setHasOptionsMenu(true);

        mBluetoothAdapter = BluetoothAdapter.getDefaultAdapter();

        if(mBluetoothAdapter == null) {
            FragmentActivity activity = getActivity();
            Toast.makeText(activity,
                    R.string.bluetooth_device_not_available,
                    Toast.LENGTH_LONG
                    ).show();
            activity.finish();
        }
    }

    @Override
    public void onStart() {
        super.onStart();

        if(!mBluetoothAdapter.isEnabled()) {
            Intent enableIntent = new Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE);
            startActivityForResult(enableIntent, REQUEST_ENABLE_BLUETOOTH);
        } else if(mChatService == null) {
            startChartService();
        }
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        if( mChatService != null) {
            mChatService.stop();
        }
    }

    @Override
    public void onResume() {
        super.onResume();

        if(mChatService != null) {
            if(mChatService.getState() == BluetoothChatService.STATE_NONE) {
                mChatService.start();
            }
        }
    }

    @Nullable
    @Override
    public View onCreateView(LayoutInflater inflater, @Nullable ViewGroup container, @Nullable Bundle savedInstanceState) {
        //return super.onCreateView(inflater, container, savedInstanceState);
        return inflater.inflate(R.layout.fragment_bluetooth_chat, container, false);
    }

    @Override
    public void onViewCreated(View view, @Nullable Bundle savedInstanceState) {

        mMessageListView = (ListView) view.findViewById(R.id.message_list_view);
        mOutMessageEdit = (EditText)view.findViewById(R.id.out_message_edit);
        mSendButton = (Button)view.findViewById(R.id.button_send);

    }


    private void startChartService() {
        Log.d(TAG, "startChartService");

        mMessageListAdapter = new ArrayAdapter<String>(getActivity(), R.layout.message);
        mMessageListView.setAdapter(mMessageListAdapter);

        mOutMessageEdit.setOnEditorActionListener(mWriteListener);

        mSendButton.setOnClickListener(
                new View.OnClickListener(){
                    @Override
                    public void onClick(View v) {

                        View view = getView();
                        if(null != view) {
                           TextView textView = (TextView) view.findViewById(R.id.out_message_edit);
                            String message = textView.getText().toString();
                            sendMessage(message);
                        }

                    }
                }
        );

        mChatService = new BluetoothChatService(getActivity(), mHandler);

        mOutStringBuffer = new StringBuffer("");

    }

    private TextView.OnEditorActionListener mWriteListener
            = new TextView.OnEditorActionListener() {
        @Override
        public boolean onEditorAction(TextView textView, int actionId, KeyEvent event) {
            if(actionId == EditorInfo.IME_NULL && event.getAction() == KeyEvent.ACTION_UP) {
                String message = textView.getText().toString();
                sendMessage(message);

            }
            return true;
        }
    };

    private void sendMessage(String message) {
        if(mChatService.getState() != BluetoothChatService.STATE_CONNECTED) {
            Toast.makeText(getActivity(), R.string.not_connected, Toast.LENGTH_SHORT).show();
            return;
        }

        if(message.length() > 0) {
            byte[] send = (message + "\n").getBytes();
            mChatService.write(send);

            mOutStringBuffer.setLength(0);
            mOutMessageEdit.setText(mOutStringBuffer);
        }
    }

    private final Handler mHandler = new Handler(new Handler.Callback() {
        @Override
        public boolean handleMessage(Message message) {
            FragmentActivity activity = getActivity();

            switch (message.what) {
                case Constants.MESSAGE_STATE_CHANGE:
                    switch (message.arg1) {
                        case BluetoothChatService.STATE_CONNECTED:
                            setStatus(getString(R.string.title_connected_to, mConnectedDevcieName));
                            mMessageListAdapter.clear();
                            break;
                        case BluetoothChatService.STATE_CONNECTING:
                            setStatus(getString(R.string.title_connecting));
                            break;
                        case BluetoothChatService.STATE_LISTEN:
                        case BluetoothChatService.STATE_NONE:
                            setStatus(getString(R.string.title_not_connected));
                    }
                    break;
                case Constants.MESSAGE_WRITE:
                    byte[] writeBuf = (byte[]) message.obj;
                    String writeMessage = new String(writeBuf);
                    mMessageListAdapter.add("Me: " + writeMessage);
                    break;
                case Constants.MESSAGE_READ:
                    byte[] readBuf = (byte[]) message.obj;
                    String readmessage = new String(readBuf, 0, message.arg1);
                    mMessageListAdapter.add(mConnectedDevcieName + ": " + readmessage);
                    break;
                case Constants.MESSAGE_DEVICE_NAME:
                    mConnectedDevcieName = message.getData().getString(Constants.DEVICE_NAME);

                    if(null != activity) {
                        Toast.makeText(activity, "已连接到" + mConnectedDevcieName,
                                Toast.LENGTH_SHORT).show();
                    }
                    break;
                case Constants.MESSAGE_TOAST:
                    if(null != activity) {
                        Toast.makeText(activity, message.getData().getString(Constants.TOAST),
                                Toast.LENGTH_SHORT).show();
                    }
                    break;
            }
            return true;
        }
    });

    private void setStatus(int resId) {
        FragmentActivity activity = getActivity();
        if(null == activity)
        {
            return ;
        }

        final ActionBar actionBar = activity.getActionBar();
        if(null == actionBar) {
            return;
        }

        actionBar.setSubtitle(resId);

    }
    private void setStatus(CharSequence subTitle) {
        FragmentActivity activity = getActivity();
        if(null == activity)
        {
            return ;
        }

        final ActionBar actionBar = activity.getActionBar();
        if(null == actionBar) {
            return;
        }

        actionBar.setSubtitle(subTitle);

    }

    @Override
    public void onActivityResult(int requestCode, int resultCode, Intent data) {
        switch (requestCode) {
            case REQUEST_SEARCH_DEVICES:
                if(resultCode == Activity.RESULT_OK) {
                    connectDevice(data);
                }
                break;
            case REQUEST_ENABLE_BLUETOOTH:
                if(resultCode == Activity.RESULT_OK) {
                    startChartService();
                } else {
                    Log.d(TAG, "Bluetooth cannot enable");
                    Toast.makeText(getActivity(), R.string.bluetooth_device_cannot_available,
                            Toast.LENGTH_LONG).show();
                    getActivity().finish();
                }
        }
    }

    private void connectDevice(Intent data) {
        String address = data.getExtras().getString(DeviceListActivity.EXTRA_DEVICE_ADDRESS);

        BluetoothDevice device = mBluetoothAdapter.getRemoteDevice(address);

        mChatService.connectTo(device);
    }

    @Override
    public void onCreateOptionsMenu(Menu menu, MenuInflater inflater) {
        inflater.inflate(R.menu.bluetooth_chat, menu);
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        switch (item.getItemId()){
            case R.id.menu_search_remote :
                Intent serverIntent = new Intent(getActivity(), DeviceListActivity.class);
                startActivityForResult(serverIntent, REQUEST_SEARCH_DEVICES);
                return true;
        }
        return false;

    }
}
