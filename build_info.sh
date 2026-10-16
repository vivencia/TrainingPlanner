#!/bin/bash

COMP_DATE="$(date +%Y%m%d)"
COMP_TIME="$(date +%H%M%S)"

#.build.info will be inside the build dir as CMake executes this, not the source dir
DATA_FILE="./.build.info"
BUILD_NUMBER=1

if [ -f $DATA_FILE ]; then
	BUILD_NUMBER=$(cat "$DATA_FILE")
	(( BUILD_NUMBER++ ))
	echo "$BUILD_NUMBER" > "$DATA_FILE"
else
	echo "$BUILD_NUMBER" >> "$DATA_FILE"
fi

echo -e "#pragma once
#define TP_APP_VERSION \"$COMP_DATE\"
#define TP_APP_BUILD \"$BUILD_NUMBER\"" > "$(dirname $0)/cxx/version.h"

echo -e "Starting build $COMP_DATE-$COMP_TIME build $BUILD_NUMBER"
exit 0
