#!/usr/bin/php

<?php

/*
 * Renames all Google Person Finder images for all events. This is useful if a complete rename of all pfif images is required. 
 * This is to defeat bookmarking of images for viewing after they are no longer supposed to be retained.
 * NOTE: This functionality is also incorporated in PL and applied to events that are unlisted/archived.
 */

error_reporting(E_ALL);
ini_set("display_errors", "stdout");
require_once("../conf/taupo.conf");
require_once("../inc/lib_includes.inc");

// Get all Google Person Finder images.
$sql = "SELECT i.image_id, i.url, i.url_thumb FROM image i, person_uuid pu" .
	" WHERE pu.p_uuid = i.p_uuid" .
	" AND i.url LIKE '%pfif_cache%'";
$result = $global['db']->Execute($sql);
if ($result === false) {
   $errchk = $global['db']->ErrorMsg();
   die("Error getting images: " . $errchk);
}
$pathprefix = $conf['approot'].'www/';
$random_len = 6;
while ($row = $result->FetchRow()) {
  $image_id = $row['image_id'];
  $full = $row['url'];
  $thumb = $row['url_thumb'];
  $samefilename = ($full == $thumb);
  // Check if already renamed.
  $pattern = '/_([^_]{'.$random_len.'})\./';
  if (preg_match($pattern, $full, $match)) {
    // Already renamed. Replace previous random string.
    $fullfile = $pathprefix.$full;
    $thumbfile = $pathprefix.$thumb;
    $fullnew = str_replace($match[1], random_str($random_len), $full);
    if ($samefilename) {
      // Full and thumb filenames the same so just copy the new name.
      $thumbnew = $fullnew; 
    } else {
      $thumbnew = str_replace($match[1], random_str($random_len), $thumb);
    }
    $fullfilenew = $pathprefix.$fullnew;
    $thumbfilenew = $pathprefix.$thumbnew;
  } else {
    // Insert new random string.
    $fullfile = $pathprefix.$full;
    $thumbfile = $pathprefix.$thumb;
    // Append random string after p_uuid/image_id.
    $pattern = '/(.*)(\..+)$/';
    $replace = '${1}_'.random_str($random_len).'${2}';
    $fullnew = preg_replace($pattern, $replace, $full); 
    if ($samefilename) {
      // Full and thumb filenames the same so just copy the new name.
      $thumbnew = $fullnew; 
    } else {
      $thumbnew = preg_replace($pattern, $replace, $thumb); 
    }
    $fullfilenew = $pathprefix.$fullnew;
    $thumbfilenew = $pathprefix.$thumbnew;
  }
  // Rename the full file.
  if (rename($fullfile, $fullfilenew)) {
    // Update DB.
    $q = "
      UPDATE  image
      SET url = '".$fullnew."'
      WHERE image_id = '".$image_id."';
    ";
    $status = $global['db']->Execute($q);
    if ($status === false) {
      die ("ErrorMsg: ".$global['db']->ErrorMsg());
    }
    echo "$fullfile renamed to $fullfilenew.\n";
  } else {
   echo "Rename of $fullfile to $fullfilenew failed!\n";
  }
  // Rename the thumb file if different file name. Otherwise, just update the DB.
  if (!$samefilename) {
    $status = rename($thumbfile, $thumbfilenew);
  } else {
    $status = 1;
  }
  if ($status) {
    // Update DB.
    $q = "
      UPDATE  image
      SET url_thumb = '".$thumbnew."'
      WHERE image_id = '".$image_id."';
    ";
    $status = $global['db']->Execute($q);
    if ($status === false) {
      die ("ErrorMsg: ".$global['db']->ErrorMsg());
    }
    echo "$thumbfile renamed to $thumbfilenew.\n";
  } else {
    echo "Rename of $thumbfile to $thumbfilenew failed!\n";
  }
}
