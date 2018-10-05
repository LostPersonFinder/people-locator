#!/bin/bash

mailto=$1

# The standard e-mailed report.
./generateEventsReport.php > /opt/pl/www/tmp/ga_cache/events.html
# The report generated for viewing in browser.
./generateEventsReport.php 1 > /opt/pl/www/tmp/ga_cache/events_for_browser.html
mutt -e 'set content_type=text/html' -s "PL Events Report for `date '+%m/%d/%y%n'`" $mailto < /opt/pl/www/tmp/ga_cache/events.html
