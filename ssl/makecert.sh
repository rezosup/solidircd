#!/bin/sh

if test "$1" = "newcert"; then
    openssl req -new -x509 -nodes -days 365 -config ssl.cnf -out rz.pem -keyout rzkey.pem
    echo ""
    openssl x509 -subject -dates -issuer -serial -fingerprint -noout -in rz.pem
    echo ""
else

echo ""
echo "Syntax: ./makecert.sh newcert"
echo "Warning! This will replace your current certificate, if any"
echo ""


fi
