#!/bin/bash

#go to
#https://www.shellcheck.net/
#to spell check shell scripts and make them more robust

USER_ID=""
BASE_SERVER_DIR=/var/www/html
TP_DIR=$BASE_SERVER_DIR/trainingplanner
ADMIN="admin"
USERS_DB=$TP_DIR/$ADMIN/users.db
FIELDS_FILE=$TP_DIR/$ADMIN/user.fields
USER_DIR=$TP_DIR/$USER_ID
DATA_FILE=$USER_DIR/user.data
SQLITE=$(which sqlite3)

SOURCES_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd ) #the directory of this script
cd "${SOURCES_DIR}" || exit

user_exists() {
    RETURN_ID=$(sqlite3 -line $USERS_DB "SELECT id FROM user_table WHERE id=$USER_ID;")
    if [[ $RETURN_ID != "" ]]; then
        return 0
    else
        return 1
    fi
}

declare -a VALUES
#VALUES+=("" "" "" "" "" "" "" "" "" "" "" "" "" "")
get_values() {
    i=0;
    if [ -f $DATA_FILE ]; then
        while IFS= read -r line || [[ -n "$line" ]]; do
            VALUES+=("${line}")
            (( i++ ))
        done < $DATA_FILE
    fi
    rm -f $DATA_FILE #do not leave file behind. Might lead to problems if, for example, a file upload fails and this script uses an already used file
    if [ $i == 14 ]; then
        return 0
    else
        return 1
    fi
}

declare -a FIELDS
get_fields() {
    x=0;
    if [ -f $FIELDS_FILE ]; then
        while IFS= read -r line || [[ -n "$line" ]]; do
            FIELDS+=("${line}")
            (( x++ ))
        done < $FIELDS_FILE
    fi
    if [[ $x == 14 ]]; then
        return 0
    else
        return 1
    fi
}

UPDATE_CMD=""
get_update_command() {
    if get_fields; then
        if get_values; then
            i=1
            while [ $i -le 13 ]; do
                if [ $i != 13 ]; then
                    UPDATE_CMD="$UPDATE_CMD ${FIELDS[$i]}=${VALUES[$i]},"
                else
                    UPDATE_CMD="UPDATE user_table SET $UPDATE_CMD ${FIELDS[$i]}=${VALUES[$i]} WHERE ${FIELDS[0]}=${VALUES[0]};"
                fi
                (( i++ ))
            done
            return 0
        fi
    fi
    return 1
}

INSERT_CMD=""
get_insert_command() {
    if get_fields; then
        (( z=0 ))
        for field in "${FIELDS[@]}"; do
            if [ $z != 13 ]; then
                INSERT_CMD="$INSERT_CMD${field},"
            else
                INSERT_CMD="$INSERT_CMD${field}) VALUES("
            fi
            (( z++ ))
        done
        if get_values; then
            (( z=0 ))
            while [ $z -le 13 ]; do
                if [ $z != 13 ]; then
                    INSERT_CMD="$INSERT_CMD${VALUES[$z]},"
                else
                    INSERT_CMD="INSERT INTO user_table ($INSERT_CMD${VALUES[$z]});"
                fi
                (( z++ ))
            done
            return 0
        fi
    fi
    return 1
}

DEL_CMD="DELETE FROM user_table WHERE id="
get_del_command() {
    DEL_CMD="$DEL_CMD$USER_ID;"
}

ALL_USER_VALUES=""
get_all_values() {
    while read -r line; do
            VALUE=$(echo "${line}" | cut -d '=' -f 2)
            value_len=${#VALUE}
            VALUE=${VALUE:1:$value_len-1} #the first 1: start at the second char, skipping the space that sqlite3 puts before the value
            ALL_USER_VALUES=${ALL_USER_VALUES}${VALUE}$'\n'
    done < <(sqlite3 -line $USERS_DB "SELECT * FROM user_table WHERE id=$USER_ID;")
    if [[ $ALL_USER_VALUES != "" ]]; then
        return 0
    else
        return 1
    fi
}

REQUESTED_FIELD_VALUE=""
get_field_value() {
    REQUESTED_FIELD_VALUE=$(sqlite3 -line $USERS_DB "SELECT $1 FROM user_table WHERE id=$USER_ID;")
    if [[ $REQUESTED_FIELD_VALUE != "" ]]; then
        field_name_len=${#1}
        value_len=${#REQUESTED_FIELD_VALUE}
        REQUESTED_FIELD_VALUE=${REQUESTED_FIELD_VALUE:0-($value_len-$field_name_len-3)}
        return 0
    else
        return 1
    fi
}

REQUESTED_ID=""
#argument must be in the format: field=value i.e. email=john@mail.com
get_id() {
    FIELD=$(echo "${1}" | cut -d '=' -f 1)
    VALUE=$(echo "${1}" | cut -d '=' -f 2)
    VALUE="'$VALUE'" #sqlite3 needs single quotes for string values
    REQUESTED_ID=$($SQLITE -line $USERS_DB "SELECT id FROM user_table WHERE $FIELD=$VALUE;")
    if [[ $REQUESTED_ID != "" ]]; then
        REQUESTED_ID=$(echo "${REQUESTED_ID}" | cut -d '=' -f 2)
        return 0
    else
        return 1
    fi
}

check_user_dir()
{
    if [ ! -d $USER_DIR ]; then
        echo "User dir does not exist: $USER_DIR"
        return 1
    else
        return 0
    fi
}

for var in "$@"
do
        case "$var" in
        add)
            if ! check_user_dir; then
                exit 10
            fi
            if user_exists; then
                if get_update_command; then
                    sqlite3 -line $USERS_DB "$UPDATE_CMD"
                    exit $?
                else
                    echo "User update failed: No user.data file found or file corrupt"
                    exit 11
                fi
                #echo $UPDATE_CMD
            else
                if get_insert_command; then
                    sqlite3 -line $USERS_DB "$INSERT_CMD"
                    exit $?
                else
                    echo "User insertion failed: No user.data file found or file corrupt"
                    exit 12
                fi
                #echo $INSERT_CMD
            fi
        ;;
        del)
            if [ "$USER_ID" == "" ]; then
                echo "Missing user ID argument"
                exit 9
            fi
            if user_exists; then
                get_del_command
                #echo $DEL_CMD
                sqlite3 -line $USERS_DB "$DEL_CMD"
                exit $?
            else
                exit 13
            fi
        ;;
        getall)
            if [ "$USER_ID" == "" ]; then
                echo "Missing user ID argument"
                exit 9
            fi
            if get_all_values; then
                echo "$ALL_USER_VALUES"
                exit 0
            else
                exit 14
            fi
        ;;
        get)
            if [ "$USER_ID" == "" ]; then
                echo "Missing user ID argument"
                exit 9
            fi
            if get_field_value "${3}"; then
                echo "$REQUESTED_FIELD_VALUE"
                exit 0
            else
                exit 15
            fi
        ;;
        getid)
            if get_id "${2}"; then
                echo "$REQUESTED_ID"
                exit 0
            else
                exit 16
            fi
        ;;
        *)
            USER_ID=$var
        ;;
    esac
done
