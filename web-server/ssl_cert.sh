#!/bin/bash

source "function_library.sh"
find_local_ip

CERTIFICATE_FILE="../security/nginx2.crt"
KEY_FILE="../security/nginx2.key"
RET_CODE=0

make_cert() {
	run_as_sudo echo "Generating certificate..."
	sudo openssl req -x509 -nodes -days 365 -newkey rsa:2048 \
				-keyout $KEY_FILE -out $CERTIFICATE_FILE -addext "subjectAltName = IP:$SERVER_IP,DNS:localhost"
	RET_CODE=$?
}

if make_cert; then
	sudo cp $KEY_FILE /DATA/software/
	sudo cp $CERTIFICATE_FILE /DATA/software/
	sudo chmod 644 $KEY_FILE $CERTIFICATE_FILE
	sudo chown $USER_NAME:$USER_NAME $KEY_FILE $CERTIFICATE_FILE
	echo "Certificates generated successfully!"
	exit 0
else
	echo "Error generating certificate ($RET_CODE)"
	exit 1
fi
