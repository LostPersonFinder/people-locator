#!/usr/bin/php

<?php

/*
 * Takes all the images for a single event or "all" events and ingests them in FaceMatch. 
 * Nice tool to have when images need to be ingested or reingested for some reason.
 * NOTE: If images were originally ingested with region specified, this info
 * will be lost. But since PL doesn't ingest with regions, this affects mainly reloading 
 * of ground truth images. So when the latter need to be reingested, use BIU.
 */

error_reporting(E_ALL);
ini_set("display_errors", "stdout");
require_once("../conf/taupo.conf");
require_once("../inc/lib_includes.inc");

// Get incident number from arg 1.
if ($argc < 2) { die("Wrong number of arguments: Please supply an incident #.\n"); }

$incidents = [];
if ($argv[1] == "all") {
  // Fetch all existing incident IDs
  $sql = "SELECT incident_id FROM incident";
  $result = $global['db']->Execute($sql);
  while ($row = $result->FetchRow()) {
    $incidents[] = $row['incident_id'];
  }
} else {
  $incidents[] = $argv[1];
}
 
foreach ($incidents as $incident_id) {
  // Recreate the event.
  try {
    taupo_facematch_delete_event($incident_id);
  } catch (Exception $e) {
    // Proababy a new event so ignore.
  }
  try {
    taupo_facematch_add_event($incident_id);
  } catch (Exception $e) {
    // Don't proceed if can't create the event.
    die($e->getMessage()."\n");
  }
  $attr = array();
  // Get all images, gender and age for this event.
  $sql = "SELECT i.url, pd.opt_gender, pd.years_old, pd.minAge, pd.maxAge FROM image i, person_uuid pu, person_details pd" .
	  " where pu.incident_id=$incident_id" .
	  " and pu.p_uuid = i.p_uuid" .
	  " and pu.p_uuid = pd.p_uuid";
  $image_result = $global['db']->Execute($sql);
  if ($image_result === false) {
    $errchk = $global['db']->ErrorMsg();
    die("Error getting images with attributes: " . $errchk);
  }
  while ($image_row = $image_result->FetchRow()) {
    if ($image_row['opt_gender'] == 'fml') {
      $attr['gender'] = 'female';
    } else if ($image_row['opt_gender'] == 'mal') {
      $attr['gender'] = 'male';
    } else {
      $attr['gender'] = 'unknown';
    }        
    if ($image_row['years_old'] != '') {
      $attr['age'] = $image_row['years_old'];
    } else if ($image_row['minAge'] != ''  && $image_row['maxAge'] != '') {
      $attr['age'] = floor(($image_row['minAge']  + $image_row['maxAge']) / 2);
    } else {
      $attr['age'] = -1;
    }        
    try {
      taupo_facematch_ingest($image_row['url'], $incident_id, UNKNOWN_SOURCE, $attr);
      print "FaceMatch ingest success: image '".$image_row['url']."', incident '".$incident_id."', gender '".$attr['gender']."', age '".$attr['age']."'\n";
    } catch (Exception $e) {
      echo $e->getMessage()."\n";
    }
  }
}
