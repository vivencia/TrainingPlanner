#!/bin/sh

cd /home/guilherme/software/trainingplanner/translations/

case "$1" in
  1)
        lupdate /home/guilherme/software/trainingplanner/cxx/* /home/guilherme/software/trainingplanner/qml/*.qml -no-obsolete -ts tplanner.pt_BR.ts
    ;;

  2)
        lrelease tplanner.pt_BR.ts
    ;;
esac

exit 0
