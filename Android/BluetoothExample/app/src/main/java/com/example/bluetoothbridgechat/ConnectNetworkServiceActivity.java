package com.example.bluetoothbridgechat;

import android.app.Activity;
import android.content.Intent;
import android.support.annotation.Nullable;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;

public class ConnectNetworkServiceActivity extends AppCompatActivity {

    public static final String EXTRA_SERVICE_ADDRESS = "address";
    public static final String EXTRA_SERVICE_PORT = "port";

    private EditText mAddressEdit;
    private EditText mPortEdit;

    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        setContentView(R.layout.activity_connect_network_service);

        setResult(Activity.RESULT_CANCELED);
        mAddressEdit = (EditText)findViewById(com.example.bluetoothbridgechat.R.id.address_edit);
        mPortEdit = (EditText)findViewById(com.example.bluetoothbridgechat.R.id.port_edit);

        Button connectButton = (Button)findViewById(R.id.connect_service_button);

        connectButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                String address = mAddressEdit.getText().toString();
                int port = Integer.valueOf(mPortEdit.getText().toString());


                Intent intent = new Intent();
                intent.putExtra(EXTRA_SERVICE_ADDRESS, address);
                intent.putExtra(EXTRA_SERVICE_PORT, port);
                setResult(Activity.RESULT_OK, intent);
                finish();
            }
        });

    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
    }
}
