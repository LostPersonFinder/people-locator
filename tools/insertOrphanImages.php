<?php

// This script prints out SQL insert commands for orphan PFIF images.
// It does NOT insert the images, as the program names suggests.
// Provide it with next image id, which you get by adding '1' to image_seq.id.
// Be sure to update the image sequence table after you insert new images.
// A better scheme would get the image id sequence dynamically.

error_reporting(E_ALL);
ini_set("display_errors", "stdout");

$conf['approot'] = getcwd() . "/../";
require_once("../conf/taupo.conf");
require_once("../inc/lib_includes.inc");

// Arg 1 must have next image id.
if ($argc < 2) { die("Expect 'next image id' as argument."); }
$index = $argv[1];

// Get the file list image files.
$directory = "../www/tmp/pfif_cache/";
 
//get all files in specified directory
$files = glob($directory . "*");
 
$urls = array();
foreach($files as $file) { $urls[] = substr($file, 7); }
 
// print names not in DB
foreach ($urls as $url) {
 	if (strpos($url,"thumb") === false && strpos($url,"turkey") === false) {
		$q = "SELECT * FROM image WHERE url = '" . $url . "'"; 
		$result = $global['db']->Execute($q);
		if($result == false) { echo "DB Error!"; }
		if($result == NULL || $result->EOF) {
			// Is there an associated person?
			$tmp = substr(strrchr($url, "/"), 1);  
			$end = strpos($tmp, "__");  
			$person_id = str_replace("SLASH", "/", substr($tmp, 0, $end));  
			$q = "SELECT * FROM person_uuid WHERE p_uuid = '" . $person_id . "'"; 
			$result = $global['db']->Execute($q);
			if($result == false) { echo "DB Error!"; }
    	$size = getimagesize("../www/".$url);
    	$channels = isset($size['channels']) ? (int)$size['channels'] : 3;
      $width = $size[0];
			$height = $size[1];
			$type = substr(strrchr($size['mime'], "/"), 1);
			if ($result == NULL || $result->EOF) { print "No person associated with missing image: '" . $url . "'\n"; }
			else {
				print "INSERT INTO image (image_id, p_uuid, url, url_thumb, image_type, image_width, image_height, color_channels) VALUES (" . 
				$index++ . ",'" . 
				$person_id . "','" . 
				$url . "','". 
				$url . "','" .
				$type . "'," .
				$width . "," .
				$height . "," .
				$channels . ");\n";
			}
		}
	}
}
