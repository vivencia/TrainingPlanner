#!/bin/bash

#go to
#https://www.shellcheck.net/
#to spell check shell scripts and make them more robust

BASE_SERVER_DIR=/var/www/html
TP_DIR=$BASE_SERVER_DIR/trainingplanner
ADMIN="admin"
ADMIN_DIR="$TP_DIR/$ADMIN/"
USERS_DB=$ADMIN_DIR"users.db"
FIELDS_FILE=$ADMIN_DIR"user.fields"
SQLITE="/usr/bin/sqlite3"

SOURCES_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd ) #the directory of this script
cd "${SOURCES_DIR}" || exit
N_FIELDS=$(wc -l "${FIELDS_FILE}" | cut -d ' ' -f 1)
LAST_FIELD=$N_FIELDS
(( LAST_FIELD-- ))

user_exists() {
    RETURN_ID=$($SQLITE -line $USERS_DB "SELECT id FROM users_table WHERE id=$USER_ID;")
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
    if [ $i == $N_FIELDS ]; then
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
    if [[ $x == $N_FIELDS ]]; then
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
            while [ $i -lt $N_FIELDS ]; do
                if [ $i != $LAST_FIELD ]; then
                    UPDATE_CMD="$UPDATE_CMD ${FIELDS[$i]}='${VALUES[$i]}',"
                else
                    UPDATE_CMD="UPDATE users_table SET $UPDATE_CMD ${FIELDS[$i]}='${VALUES[$i]}' WHERE ${FIELDS[0]}='${VALUES[0]}';"
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
            if [ $z != $LAST_FIELD ]; then
                INSERT_CMD="$INSERT_CMD${field},"
            else
                INSERT_CMD="$INSERT_CMD${field}) VALUES("
            fi
            (( z++ ))
        done
        if get_values; then
            (( z=0 ))
            while [ $z -lt $N_FIELDS  ]; do
                if [ $z != $LAST_FIELD ]; then
                    INSERT_CMD="$INSERT_CMD'${VALUES[$z]}',"
                else
                    INSERT_CMD="INSERT INTO users_table ($INSERT_CMD'${VALUES[$z]}');"
                fi
                (( z++ ))
            done
            return 0
        fi
    fi
    return 1
}

DEL_CMD="DELETE FROM users_table WHERE id="
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
    done < <($SQLITE -line $USERS_DB "SELECT * FROM users_table WHERE id=$USER_ID;")
    if [[ $ALL_USER_VALUES != "" ]]; then
        return 0
    else
        return 1
    fi
}

REQUESTED_FIELD_VALUE=""
get_field_value() {
    REQUESTED_FIELD_VALUE=$($SQLITE -line $USERS_DB "SELECT $1 FROM users_table WHERE id=$USER_ID;")
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
get_id() {
    FIELD=$(echo "${1}" | cut -d '=' -f 1)
    VALUE=$(echo "${1}" | cut -d '=' -f 2)
    VALUE="'$VALUE'" #sqlite3 needs single quotes for string values
    REQUESTED_ID=$($SQLITE -line $USERS_DB "SELECT id FROM users_table WHERE $FIELD=$VALUE;")
    if [[ $REQUESTED_ID != "" ]]; then
        REQUESTED_ID=$(echo "${REQUESTED_ID}" | cut -d '=' -f 2)
        REQUESTED_PASSWD=$($SQLITE -line $USERS_DB "SELECT password FROM users_table WHERE id=$REQUESTED_ID;")
        REQUESTED_PASSWD=$(echo "${REQUESTED_PASSWD}" | cut -d '=' -f 2)
        TEMP_HT_FILE=$ADMIN_DIR$REQUESTED_ID".htpasswd"
        echo $REQUESTED_ID:$REQUESTED_PASSWD > $TEMP_HT_FILE
        /usr/bin/htpasswd -bv $TEMP_HT_FILE $REQUESTED_ID $REQUESTED_PASSWD > /dev/null 2>&1
        rm -f $TEMP_HT_FILE
        return_var = "$?";
        case "$return_var" in
            0) error_string = "User exists and password is correct";;
            3) error_string = "User exists and password is wrong";;
            6) error_string = "User does not exist";;
            *) error_string = "User does not exist";;
        esac
    else
        error_string = "User does not exist"
        return_var = 6
    fi
    echo $error_string
    return $return_var
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
                    echo "${UPDATE_CMD}" | $SQLITE $USERS_DB
                    exit $?
                else
                    echo "User update failed: No user.data file found or file corrupt"
                    exit 11
                fi
            else
                if get_insert_command; then
                    echo "${INSERT_CMD}" | $SQLITE $USERS_DB
                    exit $?
                else
                    echo "User insertion failed: No user.data file found or file corrupt"
                    exit 12
                fi
            fi
        ;;
        del)
            if [ "$USER_ID" == "" ]; then
                echo "Missing user ID argument"
                exit 9
            fi
            if user_exists; then
                get_del_command
                echo "${DEL_CMD}" | $SQLITE $USERS_DB
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
            get_id "${2}" "${3}"
            exit $?
        ;;
        *)
            USER_ID=$var
            USER_DIR=$TP_DIR/$USER_ID
            DATA_FILE=$USER_DIR/user.data
        ;;
    esac
done
