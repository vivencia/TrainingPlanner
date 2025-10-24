#!/bin/bash

NGINX="$(which nginx)"

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

start_nginx() {
    if [ -f "$NGINX" ]; then
        if pgrep -fl "$NGINX" &>/dev/null; then
            echo "The NGINX service is already running."
        else
            echo "Starting NGINX..."
            if ! run_as_sudo systemctl start nginx; then
                echo "Error starting nginx service."
                return 4
            else
                echo "The NGINX service started successfully."
            fi
        fi
        return 0;
    else
        echo "Please install NGINX."
        return 4
    fi
    return 0
}
