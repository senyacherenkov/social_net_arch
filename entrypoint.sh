#!/bin/sh

service mysql start
exec /root/socialnetotus/build-dir/socialnetotus --port $PORT --address $ADDRESS
