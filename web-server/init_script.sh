#!/bin/bash

read -p "Sudo's password: " -s
echo
echo $REPLY | sudo -S 1&> /dev/null

run_as_sudo() {
    echo $REPLY | sudo -S $@
}

if [ $(run_as_sudo whoami) != "root" ]; then
    echo "Wrong sudo password. Exiting..."
    exit
fi

SOURCES_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd ) #the directory of this script
BASE_SERVER_DIR=/var/www/html
TP_DIR=$BASE_SERVER_DIR/trainingplanner
SCRIPTS_DIR=$TP_DIR/scripts

NGINX="$(which nginx)"
NGINX_CONFIG_DIR=/etc/nginx
NGINX_SERVER_CONFIG_DIR=/etc/nginx/sites-available
NGINX_USER=www-data

PHP_FPM="$(which php-fpm)"
PHP_FPM_CONFIG_DIR=/etc/php/8.2/fpm/pool.d
PHP_FPM_SERVICE=php8.2-fpm

if [ ! -d "$TP_DIR" ]; then

    echo "Preparing the firesystem layout and copying configuration files to their respective locations..."

    run_as_sudo mkdir -p $SCRIPTS_DIR
    run_as_sudo chmod -R 770 $TP_DIR
    run_as_sudo chmod -R 770 $SCRIPTS_DIR
    run_as_sudo cp $SOURCES_DIR/$(basename "$0") $SCRIPTS_DIR #copy this file to the scripts dir
    run_as_sudo cp $SOURCES_DIR/uri_parser.php $SCRIPTS_DIR
    run_as_sudo chown -R $NGINX_USER:$NGINX_USER $TP_DIR
    run_as_sudo usermod -a -G $NGINX_USER $(whoami)

    run_as_sudo cp -f $SOURCES_DIR/nginx.conf $NGINX_CONFIG_DIR
    run_as_sudo chown root:root $NGINX_CONFIG_DIR/nginx.conf
    run_as_sudo chmod g+w $NGINX_CONFIG_DIR/nginx.conf

    run_as_sudo cp -f $SOURCES_DIR/default $NGINX_SERVER_CONFIG_DIR
    run_as_sudo chown root:root $NGINX_SERVER_CONFIG_DIR/default
    run_as_sudo chmod g+w $NGINX_SERVER_CONFIG_DIR/default

    run_as_sudo cp -f $SOURCES_DIR/www.conf $PHP_FPM_CONFIG_DIR

    echo "Filesystem directories and files setup. Log out and in again in order for the changes to work"
    exit

else

    if pgrep -fl nginx &>/dev/null; then
        echo "Service nginx is already running."
    else
        echo "Starting http server..."
        if [ -x "$NGINX" ]; then
            run_as_sudo service nginx start
            if [ $? != 0 ]; then
                exit
            fi
        else
            echo "Please install nginx"
            exit
        fi
    fi

    if pgrep -fl php-fpm &>/dev/null; then
        echo "Service php-fpm is already running."
    else
        echo "Starting php-fpm..."
        if [ -x "$PHP_FPM" ]; then
            run_as_sudo service $PHP_FPM_SERVICE start
            if [ $? != 0 ]; then
                exit
            fi
        else
            echo "Please install php-fpm"
            exit
        fi
    fi

    echo "Local server configured!"

fi
