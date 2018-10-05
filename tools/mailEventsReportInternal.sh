#!/bin/bash

mailto=$1

# The standard e-mailed internal report.
./generateEventsReport.php 2 > /opt/pl/www/tmp/ga_cache/events_internal.html
# The internal report formatted for viewing in a browser.
./generateEventsReport.php 3 > /opt/pl/www/tmp/ga_cache/events_for_browser_internal.html
mutt -e 'set content_type=text/html' -s "PL Events Report for `date '+%m/%d/%y%n'`" $mailto < /opt/pl/www/tmp/ga_cache/events_internal.html
