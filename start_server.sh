#!/bin/bash

# Start the first process
service mysql start
  
# Start the second process
/root/socialnetotus/build-dir/socialnetotus --port $1 --address $2
  
# Wait for any process to exit
wait -n
  
# Exit with status of process that exited first
exit $?
