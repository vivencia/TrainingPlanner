#!/usr/bin/env bash

#go to
#https://www.shellcheck.net/
#to spell check shell scripts and make them more robust

VERSION="20260910-A"
SERVER_PORT="8443"
SCRIPT_NAME=$(basename "$0")
BASE_SERVER_DIR="/var/www/html"
TP_DIR=$BASE_SERVER_DIR"/trainingplanner"
SCRIPTS_DIR="$TP_DIR/scripts"
TPSERVER="TPServer"
EXIT_STATUS="0"

source "$SCRIPTS_DIR/function_library.sh"
ENABLE_DEBUG=0

CODES_FILE="$SCRIPTS_DIR/return_codes.h"

if [ $ENABLE_DEBUG -eq 1 ]; then
	NOW="$(date +%F-%H:%M)"
	LOG_FILE="/DATA/trainingplanner-log/tp-$NOW.txt"
	touch "$LOG_FILE"
fi

print_usage() {
	print "Usage: $SCRIPT_NAME <COMMAND> <OPTION>
	Commands:
		-s,--setup
		-q,--status
		-i,--start
		-t,--stop
		-r,--restart
		-p,--pause
		-c, --createdb
		-h,--help
		-v,--version

	Options:
		$$TPSERVER,$NGINX,$PHP_FPM"
	exit 0
}

print_version() {
	case $1 in
		"$NGINX") $NGINX_BIN -V;;
		"$PHP_FPM") $PHP_FPM_BIN -v;;
		*) echo "TrainingPlanner App Server Management Script version $VERSION";;
	esac
	exit 0
}

if [ "$1" != "" ]; then
	for i in "$@"; do
		case $i in
			-p=*|--password=*)
				PASSWORD="${i#*=}"
				shift # past argument=value
			;;
			-h|--help)
				print_usage
			;;
			--*|-*)
				COMMAND="${i}"
			;;
			*)
				OPTION="${i}"
			;;
		esac
	done
else
	print_usage
fi

SOURCES_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd ) #the directory of this script
NGINX_CONFIG_DIR="/etc/nginx"
NGINX_SERVER_CONFIG_DIR="/etc/nginx/conf.d"
NGINX_USER="www-data"
PHP_FPM_CONFIG_DIR="/etc/php/php-fpm.d"
ADMIN="admin"
ADMIN_DIR="$TP_DIR/$ADMIN"
USERS_DB="$TP_DIR/$ADMIN/users.db"
PAUSE_FILE="$TP_DIR/pause"

create_admin_user() {
	PASS_FILE=$ADMIN_DIR/.passwds
	HTPASSWD=$(which htpasswd)
	if "$HTPASSWD" -bv $PASS_FILE $ADMIN $ADMIN; then
		print "Creating the main app user"
		#always use the -B option for htpasswd to create bcrypt hashes compatible with PHP's password_verify()
		if run_as_sudo "$HTPASSWD" -cbB $PASS_FILE $ADMIN $ADMIN; then
			run_as_sudo mkdir -m 774 $ADMIN_DIR
			run_as_sudo cp -f "$SOURCES_DIR/user.fields" $ADMIN_DIR
			run_as_sudo chown -R $NGINX_USER:$NGINX_USER $ADMIN_DIR
			print "Administrator user \"admin\" created successfully"
			echo "0"
		fi
	fi
	echo "1"
}

create_users_db() {
	run_as_sudo rm -f "${USERS_DB}"
	if sqlite3 -line ${USERS_DB} 'CREATE TABLE IF NOT EXISTS users_table (userid INTEGER PRIMARY KEY, inserttime INTEGER, onlineaccount INTEGER, name TEXT, birthday INTEGER, sex INTEGER, phone TEXT, email TEXT, social TEXT, role TEXT, coach_role TEXT, goal TEXT, use_mode INTEGER, password TEXT);' &>/dev/null; then
		run_as_sudo chown -R $NGINX_USER:$NGINX_USER $USERS_DB
		run_as_sudo chmod 664 $USERS_DB
		print "Users database created"
		echo "0"
	else
		print "Failed to create users database: $USERS_DB"
		echo "1"
	fi
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
	return "$RESULT"
}

