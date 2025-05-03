#!/bin/bash

BASE_SERVER_DIR=/var/www/html
TP_DIR=$BASE_SERVER_DIR/trainingplanner
PHP_FPM_SERVICE=php8.2-fpm
SCRIPT_NAME=$(basename "$0")
USER_NAME=$(whoami)
PASSWORD=""

print_usage() {
	echo "Usage: $SCRIPT_NAME {setup|test|stop|restart|dbcreate}" >&2
}

get_passwd() {
    if [ ! $PASSWORD ]; then
        read -p "Sudo's password: " -s
        PASSWORD=$REPLY
        echo $PASSWORD | sudo -S whoami
        if [ $? != 0 ]; then
            echo "Wrong sudo password. Exiting..."
            exit 3
        fi
        echo
    else
        echo $PASSWORD | sudo -S whoami
        if [ $? != 0 ]; then
            echo "Wrong sudo password. Exiting..."
            exit 3
        fi
    fi
}

run_as_sudo() {
    get_passwd
    echo $PASSWORD | sudo -S "$@"
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
        -*|--*)
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
NGINX_CONFIG_DIR=/etc/nginx
NGINX_SERVER_CONFIG_DIR=/etc/nginx/sites-available
NGINX_USER=www-data

PHP_FPM_CONFIG_DIR=/etc/php/8.2/fpm/pool.d

/usr/bin/id -nG $USER_NAME | grep -qw $NGINX_USER
USER_BELONGS_TO_GROUP=$?

ADMIN=admin
ADMIN_DIR=$TP_DIR/$ADMIN
USERS_DB=$TP_DIR/$ADMIN/users.db

create_admin_user() {
    PASS_FILE=$SCRIPTS_DIR/.passwds
    HTPASSWD=$(which htpasswd)
    $HTPASSWD -bv $PASS_FILE $ADMIN $ADMIN
    if [ $? != 0 ]; then
        echo "Creating the main app user"
        run_as_sudo $HTPASSWD -bd5 $PASS_FILE $ADMIN $ADMIN
        if [ $? == 0 ]; then
            run_as_sudo mkdir -m 774 $ADMIN_DIR
            run_as_sudo chown $NGINX_USER:$NGINX_USER $ADMIN_DIR
            echo "Main app user created successfully"
        fi
    fi
}

create_users_db() {
    run_as_sudo rm -f "${USERS_DB}"
    if sqlite3 -line ${USERS_DB} 'CREATE TABLE IF NOT EXISTS users_table (id INTEGER PRIMARY KEY, name TEXT, birthday INTEGER, sex TEXT, phone TEXT, email TEXT, social TEXT, role TEXT, coach_role TEXT, goal TEXT,  use_mode INTEGER DEFAULT 1, password TEXT);' &>/dev/null; then
        run_as_sudo chown -R $NGINX_USER:$NGINX_USER $USERS_DB
        run_as_sudo chmod 664 $USERS_DB
        echo "Users database created"
        return 0
    else
        echo "Failed to create users database: " $USERS_DB
        return 1
    fi
}

case "$COMMAND" in
    test)
        if [ -d "$TP_DIR" ]; then
            if /sbin/service nginx status 1&> /dev/null; then
                if /sbin/service $PHP_FPM_SERVICE status 1&> /dev/null; then
                    echo "Local TP Server up and running."
                    exit 0
                else
                    echo "PHP-FPM service is not running("$?")."
                fi
            else
                echo "nginx service is not running("$?")."
            fi
        else
            echo "Local TP Server needs to be setup. Run" $SCRIPT_NAME "with the setup option."
        fi
        exit 2
    ;;
    setup)
        echo "Beginning TP Server configuration..."
    ;;
    stop)
        run_as_sudo /sbin/service nginx stop
        run_as_sudo /sbin/service $PHP_FPM_SERVICE stop
        exit 0
    ;;
    restart)
        run_as_sudo /sbin/service nginx restart
        run_as_sudo /sbin/service $PHP_FPM_SERVICE restart
        exit 0
    ;;
    dbcreate)
        create_users_db
        exit $?
    ;;
    *)
        print_usage
        exit 0
    ;;
esac

if [ ! -d "$TP_DIR" ]; then

    echo "Preparing the firesystem layout and copying configuration files to their respective locations..."

    run_as_sudo mkdir -p $SCRIPTS_DIR
    run_as_sudo chmod -R 770 $TP_DIR
    run_as_sudo chmod -R 770 $SCRIPTS_DIR
    run_as_sudo cp $SOURCES_DIR/$SCRIPT_NAME $SCRIPTS_DIR #copy this file to the scripts dir
    run_as_sudo cp $SOURCES_DIR/url_parser.php $SCRIPTS_DIR
    run_as_sudo chown -R $NGINX_USER:$NGINX_USER $TP_DIR

    run_as_sudo cp -f $SOURCES_DIR/nginx.conf $NGINX_CONFIG_DIR
    run_as_sudo chown root:root $NGINX_CONFIG_DIR/nginx.conf
    run_as_sudo chmod g+w $NGINX_CONFIG_DIR/nginx.conf

    run_as_sudo cp -f $SOURCES_DIR/default $NGINX_SERVER_CONFIG_DIR
    run_as_sudo chown root:root $NGINX_SERVER_CONFIG_DIR/default
    run_as_sudo chmod g+w $NGINX_SERVER_CONFIG_DIR/default

    run_as_sudo cp -f $SOURCES_DIR/www.conf $PHP_FPM_CONFIG_DIR

    create_admin_user
    create_users_db

    if ! $USER_BELONGS_TO_GROUP; then
        run_as_sudo usermod -a -G $NGINX_USER $(whoami)
        echo "Filesystem directories and files setup."
    else
        echo "Filesystem directories and files setup. Log out and in again in order for the changes to work."
        exit 0
    fi

else

    if pgrep -fl nginx &>/dev/null; then
        echo "Service nginx is already running."
    else
        echo "Starting http server..."
        if [ -f $NGINX ]; then
            run_as_sudo /sbin/service nginx start
            if [ $? != 0 ]; then
                echo "Error starting nginx service."
                exit 4
            fi
        else
            echo "Please install nginx."
            exit 4
        fi
    fi

    if pgrep -fl php-fpm &>/dev/null; then
        echo "Service php-fpm is already running."
    else
        echo "Starting php-fpm..."
        run_as_sudo /sbin/service $PHP_FPM_SERVICE start
        if [ $? != 0 ]; then
            echo "Error starting php-fpm service."
            exit 5
        fi
    fi

    echo "Local TP server configured and running!"
    exit 0
fi
