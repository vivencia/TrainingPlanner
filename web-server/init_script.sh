#!/usr/bin/env bash

#go to
#https://www.shellcheck.net/
#to spell check shell scripts and make them more robust

BASE_SERVER_DIR="/var/www/html"
TP_DIR=$BASE_SERVER_DIR"/trainingplanner"
PHP_FPM_SERVICE="php-fpm"
SCRIPT_NAME=$(basename "$0")
USER_NAME=$(whoami)
PASSWORD=""

print_usage() {
	echo "Usage: $SCRIPT_NAME {setup|status|start|stop|restart|pause|createdb}" >&2
}

get_passwd() {
    if [ ! "$PASSWORD" ]; then
        read -p "Sudo's password: " -sr
        PASSWORD=$REPLY
        if ! echo "$PASSWORD" | sudo -S whoami | grep -q "root"; then
            echo "Wrong sudo password. Exiting..."
            return 1
        fi
        echo
    else
        if ! echo "$PASSWORD" | sudo -S whoami | grep -q "root"; then
            echo "Wrong sudo password. Exiting..."
            return 1
        fi
    fi
    return 0
}

run_as_sudo() {
    if get_passwd; then
        echo "$PASSWORD" | sudo -S "$@" | grep -q "root"
        return 0
    fi
    exit 3
}

for i in "$@"; do
    case $i in
        -p=*|--password=*)
            PASSWORD="${i#*=}"
            shift # past argument=value
        ;;
        -h|--help)
            print_usage
            exit 0
        ;;
        --*|-*)
            echo "Unknown option $i"
            exit 1
        ;;
        *)
            COMMAND="${i}"
        ;;
    esac
done

SOURCES_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd ) #the directory of this script
SCRIPTS_DIR=$TP_DIR/scripts

NGINX="$(which nginx)"
NGINX_CONFIG_DIR="/etc/nginx"
NGINX_SERVER_CONFIG_DIR="/etc/nginx/conf.d"
NGINX_USER=www-data

PHP_FPM_CONFIG_DIR=/etc/php/php-fpm.d

ADMIN="admin"
ADMIN_DIR=$TP_DIR/$ADMIN
USERS_DB="$TP_DIR/$ADMIN/users.db"
PAUSE_FILE="$TP_DIR/pause"

create_admin_user() {
    PASS_FILE=$ADMIN_DIR/.passwds
    HTPASSWD=$(which htpasswd)
    if "$HTPASSWD" -bv $PASS_FILE $ADMIN $ADMIN; then
        echo "Creating the main app user"
        #always use the -B option for htpasswd to create bcrypt hashes compatible with PHP's password_verify()
        if run_as_sudo "$HTPASSWD" -cbB $PASS_FILE $ADMIN $ADMIN; then
            run_as_sudo mkdir -m 774 $ADMIN_DIR
            run_as_sudo cp -f "$SOURCES_DIR/user.fields" $ADMIN_DIR
            run_as_sudo chown -R $NGINX_USER:$NGINX_USER $ADMIN_DIR
            echo "Main app user created successfully"
            return 0
        fi
    fi
    return 1
}

create_users_db() {
    run_as_sudo rm -f "${USERS_DB}"
    if sqlite3 -line ${USERS_DB} 'CREATE TABLE IF NOT EXISTS users_table (userid INTEGER PRIMARY KEY, inserttime INTEGER, onlineaccount INTEGER, name TEXT, birthday INTEGER, sex INTEGER, phone TEXT, email TEXT, social TEXT, role TEXT, coach_role TEXT, goal TEXT, use_mode INTEGER, password TEXT);' &>/dev/null; then
        run_as_sudo chown -R $NGINX_USER:$NGINX_USER $USERS_DB
        run_as_sudo chmod 664 $USERS_DB
        echo "Users database created"
        return 0
    else
        echo "Failed to create users database: " $USERS_DB
        return 1
    fi
}

find_local_ip() {
    SERVER_IP=""
    INTERFACE_DATA=$(ip addr show | grep 'inet.*wlan0')
    N_WORDS=$(echo "${INTERFACE_DATA}" | awk -F ' ' '{ print NF }')
    (( N_WORDS++ ))
    (( z=1 ))
    while [ $z -ne "$N_WORDS" ]; do
        WORD=$(echo "${INTERFACE_DATA}" | cut -d ' ' -s -f $z)
        if [ $WORD ]; then
            SERVER_IP=$(echo "${WORD}" | cut -d '/' -s -f 1)
            if [ $SERVER_IP ]; then
                return 0
            fi
        else
            (( N_WORDS++ ))
        fi
        (( z++))
    done
    return 1
}