test_tp_server() {
	query_address "$1"
	case "$?" in
		0)
			if [ "$2" == "lan" ]; then
				MESSAGE="$MESSAGE\n$TPSERVER up and running($1)."
			else
				MESSAGE="$MESSAGE\n$TPSERVER is running on localhost only($1)."
			fi
		;;
		1)
			MESSAGE="$MESSAGE\n$TPSERVER is not reachable."
			EXIT_STATUS=$(get_return_code "tpserver not reachable")
		;;
		2)
			if [ "$2" == "lan" ]; then
				MESSAGE="$MESSAGE\n$TPSERVER paused."
			else
				MESSAGE="$MESSAGE\n$TPSERVER paused on localhost only."
			fi
			EXIT_STATUS=$(get_return_code "tpserver paused")
		;;
	esac
}

start_server() {
	OK=0
	if [[ "$1" == "$NGINX" || "$1" != "$PHP_FPM" ]]; then
		start_nginx
		if [ $? != 0 ]; then
			OK=$?
			if [ $OK == 1 ]; then
				if [ "$1" != "$NGINX" ]; then
					MESSAGE="$TPSERVER not starting because $NGINX is not installed"
				fi
			else
				if [ "$1" != "$NGINX" ]; then
					MESSAGE="$TPSERVER not starting because $NGINX failed to start"
				fi
			fi
		fi
	fi
	if [[ $OK == 0 && "$1" == "$PHP_FPM" || "$1" != "$NGINX" ]]; then
		start_phpfpm
		if [ $? == 0 ]; then
			MESSAGE="Checking $TPSERVER..."
			if find_local_ip; then
				test_tp_server "$SERVER_IP:$SERVER_PORT" "lan"
			else
				test_tp_server "localhost:$SERVER_PORT" "lan"
			fi
		else
			if [ $? == 1 ]; then
				if [ "$1" != "$PHP_FPM" ]; then
					MESSAGE="$TPSERVER not starting because $PHP_FPM is not installed"
				fi
			else
				if [ "$1" != "$PHP_FPM" ]; then
					MESSAGE="$TPSERVER not starting because $PHP_FPM failed to start"
				fi
			fi
		fi
	fi
	if [ "$MESSAGE" != "" ]; then
		print "$MESSAGE"
	fi
}

stop_server() {
	case "$1" in
		"$NGINX")
			EXIT_STATUS="101"
			print "TP software depends on $NGINX but will not stop the service because other applications might be using it"
			;;
		*)
			stop_phpfpm
			case $? in
				0)
					EXIT_STATUS="0"
					print "$TPSERVER stopped!"
					;;
				1)
					if [ "$1" != "$PHP_FPM" ]; then
						MESSAGE="$TPSERVER not stopping because $PHP_FPM is not installed"
					fi
					;;
				2)
					if [ "$1" != "$PHP_FPM" ]; then
						print "$TPSERVER is likely still running because $PHP_FPM did not stop"
					fi
					;;
			esac
		;;
	esac
}

pause_server() {
	echo "1" > $PAUSE_FILE
	print "$TPSERVER paused"
}

unpause_server() {
	echo "0" > $PAUSE_FILE
	print "$TPSERVER running"
}

setup_tpserver() {
	print "Beginning TP Server configuration..."
	if [ ! -d "$TP_DIR" ]; then
		print "Preparing the firesystem layout and copying configuration files to their respective locations..."

		run_as_sudo groupadd $NGINX_USER
		run_as_sudo useradd -r $NGINX_USER -g $NGINX_USER -G network,sys

		run_as_sudo mkdir -p $SCRIPTS_DIR
		run_as_sudo chmod -R 770 $TP_DIR
		run_as_sudo chmod -R 770 $SCRIPTS_DIR
		run_as_sudo cp "$SOURCES_DIR/$SCRIPT_NAME" $SCRIPTS_DIR #copy this file to the scripts dir
		run_as_sudo cp "$SOURCES_DIR/function_library.sh" $SCRIPTS_DIR
		run_as_sudo cp "$SOURCES_DIR/run_cmds.sh" $SCRIPTS_DIR
		run_as_sudo cp "$SOURCES_DIR/usersdb.sh" $SCRIPTS_DIR
		run_as_sudo cp "$SOURCES_DIR/url_parser.php" $SCRIPTS_DIR
		run_as_sudo cp "$SOURCES_DIR/tp_functions.php" $SCRIPTS_DIR
		run_as_sudo cp "$SOURCES_DIR/return_codes.h" $SCRIPTS_DIR
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
			print "Filesystem directories and files setup."
		else
			print "Filesystem directories and files setup. Log out and in again in order for the changes to work."
			echo "0"
		fi
	else
		start_server "$TPSERVER"
	fi
}

