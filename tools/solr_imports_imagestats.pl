#!/usr/bin/perl
# Perform SOLR imagestats imports forever. No logging (check /opt/solr/logs).
# Do delta imports every hour and full imports every midnight.

$datecmd = "date +%d";
$last  =  `$datecmd`;
while (1) {
        $current = `$datecmd`;
        if ($current != $last) {
                # Full import.
                system("/usr/bin/curl 'http://localhost:8983/solr/imagestats/dataimport?command=full-import&optimize=true' > /dev/null 2>&1");
                $last = $current;
        } else {
         	# Incremental import.
         	system("/usr/bin/curl 'http://localhost:8983/solr/imagestats/dataimport?command=delta-import&optimize=true' > /dev/null 2>&1");
        }
        sleep 3600;
}
