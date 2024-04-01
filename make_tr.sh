#!/bin/sh

cd /home/guilherme/software/trainingplanner/translations/


lupdate /home/guilherme/software/trainingplanner/cxx/* /home/guilherme/software/trainingplanner/qml/*.qml -no-obsolete -ts tplanner.pt_BR.ts
lrelease tplanner.pt_BR.ts
