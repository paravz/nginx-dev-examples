#!/bin/bash
echo -n '/index.htmlfoo'|openssl md5 -binary|openssl base64|tr +/ -_|tr -d =
