#!/usr/bin/perl
# Perform SOLR TP imports forever. No logging (check /opt/solr/logs).
# Do delta imports every 20 seconds and full imports once a day.
# Full imports will occur at midnight each night. To change them to
# to occur at e.g. 2 AM, change $datecmd to:
#
#     date --date="two hours ago" +%d

$datecmd = "date +%d";
$last  =  `$datecmd`;
while (1) {
        $current = `$datecmd`;
        if ($current != $last) {
                # Full import.
                system("/usr/bin/curl 'http://localhost:8983/solr/tp/dataimport?command=full-import&optimize=true' > /dev/null 2>&1");
                $last = $current;
        } else {
         	# Incremental import.
         	system("/usr/bin/curl 'http://localhost:8983/solr/tp/dataimport?command=delta-import&optimize=true' > /dev/null 2>&1");
        }
        sleep 20;
}
