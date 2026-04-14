package com.goliath.master.v75;

import android.app.AlertDialog;
import android.content.ClipData;
import android.content.ClipboardManager;
import android.content.Context;
import android.content.Intent;
import android.net.Uri;
import android.os.Build;
import android.os.Bundle;
import android.provider.Settings;
import android.view.Gravity;
import android.view.LayoutInflater;
import android.view.View;
import android.view.WindowManager;
import android.widget.Button;
import android.widget.EditText;
import android.widget.LinearLayout;
import android.widget.TextView;
import android.widget.Toast;
import androidx.appcompat.app.AppCompatActivity;

public class MainActivity extends AppCompatActivity {
    private static native boolean initGoliath(float height);
    private static native void toggleGoliath(boolean enable);
    private static native void setAuthSuccess(boolean success);
    private static native String getAdminEmail();
    private static native String generateLicenseKey(String hwid);

    private WindowManager wm;
    private View toggleView;
    private boolean isActive = false, isAuthorized = false;
    private String hwid;

    static { System.loadLibrary("goliath"); }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        
        if (!Settings.canDrawOverlays(this)) {
            startActivity(new Intent(Settings.ACTION_MANAGE_OVERLAY_PERMISSION));
            finish();
            return;
        }

        hwid = Build.SERIAL + Build.ID;
        showLicenseScreen();
    }

    private void showLicenseScreen() {
        LinearLayout layout = new LinearLayout(this);
        layout.setOrientation(LinearLayout.VERTICAL);
        layout.setPadding(60, 60, 60, 60);
        layout.setGravity(Gravity.CENTER);

        TextView title = new TextView(this);
        title.setText("🏆 GOLIATH V7.5 MASTER 🏆");
        title.setTextSize(28);
        title.setTextColor(0xFFFF5722);
        title.setGravity(Gravity.CENTER);
        title.setPadding(0, 0, 0, 40);

        TextView info = new TextView(this);
        info.setText("Precision Head-Lock Engine\n\nAdmin: " + getAdminEmail() + "\n\nDevice HWID:\n" + hwid.substring(0, 16));
        info.setTextSize(16);
        info.setTextColor(0xFFFFFFFF);
        info.setGravity(Gravity.CENTER);
        info.setPadding(0, 0, 0, 40);

        Button copyHWID = new Button(this);
        copyHWID.setText("📋 Copy HWID");
        copyHWID.setOnClickListener(v -> {
            ClipboardManager cm = (ClipboardManager) getSystemService(Context.CLIPBOARD_SERVICE);
            cm.setPrimaryClip(ClipData.newPlainText("HWID", hwid));
            Toast.makeText(this, "HWID Copied!", Toast.LENGTH_SHORT).show();
        });

        Button buyLicense = new Button(this);
        buyLicense.setText("💎 Buy License");
        buyLicense.setOnClickListener(v -> 
            startActivity(new Intent(Intent.ACTION_VIEW, Uri.parse("https://t.me/goliathv75"))));

        Button enterKey = new Button(this);
        enterKey.setText("🔑 Enter License Key");
        enterKey.setOnClickListener(v -> showKeyInput());

        layout.addView(title);
        layout.addView(info);
        layout.addView(copyHWID);
        layout.addView(buyLicense);
        layout.addView(enterKey);

        new AlertDialog.Builder(this)
            .setView(layout)
            .setCancelable(false)
            .setNegativeButton("Exit", null)
            .show();
    }

    private void showKeyInput() {
        EditText input = new EditText(this);
        input.setHint("Enter 32-char License Key");

        new AlertDialog.Builder(this)
            .setTitle("🔑 License Activation")
            .setView(input)
            .setPositiveButton("Activate", (d, w) -> {
                String key = input.getText().toString().trim();
                String expected = generateLicenseKey(hwid);
                if (key.equalsIgnoreCase(expected)) {
                    isAuthorized = true;
                    setAuthSuccess(true);
                    initGoliath(getResources().getDisplayMetrics().heightPixels);
                    createFloatingToggle();
                    Toast.makeText(this, "✅ GOLIATH V7.5 ACTIVATED", Toast.LENGTH_LONG).show();
                } else {
                    Toast.makeText(this, "❌ INVALID KEY", Toast.LENGTH_LONG).show();
                }
            })
            .show();
    }

    private void createFloatingToggle() {
        wm = (WindowManager) getSystemService(WINDOW_SERVICE);
        toggleView = new Button(this);
        ((Button) toggleView).setText("🎯 GOLIATH\nOFF");
        ((Button) toggleView).setOnClickListener(v -> {
            isActive = !isActive;
            toggleGoliath(isActive);
            ((Button) v).setText("🎯 GOLIATH\n" + (isActive ? "ON" : "OFF"));
        });

        WindowManager.LayoutParams params = new WindowManager.LayoutParams(
            180, 120,
            Build.VERSION.SDK_INT >= Build.VERSION_CODES.O ?
                WindowManager.LayoutParams.TYPE_APPLICATION_OVERLAY :
                WindowManager.LayoutParams.TYPE_PHONE,
            WindowManager.LayoutParams.FLAG_NOT_FOCUSABLE,
            android.graphics.PixelFormat.TRANSLUCENT
        );
        params.gravity = Gravity.TOP | Gravity.RIGHT;
        params.x = 30;
        params.y = 200;
        wm.addView(toggleView, params);
    }
}
