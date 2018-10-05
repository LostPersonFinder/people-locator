#!/bin/sh
# Monitor Google Person Finder Event Feed 

while :
do
        /usr/bin/python /opt/pl/tools/pfmon.py 2>> /opt/pl/www/tmp/pfmon_logs/error.log > /dev/null
        sleep 3600
done
