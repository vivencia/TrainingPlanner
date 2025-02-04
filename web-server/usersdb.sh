#!/bin/bash

SCRIPT_NAME=$(basename "$0")
USER_DIR=$1
COMMAND=$2

BASE_SERVER_DIR=/var/www/html
TP_DIR=$BASE_SERVER_DIR/trainingplanner
ADMIN=admin
ADMIN_DIR=$TP_DIR/$ADMIN
USERS_DB=$TP_DIR/$ADMIN/users.db
COMMANDS_DIR=$TP_DIR/$USER_DIR
COMMAND_FILE=$COMMANDS_DIR/cmd.$COMMAND

if [ -d $COMMANDS_DIR ]; then
    if [ -f $COMMAND_FILE ]; then
        if [ $COMMAND == "add" ]; then
            if sqlite3 -line $USERS_DB "SELECT id FROM user_table WHERE id=$USER_DIR;"; then
                COMMAND_FILE=$COMMANDS_DIR/cmd.update
            fi
            $(cat $COMMAND_FILE | sqlite3 -line $USERS_DB)
        else
            if [ $COMMAND == "del" ]; then
                $(cat $COMMAND_FILE | sqlite3 -line $USERS_DB)
            fi
        fi
    fi
fi


exit $?
