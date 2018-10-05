<?php

// Report any cached PFIF images which are not in the image table
// and whether the image has an associated person.
//
// Note: This is easily generalized to checking all cached images.

error_reporting(E_ALL);
ini_set("display_errors", "stdout");

$conf['approot'] = getcwd() . "/../";
require_once("../conf/taupo.conf");
require_once("../inc/lib_includes.inc");

// Get the file list image files.
$directory = "../www/tmp/pfif_cache/";
 
//get all files in specified directory
$files = glob($directory . "*");
 
$urls = array();
foreach($files as $file) {
	$urls[] = substr($file, 7);
}
 
// print names not in DB
foreach ($urls as $url) {
 	if (strpos($url,"thumb") !== false) {
        	// It's a thumbnail, so check image thumbnail column.
		$q = "SELECT * FROM image WHERE url_thumb = '" . $url . "'"; 
		$result = $global['db']->Execute($q);
		if($result == false) { echo "DB Error!"; }
	
		if ($result == NULL || $result->EOF) {
			// No such image. Is there an associated person?
			$tmp = substr(strrchr($url, "/"), 7);  
			$end = strpos($tmp, "__");  
			$person_id = str_replace("SLASH", "/", substr($tmp, 0, $end));  
	
			$q = "SELECT * FROM person_uuid WHERE p_uuid = '" . $person_id . "'"; 
			$result = $global['db']->Execute($q);
			if($result == false) { echo "DB Error!"; }
		
			if ($result == NULL || $result->EOF) {
				$hasPerson = " (has no associated person)";	
			} else {
				$hasPerson = "";	
			}
			print "../www/" . $url . $hasPerson . "\n";
		}
	} else {
		$q = "SELECT * FROM image WHERE url = '" . $url . "'"; 
		$result = $global['db']->Execute($q);
		if($result == false) { echo "DB Error!"; }
	
		if ($result == NULL || $result->EOF) {
			// Is there an associated person?
			$tmp = substr(strrchr($url, "/"), 1);  
			$end = strpos($tmp, "__");  
			$person_id = str_replace("SLASH", "/", substr($tmp, 0, $end));  

			$q = "SELECT * FROM person_uuid WHERE p_uuid = '" . $person_id . "'"; 
			$result = $global['db']->Execute($q);
			if($result == false) { echo "DB Error!"; }
	
			if ($result == NULL || $result->EOF) {
				$hasPerson = " (has no associated person)";	
			} else {
				$hasPerson = "";	
			}
			print "../www/" . $url . $hasPerson . "\n";
		}
	}
}
