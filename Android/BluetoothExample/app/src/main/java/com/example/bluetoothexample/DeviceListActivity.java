package com.example.bluetoothexample;

import android.app.Activity;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Bundle;
import android.support.annotation.Nullable;
import android.util.Log;
import android.view.View;
import android.view.Window;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.ListView;
import android.widget.ProgressBar;
import android.widget.TextView;

import java.util.Set;

/**
 * Created by xujun on 2017/6/24.
 */

class DeviceListActivity extends Activity{
    public static final String TAG = DeviceListActivity.class.getSimpleName();

    public static final String EXTRA_DEVICE_ADDRESS = "device_address";

    private BluetoothAdapter mBtAdapter;

    private ArrayAdapter<String> mNewDevicesArrayAdapter;


    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        setContentView(R.layout.activity_device_list);

        setResult(Activity.RESULT_CANCELED);

        Button scanButton = (Button)findViewById(R.id.button_scan);

        scanButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                doDiscovery();
                view.setVisibility(View.GONE);
            }
        });

        ArrayAdapter<String> pairedDeviceArrayAdapter =
                new ArrayAdapter<String>(this, R.layout.device_name);
        mNewDevicesArrayAdapter = new ArrayAdapter<String>(this, R.layout.device_name);

        ListView pairedListView = (ListView) findViewById(R.id.paired_devices);
        pairedListView.setAdapter(pairedDeviceArrayAdapter);
        pairedListView.setOnItemClickListener(mDeviceClickListener);

        ListView newDevicesListView = (ListView) findViewById(R.id.new_devices);
        newDevicesListView.setAdapter(mNewDevicesArrayAdapter);
        newDevicesListView.setOnItemClickListener(mDeviceClickListener);

        IntentFilter filter = new IntentFilter(BluetoothDevice.ACTION_FOUND);
        this.registerReceiver(mReceiver, filter);

        filter = new IntentFilter(BluetoothAdapter.ACTION_DISCOVERY_FINISHED);
        this.registerReceiver(mReceiver, filter);

        mBtAdapter = BluetoothAdapter.getDefaultAdapter();

        Set<BluetoothDevice> pairedDevices = mBtAdapter.getBondedDevices();

        if(pairedDevices.size() > 0)
        {
            findViewById(R.id.title_paried_devices).setVisibility(View.VISIBLE);
            for(BluetoothDevice device: pairedDevices) {
                pairedDeviceArrayAdapter.add(device.getName() + "\n" + device.getAddress());
            }

        }else {
            String noDeivces = getResources().getText(R.string.no_devcie_have_been_paired).toString();
            pairedDeviceArrayAdapter.add(noDeivces);
        }

        findViewById(R.id.progressbar).setVisibility(View.GONE);


    }

    @Override
    protected void onDestroy() {
        super.onDestroy();

        if(mBtAdapter != null) {
            mBtAdapter.cancelDiscovery();
        }
        this.unregisterReceiver(mReceiver);

    }

    private void doDiscovery() {
        Log.d(TAG, "doDiscovery()");
        ProgressBar progressBar =  (ProgressBar)findViewById(R.id.progressbar);
        progressBar.setVisibility(View.VISIBLE);
        setTitle(R.id.title_scan_devices);

        findViewById(R.id.title_new_devices).setVisibility(View.VISIBLE);

        if(mBtAdapter.isDiscovering())
        {
            mBtAdapter.cancelDiscovery();
        }

        mBtAdapter.startDiscovery();

    }

    private AdapterView.OnItemClickListener mDeviceClickListener = new AdapterView.OnItemClickListener() {
        @Override
        public void onItemClick(AdapterView<?> adapterView, View view, int i, long l) {
            mBtAdapter.cancelDiscovery();

            String info = ((TextView) view).getText().toString();
            String address = info.substring(info.length() - 17);

            Intent intent = new Intent();
            intent.putExtra(EXTRA_DEVICE_ADDRESS, address);

            setResult(Activity.RESULT_OK, intent);

            finish();
        }
    };

    private BroadcastReceiver mReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();

            if(BluetoothDevice.ACTION_FOUND.equals(action)) {
                BluetoothDevice device = intent.getParcelableExtra(BluetoothDevice.EXTRA_DEVICE);

                if(device.getBondState() != BluetoothDevice.BOND_BONDED) {
                    mNewDevicesArrayAdapter.add(device.getName() + "\n" + device.getAddress());
                }
            } else if (BluetoothAdapter.ACTION_DISCOVERY_FINISHED.equals(action)) {
                findViewById(R.id.progressbar).setVisibility(View.GONE);
                setTitle(R.string.title_select_device);

                if(mNewDevicesArrayAdapter.getCount() == 0) {
                    String noDevice = getResources().getText(R.string.no_devices_found).toString();
                    mNewDevicesArrayAdapter.add(noDevice);
                }
            }


        }
    };

}