query_address() {
    SERVER_RESPONSE=$(curl -s "$1/trainingplanner/")
    echo "$SERVER_RESPONSE" | grep -q "Forbidden" && RESULT=1 || RESULT=0
    if [ "$RESULT" == 0 ]; then
        echo "$SERVER_RESPONSE" | grep -q "Bad Gateway" && RESULT=1 || RESULT=0
        if [ "$RESULT" == 0 ]; then
            echo "$SERVER_RESPONSE" | grep -q "Welcome" && RESULT=0 || RESULT=1
            if [ "$RESULT" == 1 ]; then
                echo "$SERVER_RESPONSE" | grep -q "paused" && RESULT=2 || RESULT=0
            fi
        fi
    fi
    return $RESULT
}

test_tp_server() {
    query_address "$1"
    case "$?" in
        0)
            if [ "$2" == "lan" ]; then
                echo "TPSERVER up and running($1)."
                return 0
            else
                echo "TPSERVER is running on localhost only."
                return 5
            fi
        ;;
        1)
            if [ $2 != "lan" ]; then
                echo "TPSERVER not running in the local network. Trying localhost only"
                test_tp_server "http://localhost:8080"
                return $?
            else
                echo "TPSERVER is not reachable."
                return 1
            fi
        ;;
        2)
            if [ $2 == "lan" ]; then
                echo "TPSERVER paused."
                return 6
            else
                echo "TPSERVER paused on localhost only."
                return 7
            fi
        ;;
    esac
}

start_nginx() {
    if [ -f "$NGINX" ]; then
        if pgrep -fl "$NGINX" &>/dev/null; then
            echo "The NGINX service is already running."
        else
            echo "Starting NGINX..."
            if ! run_as_sudo systemctl start nginx; then
                echo "Error starting nginx service."
                return 2
            else
                echo "The NGINX service started successfully."
            fi
        fi
        return 0;
    else
        echo "Please install NGINX."
        return 2
    fi
    return 0
}

stop_nginx() {
    if [ -f "$NGINX" ]; then
        if pgrep -fl "$NGINX" &>/dev/null; then
            echo "Stopping the NGINX service..."
            if ! run_as_sudo systemctl stop nginx; then
                echo "Error starting the NGINX service."
                return 2
            else
                echo "The NGINX service started successfully."
            fi
        else
            echo "The NGINX service is not running."
        fi
        return 0
    else
        echo "Please install NGINX."
        return 2
    fi
}

start_phpfpm() {
    if pgrep -fl $PHP_FPM_SERVICE &>/dev/null; then
        echo "The PHP-FPM service is already running."
    else
        echo "Starting PHP-FPM..."
        if [ ! -f "/run/php" ]; then
            run_as_sudo mkdir "/run/php"
        fi
        if ! run_as_sudo systemctl start $PHP_FPM_SERVICE; then
            echo "Error starting the PHP-FPM service."
            return 3
        else
            echo "The PHP-FPM service started successfully."
        fi
    fi
    return 0
}

stop_phpfpm() {
    if pgrep -fl $PHP_FPM_SERVICE &>/dev/null; then
        echo "Stopping PHP-FPM..."
        if ! run_as_sudo systemctl stop $PHP_FPM_SERVICE; then
            echo "PHP FPM service failed to stop"
            return 3
        else
            echo "PHP FPM service stopped successfully."
        fi
    else
        echo "Service php-fpm is not running."
    fi
    return 0
}

start_server() {
    start_nginx
    EXIT_STATUS=$?
    if [ "$EXIT_STATUS" == 0 ]; then
        start_phpfpm
        EXIT_STATUS=$?
    fi
    if [ "$EXIT_STATUS" == 0 ]; then
        if find_local_ip; then
            test_tp_server "$SERVER_IP:8080" "lan"
            EXIT_STATUS=$?
        else
            EXIT_STATUS=1
        fi
    else
        echo "TPSERVER not running because PHP-FPM failed to start"
    fi
    return $EXIT_STATUS
}

stop_server() {
    stop_nginx
    EXIT_STATUS=$?
    if [ "$EXIT_STATUS" == 0 ]; then
        stop_phpfpm
        EXIT_STATUS=$?
    fi
    if [ "$EXIT_STATUS" == 0 ]; then
        echo "Local TP server stopped!"
    else
        echo "Local TP server failed to stop"
    fi
    return $EXIT_STATUS
}

pause_server() {
    echo "1" > $PAUSE_FILE
    echo "TPSERVER paused"
}

