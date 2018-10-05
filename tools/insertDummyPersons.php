<?php

// Easily customizable script for creating dummy persons. This script
// extracts the person id's from a set of image files and creates
// a PFIF import file with just a person_id and an expiry_date.

error_reporting(E_ALL);
ini_set("display_errors", "stdout");
require_once("../conf/taupo.conf");
require_once("../inc/lib_includes.inc");
// Get the file list image files.
$directory = "../www/tmp/pfif_cache_orphans/";
//get all files in specified directory
$files = glob($directory . "*");
$person_ids = array();
foreach($files as $file) {
  $s = substr($file, 30);
  $s2 = substr($s, 0, strpos($s, '__'));
  $person_ids[] = str_replace('SLASH', '/', $s2);
}
$datetime =  date('Y-m-d H:i:s');
# Print XML header.
print "<?xml version=\"1.0\" encoding=\"utf-8\"?>
<pfif:pfif xmlns:pfif=\"http://zesty.ca/pfif/1.3\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xsi:schemaLocation=\"http://zesty.ca/pfif/1.3 http://zesty.ca/pfif/1.3/pfif-1.3.xsd\">";
// print each person id
foreach ($person_ids as $person_id) {
print "
	<pfif:person>
		<pfif:person_record_id>
			$person_id
		</pfif:person_record_id>
		<pfif:expiry_date>
			$datetime
		</pfif:expiry_date>
	</pfif:person>";
}
# Print XML footer.
print "\n</pfif:pfif>";