get_tpserver_status() {
	case $1 in
		"$NGINX"|"$PHP_FPM")
			systemctl status "$1"
		;;
		*)
			if [ -d "$TP_DIR" ]; then
				if pgrep --quiet "$NGINX"; then
					MESSAGE="$NGINX is running."
				else
					EXIT_STATUS=$(get_return_code "nginx not running")
					MESSAGE="$NGINX is not running."
				fi
				if pgrep --quiet "$PHP_FPM"; then
					MESSAGE="$MESSAGE $PHP_FPM is running."
					if [ $EXIT_STATUS != "0" ]; then
						MESSAGE="$MESSAGE\n$TPSERVER not running because $NGINX is not running."
					fi
				else
					MESSAGE="$MESSAGE $PHP_FPM is not running."
					if [ $EXIT_STATUS != "0" ]; then
						MESSAGE="$MESSAGE\n$TPSERVER not running because $NGINX and $PHP_FPM are not running."
					else
						MESSAGE="$MESSAGE\n$TPSERVER not running because $PHP_FPM is not running."
					fi
					EXIT_STATUS=$(get_return_code "phpfpm not running")

				fi

				if [[ $EXIT_STATUS == "0" ]]; then
					if find_local_ip; then
						test_tp_server "$SERVER_IP:$SERVER_PORT" "lan"
					else
						test_tp_server "localhost:$SERVER_PORT" "lo"
					fi
				fi
			else
				EXIT_STATUS=$(get_return_code "tpserver not configured")
				MESSAGE="$TPSERVER needs to be setup. Run" $SCRIPT_NAME "with the --setup command."
			fi
		;;
	esac
}

case "$COMMAND" in
	-q|--status) #do not log status querying
		if [ ! -f "$NGINX_BIN" ]; then
			EXIT_STATUS=$(get_return_code "nginx not installed")
			MESSAGE="Please install $NGINX."
		else
			if [ ! -f "$PHP_FPM_BIN" ]; then
				EXIT_STATUS=$(get_return_code "phpfpm not installed")
				MESSAGE="Please install $PHP_FPM."
			else
				get_tpserver_status "$OPTION"
			fi
		fi
		if [ "$MESSAGE" != "" ]; then
			echo -e "$MESSAGE"
		fi
	;;
	-s|--setup)
		setup_tpserver
	;;
	-i|--start)
		start_server "$OPTION"
	;;
	-t|--stop)
		stop_server "$OPTION"
	;;
	-r|--restart)
		stop_server "$OPTION"
		if [ $EXIT_STATUS == 0 ]; then
			start_server "$OPTION"
			EXIT_STATUS=$?
		fi
	;;
	-p|--pause)
		get_tpserver_status "$TPSERVER"
		if [[ "$EXIT_STATUS" == "0" ]]; then
			if [ ! -f "$PAUSE_FILE" ]; then
				pause_server
			else
				PAUSE=$(head -n 1 "$PAUSE_FILE")
				if [ "$PAUSE" == 1 ]; then
					unpause_server
				else
					pause_server
				fi
			fi
		else
			print "Cannot pause $TPSERVER because it's not running."
		fi
	;;
	-c|--createdb)
		EXIT_STATUS=$(create_admin_user)
		if [ $EXIT_STATUS == 0 ]; then
			EXIT_STATUS=$(create_users_db)
		fi
	;;
	-v|--version)
		print_version "$OPTION"
	;;
	*)
		print "Unknown command($COMMAND)"
		print_usage
	;;
esac

exit $EXIT_STATUS
