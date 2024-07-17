#!/bin/sh

QTDIR=${HOME}/Qt/6.7.2
QTBINDIR=${QTDIR}/gcc_64/bin
TPHOME=${HOME}/software/trainingplanner
TRDIR=${TPHOME}/translations

cd ${TPHOME}

case "$1" in
  1)
        ${QTBINDIR}/lupdate ${TPHOME}/cxx/* ${TPHOME}/qml/* -no-obsolete -ts ${TRDIR}/tplanner.pt_BR.ts
    ;;

  2)
        ${QTBINDIR}/lrelease ${TRDIR}/tplanner.pt_BR.ts
    ;;
esac

exit 0
