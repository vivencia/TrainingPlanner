#!/usr/bin/env bash

#go to
#https://www.shellcheck.net/
#to spell check shell scripts and make them more robust

BASE_SERVER_DIR=/var/www/html
TP_DIR=$BASE_SERVER_DIR/trainingplanner
PHP_FPM_SERVICE=php-fpm
SCRIPT_NAME=$(basename "$0")
USER_NAME=$(whoami)
PASSWORD=""

print_usage() {
	echo "Usage: $SCRIPT_NAME {setup|status|start|stop|restart|dbcreate}" >&2
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
    if sqlite3 -line ${USERS_DB} 'CREATE TABLE IF NOT EXISTS users_table (userid INTEGER PRIMARY KEY, onlineuser INTEGER, name TEXT, birthday INTEGER, sex INTEGER, phone TEXT, email TEXT, social TEXT, role TEXT, coach_role TEXT, goal TEXT, use_mode INTEGER, password TEXT);' &>/dev/null; then
        run_as_sudo chown -R $NGINX_USER:$NGINX_USER $USERS_DB
        run_as_sudo chmod 664 $USERS_DB
        echo "Users database created"
        return 0
    else
        echo "Failed to create users database: " $USERS_DB
        return 1
    fi
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

        if pgrep -fl nginx &>/dev/null; then
            echo "Service nginx is already running."
        else
            echo "Starting http server..."
            if [ -f "$NGINX" ]; then
                if ! run_as_sudo systemctl start nginx; then
                    echo "Error starting nginx service."
                    return 4
                fi
            else
                echo "Please install nginx."
                return 4
            fi
        fi

        if pgrep -fl php-fpm &>/dev/null; then
            echo "Service php-fpm is already running."
        else
            echo "Starting php-fpm..."
            if ! run_as_sudo systemctl start $PHP_FPM_SERVICE; then
                echo "Error starting php-fpm service."
                return 5
            fi
        fi

        echo "Local TP server configured and running!"
        return 0
    fi
}

case "$COMMAND" in
    status)
        if [ -d "$TP_DIR" ]; then
            systemctl status nginx 1&> /dev/null
            NGINX_SETUP=$?
            PHP_FPM_SETUP=1
            if [ $NGINX_SETUP == 0 ]; then
                systemctl status $PHP_FPM_SERVICE 1&> /dev/null
                PHP_FPM_SETUP=$?
                curl -s http://192.168.10.21/trainingplanner/ | grep -q "Bad Gateway" && TP_SERVER=1 || TP_SERVER=0
                if [ "$TP_SERVER" == 0 ]; then
                    curl -s http://192.168.10.21/trainingplanner/ | grep -q "Welcome" && TP_SERVER=0 || TP_SERVER=1
                fi
            fi
            if [ "$TP_SERVER" == 0 ]; then
                EXIT_STATUS=0
                MESSAGE="Local TP Server up and running."
            else
                curl -s localhost/trainingplanner/ | grep -q "Bad Gateway" && TP_SERVER=1 || TP_SERVER=0
                if [ "$TP_SERVER" == 0 ]; then
                    curl -s localhost/trainingplanner/ | grep -q "Welcome" && TP_SERVER=0 || TP_SERVER=1
                fi
                if [ "$TP_SERVER" == 0 ]; then
                    EXIT_STATUS=5
                    MESSAGE="Local TP Server up and running on localhost only."
                else
                    EXIT_STATUS=1
                    MESSAGE="Local TP Server is not reachable."

                    if [ $NGINX_SETUP == 0 ]; then
                        MESSAGE="$MESSAGE NGINX service is up and running."
                    else
                        EXIT_STATUS=2
                        MESSAGE="$MESSAGE NGINX service is not running."
                    fi
                    if [ $PHP_FPM_SETUP == 0 ]; then
                        MESSAGE="$MESSAGE PHP-FPM service is up and running."
                    else
                        EXIT_STATUS=2
                        MESSAGE="$MESSAGE PHP-FPM service is not running."
                    fi
                fi
            fi
        else
            EXIT_STATUS=3
            MESSAGE="Local TP Server needs to be setup. Run" $SCRIPT_NAME "with the setup option."
        fi
        echo "$MESSAGE"
        exit $EXIT_STATUS
    ;;
    setup)
        setup_tpserver
        exit $?
    ;;
    start)
        if run_as_sudo systemctl start nginx; then
            run_as_sudo mkdir /run/php #an error in the fpm service might not create this needed directory
            echo "Local network server nginx started successfully"
            if run_as_sudo systemctl start $PHP_FPM_SERVICE; then
                echo "PHP FPM service started successfully";
                exit 0
            else
                echo "PHP FPM service failed to start";
                exit 1
            fi
        else
            echo "Local network server nginx failed to start"
            exit 2
        fi

    ;;
    stop)
        if run_as_sudo systemctl stop nginx; then
            echo "Local network server nginx stopped successfully"
            if run_as_sudo systemctl stop $PHP_FPM_SERVICE; then
                echo "PHP FPM service stopped successfully";
                exit 0
            else
                echo "PHP FPM service failed to stop";
                exit 1
            fi
        else
            echo "Local network server nginx failed to stop"
            exit 2
        fi
    ;;
    restart)
        if run_as_sudo systemctl restart nginx; then
            echo "Local network server nginx restarted successfully"
            if run_as_sudo systemctl restart $PHP_FPM_SERVICE; then
                echo "PHP FPM service restarted successfully";
                exit 0
            else
                echo "PHP FPM service failed to restart";
                exit 1
            fi
        else
            echo "Local network server nginx failed to restart"
            exit 2
        fi
    ;;
    dbcreate)
        if create_admin_user; then
            create_users_db
        fi
        exit $?
    ;;
    *)
        echo "Unknown option"
        print_usage
        exit 0
    ;;
esac
