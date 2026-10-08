#!/bin/bash

COMP_DATE="$(date +%Y%m%d)"
COMP_TIME="$(date +%H%M%S)"
DATA_FILE="./.build.info"
BUILD_NUMBER=1

if [ -f $DATA_FILE ]; then
	BUILD_NUMBER=$(cat "$DATA_FILE")
	(( BUILD_NUMBER++ ))
	echo "$BUILD_NUMBER" > "$DATA_FILE"
else
	echo "$BUILD_NUMBER" >> "$DATA_FILE"
fi

echo -e "$COMP_DATE-$COMP_TIME build $BUILD_NUMBER"
exit 0
