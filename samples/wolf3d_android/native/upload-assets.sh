DEST=/sdcard/wolf3

ADB=$ANDROID_HOME/platform-tools/adb

# any active device ?
DEVICE_COUNT=`$ADB devices | grep -c device$`
if [[ "$DEVICE_COUNT" -eq 0 ]]; then
	echo "done, no active device"
	exit 1
fi

# push files
$ADB shell mkdir -p $DEST
$ADB push -p assets $DEST
$ADB shell ls -la $DEST

echo "done."
