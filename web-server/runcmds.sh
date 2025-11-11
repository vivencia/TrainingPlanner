#!/usr/bin/env bash

#go to
#https://www.shellcheck.net/
#to spell check shell scripts and make them more robust

USERID=$1
CMDDIR=$2
CMDFILE=$3

BASE_SERVER_DIR="/var/www/html"
TP_DIR="$BASE_SERVER_DIR/trainingplanner"
DEVICES_FILE="$TP_DIR/$USERID/devices.txt"

cd "$TP_DIR/$USERID/$CMDDIR" || exit 1
#echo "Running command " $CMDFILE " under user directory $TP_DIR/$USERID/$CMDDIR"

declare -a DEVICES
if [ -f "$DEVICES_FILE" ]; then
	while IFS= read -r line || [[ -n "$line" ]]; do
		DEVICES+=("${line}")
		(( i++ ))
	done < "$DEVICES_FILE"
else
	echo "No devices file found in user's directory. Doing nothing. Exiting."
	exit 102
fi

N_DEVICES=${#DEVICES[@]}

if [ "$N_DEVICES" -eq 0 ]; then
	echo "No devices listed in devices file. Doing nothing. Exiting."
	exit 123
fi

check_device() {
	for device in "${DEVICES[@]}"; do
		if [ "$device" == "$2" ]; then
			return 0;
		fi
	done
	return 1;
}

#&& -w "/var/www/html/trainingplanner/1759185769213/Database/Users.db.sqlite"
if [[ -f "$CMDFILE" && -r "$CMDFILE" ]]; then
	DEVICE_ID=$(head -n 1 "$CMDFILE")
	if check_device $DEVICE_ID; then
		if source "$CMDFILE" 2>&1; then
			echo "0"
			exit 0
		else
			echo "Source command failed"
			exit 101
		fi
	fi
else
	echo "Error: File '$CMDFILE' does not exist or is not readable."
	exit 112
fi
