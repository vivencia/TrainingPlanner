#!/bin/bash

export PASSWORD
export CODES_FILE

USER_NAME=$(whoami)
export USER_NAME
NGINX="nginx"
export NGINX
NGINX_BIN="$(which $NGINX)"
export NGINX_BIN
PHP_FPM="php-fpm"
export PHP_FPM
PHP_FPM_BIN="$(which $PHP_FPM)"
export PHP_FPM_BIN
BASE_SERVER_DIR="/var/www/html"
export BASE_SERVER_DIR
ENABLE_DEBUG=0
export ENABLE_DEBUG
LOG_FILE=""
export LOG_FILE

print() {
	if [ $ENABLE_DEBUG -eq 1 ]; then
		echo -e "${@}" | tee -a "$LOG_FILE"
	else
		echo -e "${@}"
	fi
}

get_return_code() {
	SEARCH_STRING="${1^^}"
	SEARCH_STRING="${SEARCH_STRING// /_}"
	RET_CODE="101" #Unknown error code
	while IFS= read -r line || [[ -n "$line" ]]; do
		if [[ "${line}" == *"$SEARCH_STRING"* ]]; then
			read -ra words <<< "${line}"
			#${words[@]} all the words, i, e. #define RET_CODE value
			RET_CODE="${words[2]}"
			break
		fi
	done < "$CODES_FILE"
	echo "$RET_CODE"
}

get_passwd() {
	if [ ! "$PASSWORD" ]; then
		read -p "Sudo's password: " -sr PASSWORD
		if ! echo "$PASSWORD" | sudo -S whoami | grep -q "root"; then
			print "Wrong sudo password. Exiting..."
			return 1
		fi
		echo
	else
		if ! echo "$PASSWORD" | sudo -S whoami | grep -q "root"; then
			print "Wrong sudo password. Exiting..."
			return 1
		fi
	fi
	return 0
}

run_as_sudo() {
	if get_passwd; then
		echo "$PASSWORD" | sudo -S "$@"
		return 0
	fi
	exit 3g
}

run_as_sudo_detached() {
	if get_passwd; then
		bash echo "$PASSWORD" | sudo -S "$@" > /dev/null 2>&1 &
		return 0
	fi
	exit 3
}

find_local_ip() {
	SERVER_IP=""
	INTERFACE_DATA=$(ip addr show | grep 'inet.*wlan0')
	N_WORDS=$(echo "${INTERFACE_DATA}" | awk -F ' ' '{ print NF }')
	(( N_WORDS++ ))
	(( z=1 ))
	while [ $z -ne "$N_WORDS" ]; do
		WORD=$(echo "${INTERFACE_DATA}" | cut -d ' ' -s -f $z)
		if [ "$WORD" ]; then
			SERVER_IP=$(echo "${WORD}" | cut -d '/' -s -f 1)
			if [ "$SERVER_IP" ]; then
				return 0
			fi
		else
			(( N_WORDS++ ))
		fi
		(( z++))
	done
	return 1
}

start_nginx() {
	if [ -f "$NGINX_BIN" ]; then
		if pgrep --quiet "$NGINX"; then
			print "$NGINX is already running."
		else
			print "Starting $NGINX..."
			if ! run_as_sudo systemctl start "$NGINX"; then
				print "Error starting $NGINX."
				EXIT_STATUS=$(get_return_code "nginx error")
				return 2
			else
				print "$NGINX started successfully."
			fi
		fi
	else
		print "Please install $NGINX."
		EXIT_STATUS=$(get_return_code "nginx not installed")
		return 1
	fi
	EXIT_STATUS="0"
	return 0
}

stop_nginx() {
	if [ -f "$NGINX_BIN" ]; then
		if pgrep --quiet "$NGINX"; then
			print "Stopping $NGINX..."
			if ! run_as_sudo systemctl stop "$NGINX"; then
				print "Error starting $NGINX."
				EXIT_STATUS=$(get_return_code "nginx error")
				return 2
			else
				print "$NGINX started successfully."
			fi
		else
			print "$NGINX is not running."
		fi
	else
		print "Please install $NGINX."
		EXIT_STATUS=$(get_return_code "nginx not installed")
		return 2
	fi
	EXIT_STATUS="0"
	return 0
}

start_phpfpm() {
	if [ -f "$PHP_FPM_BIN" ]; then
		if pgrep --quiet "$PHP_FPM"; then
			print "$PHP_FPM is already running."
		else
			print "Starting $PHP_FPM ..."
			if [ ! -f "/run/php" ]; then
				run_as_sudo mkdir "/run/php"
			fi
			if ! run_as_sudo systemctl start "$PHP_FPM"; then
				print "Error starting $PHP_FPM."
				EXIT_STATUS=$(get_return_code "phpfpm error")
				return 2
			else
				print "$PHP_FPM started successfully."
			fi
		fi
	else
		print "Please install $PHP_FPM."
		EXIT_STATUS=$(get_return_code "phpfpm not installed")
		return 1
	fi
	EXIT_STATUS="0"
	return 0
}

stop_phpfpm() {
	if [ -f "$PHP_FPM_BIN" ]; then
		if pgrep --quiet "$PHP_FPM"; then
			print "Stopping $PHP_FPM..."
			if ! run_as_sudo systemctl stop "$PHP_FPM"; then
				print "$PHP_FPM failed to stop"
				EXIT_STATUS=$(get_return_code "phpfpm error")
				return 2
			else
				print "$PHP_FPM stopped successfully."
			fi
		else
			print "$PHP_FPM is not running."
		fi
	else
		print "Please install $PHP_FPM."
		EXIT_STATUS=$(get_return_code "phpfpm not installed")
		return 1
	fi
	EXIT_STATUS="0"
	return 0
}