unpause_server() {
    echo "0" > $PAUSE_FILE
    echo "TPSERVER running"
}

setup_tpserver() {
    echo "Beginning TP Server configuration..."
    if [ ! -d "$TP_DIR" ]; then
        echo "Preparing the firesystem layout and copying configuration files to their respective locations..."

        run_as_sudo groupadd $NGINX_USER
        run_as_sudo useradd -r $NGINX_USER -g $NGINX_USER -G network,sys

        run_as_sudo mkdir -p $SCRIPTS_DIR
        run_as_sudo chmod -R 770 $TP_DIR
        run_as_sudo chmod -R 770 $SCRIPTS_DIR
        run_as_sudo cp "$SOURCES_DIR/$SCRIPT_NAME" $SCRIPTS_DIR #copy this file to the scripts dir
        run_as_sudo cp "$SOURCES_DIR/url_parser.php" $SCRIPTS_DIR
        run_as_sudo cp "$SOURCES_DIR/usersdb.sh" $SCRIPTS_DIR
        run_as_sudo chown -R $NGINX_USER:$NGINX_USER $TP_DIR

        run_as_sudo cp -f "$SOURCES_DIR/nginx.conf" $NGINX_CONFIG_DIR
        run_as_sudo chown root:root "$NGINX_CONFIG_DIR/nginx.conf"

        run_as_sudo mkdir $NGINX_SERVER_CONFIG_DIR
        run_as_sudo cp -f "$SOURCES_DIR/tpserver.conf" $NGINX_SERVER_CONFIG_DIR
        run_as_sudo chown root:root "$NGINX_SERVER_CONFIG_DIR/tpserver.conf"

        run_as_sudo cp -f "$SOURCES_DIR/www.conf" $PHP_FPM_CONFIG_DIR
        create_admin_user
        create_users_db

        if ! /usr/bin/id -nG "$USER_NAME" | grep -qw $NGINX_USER; then
            run_as_sudo usermod -a -G $NGINX_USER "$(whoami)"
            echo "Filesystem directories and files setup."
        else
            echo "Filesystem directories and files setup. Log out and in again in order for the changes to work."
            return 0
        fi
    else
        start_server
        return $?
    fi
}

get_tpserver_status() {
    if [ -d "$TP_DIR" ]; then
        systemctl status nginx 1&> /dev/null
        NGINX_SETUP=$?
        PHP_FPM_SETUP=1
        TEST_RESULT=1
        if [ $NGINX_SETUP == 0 ]; then
            systemctl status $PHP_FPM_SERVICE 1&> /dev/null
            PHP_FPM_SETUP=$?
            if find_local_ip; then
                test_tp_server "$SERVER_IP:8080" "lan"
                TEST_RESULT=$?
            else
                TEST_RESULT=1
            fi
        fi

        if [ "$TEST_RESULT" == 1 ]; then
            if [ $NGINX_SETUP == 0 ]; then
                MESSAGE="$MESSAGE NGINX service is up and running."
            else
                TEST_RESULT=2
                MESSAGE="$MESSAGE NGINX service is not running."
            fi
            if [ $PHP_FPM_SETUP == 0 ]; then
                MESSAGE="$MESSAGE PHP-FPM service is up and running."
            else
                TEST_RESULT=3
                MESSAGE="$MESSAGE PHP-FPM service is not running."
            fi
            echo $MESSAGE
        fi
    else
        TEST_RESULT=4
        echo "TPSERVER needs to be setup. Run" $SCRIPT_NAME "with the setup option."
    fi

    return $TEST_RESULT
}

case "$COMMAND" in
    status)
        get_tpserver_status
        exit $?
    ;;
    setup)
        setup_tpserver
        exit $?
    ;;
    start)
        start_server
        exit $?
    ;;
    stop)
        stop_server
        exit $?
    ;;
    restart)
        stop_server
        if [ $? == 0 ]; then
            start_server
        fi
    ;;
    pause)
        get_tpserver_status
        if [[ $? == 0 || $? == 6 ]]; then
            if [ ! -f $PAUSE_FILE ]; then
                pause_server
            else
                PAUSE=$(head -n 1 "$PAUSE_FILE")
                if [ "$PAUSE" == 1 ]; then
                    unpause_server
                else
                    pause_server
                fi
            fi
            exit $?
        else
            echo "Cannot pause TPSERVER because it's not running."
            exit 8
        fi
    ;;
    createdb)
        if create_admin_user; then
            create_users_db
        fi
        exit $?
    ;;
    *)
        echo "Unknown option"
        print_usage
        exit 11
    ;;
esac
