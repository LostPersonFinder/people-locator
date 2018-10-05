#!/bin/bash

mailto=$1

#/usr/bin/php generateAnalyticsReport.php | mailx -s "PL Website Traffic Report for `date '+%m/%d/%y%n'`" $mailto
./generateAnalyticsReport.php > /opt/pl/www/tmp/ga_cache/report.html
mutt -e 'set content_type=text/html' -s "PL Website Traffic Report for `date '+%m/%d/%y%n'`" $mailto < /opt/pl/www/tmp/ga_cache/report.html
