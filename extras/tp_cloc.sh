#/bin/bash

DATA_FILE=/DATA/tp_cloc.data
if [ ! -f $DATA_FILE ]; then
	touch $DATA_FILE
else
	echo >> $DATA_FILE
	echo >> $DATA_FILE
fi

date >> $DATA_FILE
/home/guilhermef/software/cloc/cloc /home/guilhermef/software/trainingplanner/cxx/ /home/guilhermef/software/trainingplanner/qml/ /home/guilhermef/software/trainingplanner/java/ /home/guilhermef/software/trainingplanner/web-server/ >> $DATA_FILE
exit 0
