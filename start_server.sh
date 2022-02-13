#!/bin/bash

# Start the first process
mkfifo /var/run/mysqld/mysqld.sock
chown -R mysql /var/run/mysqld
service mysql restart
mysqladmin --user=root password "1"
mysqlcheck --check-upgrade --all-databases --auto-repair -u root -p1
mysql_upgrade --force -u root -p1
service mysql restart
mysql -p1 < poco_server.sql

# Start the second process
cd /root/socialnetotus/build-dir/
./socialnetotus --port $1 --address $2
  
# Wait for any process to exit
wait -n
  
# Exit with status of process that exited first
exit $?
