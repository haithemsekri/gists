SSID=$(python3 -c 'print(",".join(str(i) for i in bytes("WIFI_SSID", "utf-8")))')
PASS=$(python3 -c 'print(",".join(str(i) for i in bytes("WIFI_PASS", "utf-8")))')
dbus-send --system --print-reply --dest=com.WIFI /com/WIFI WIFI.connect array:byte:$SSID,0  array:byte:$PASS,0
dbus-send --system --print-reply --dest=com.WIFI /com/WIFI WIFI.connect array:byte:65,83,69,32,77,111,98,105,108,101,0 array:byte:109,111,98,105,108,101,50,48,50,50,35,0
