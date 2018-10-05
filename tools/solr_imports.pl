#!/usr/bin/perl
# Perform SOLR full imports forever. No logging (check /opt/solr/logs).
# Full imports occur at midnight each night. 

$datecmd = "date +%d";
$last  =  `$datecmd`;
while (1) {
        $current = `$datecmd`;
        if ($current != $last) {
                # Full import.
                system("/usr/bin/curl 'http://localhost:8983/solr/lpf/dataimport?command=full-import&optimize=true' > /dev/null 2>&1");
                $last = $current;
        } else {
            sleep 5;
        }
}
