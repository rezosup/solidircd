#!/bin/sh

if test "$1" = "newcert"; then
    openssl req -new -x509 -nodes -days 365 -config ssl.cnf -out vgc.pem -keyout vgckey.pem
    echo ""
    chmod 600 vgckey.pem
    openssl x509 -subject -dates -issuer -serial -fingerprint -noout -in vgc.pem
    echo ""
    chmod 600 vgc.pem
else

echo ""
echo "Syntax: ./makecert.sh newcert"
echo "Warning! This will replace your current certificate, if any"
echo ""


fi
