#!/bin/bash

BASE_SERVER_DIR=/var/www/html
TP_DIR=$BASE_SERVER_DIR/trainingplanner
PHP_FPM_SERVICE=php8.2-fpm
SCRIPT_NAME=$(basename "$0")
USER_NAME=$(whoami)
PASSWORD=""

run_as_sudo() {
    echo $PASSWORD | sudo -S "$@"
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

for i in "$@"; do
    case $i in
        -p=*|--password=*)
            PASSWORD="${i#*=}"
            shift # past argument=value
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

case "$COMMAND" in
    test)
        if [ -d "$TP_DIR" ]; then
            if /sbin/service nginx status 1&> /dev/null; then
                if /sbin/service $PHP_FPM_SERVICE status 1&> /dev/null; then
                    echo "Local TP Server up and running."
                    exit 0
                else
                    echo "PHP-FPM service is not running("$?")"
                fi
            else
                echo "nginx service is not running("$?")"
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
        get_passwd
        run_as_sudo /sbin/service nginx stop
        run_as_sudo /sbin/service $PHP_FPM_SERVICE stop
        exit 0
    ;;
    restart)
        get_passwd
        run_as_sudo /sbin/service nginx restart
        run_as_sudo /sbin/service $PHP_FPM_SERVICE restart
        exit 0
    ;;
    *)
	echo "Usage: $SCRIPT_NAME {setup|test|stop|restart}" >&2
	exit 1
    ;;
esac

get_passwd

SOURCES_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd ) #the directory of this script
SCRIPTS_DIR=$TP_DIR/scripts

NGINX="$(which nginx)"
NGINX_CONFIG_DIR=/etc/nginx
NGINX_SERVER_CONFIG_DIR=/etc/nginx/sites-available
NGINX_USER=www-data

PHP_FPM_CONFIG_DIR=/etc/php/8.2/fpm/pool.d

USER_BELONGS_TO_GROUP=/usr/bin/id -nG "$USER_NAME" | grep -qw "$NGINX_USER"

if [ ! -d "$TP_DIR" ]; then

    echo "Preparing the firesystem layout and copying configuration files to their respective locations..."

    run_as_sudo mkdir -p $SCRIPTS_DIR
    run_as_sudo chmod -R 770 $TP_DIR
    run_as_sudo chmod -R 770 $SCRIPTS_DIR
    run_as_sudo cp $SOURCES_DIR/$SCRIPT_NAME $SCRIPTS_DIR #copy this file to the scripts dir
    run_as_sudo cp $SOURCES_DIR/uri_parser.php $SCRIPTS_DIR
    run_as_sudo chown -R $NGINX_USER:$NGINX_USER $TP_DIR

    run_as_sudo cp -f $SOURCES_DIR/nginx.conf $NGINX_CONFIG_DIR
    run_as_sudo chown root:root $NGINX_CONFIG_DIR/nginx.conf
    run_as_sudo chmod g+w $NGINX_CONFIG_DIR/nginx.conf

    run_as_sudo cp -f $SOURCES_DIR/default $NGINX_SERVER_CONFIG_DIR
    run_as_sudo chown root:root $NGINX_SERVER_CONFIG_DIR/default
    run_as_sudo chmod g+w $NGINX_SERVER_CONFIG_DIR/default

    run_as_sudo cp -f $SOURCES_DIR/www.conf $PHP_FPM_CONFIG_DIR

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
                echo "Error starting nginx service"
                exit 4
            fi
        else
            echo "Please install nginx"
            exit 4
        fi
    fi

    if pgrep -fl php-fpm &>/dev/null; then
        echo "Service php-fpm is already running."
    else
        echo "Starting php-fpm..."
        $(run_as_sudo /sbin/service $PHP_FPM_SERVICE start)
        if [ $? != 0 ]; then
            echo "Error starting php-fpm service"
            exit 5
        fi
    fi

    echo "Local TP server configured and running!"
    exit 0
fi
