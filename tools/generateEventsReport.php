#!/usr/bin/php

<?php
/**
 * Generate historical events report in html format for periodic e-mail.
 * NOTE: Only show events with non-zero reports.
 */

error_reporting(E_ALL);
ini_set("display_errors", "stdout");

$conf['approot'] = getcwd() . "/../";
require_once("../conf/config_PL.inc");
require_once("../inc/lib_includes.inc");

// Is this for browser viewing?
$browser = ($argc > 1 && ($argv[1] == "1" || $argv[1] == "3"));
// Is this an internal report?
$internal = ($argc > 1 && ($argv[1] == "2" || $argv[1] == "3"));

// Google Maps Geocoding key generated using LPF's Google Cloud Platform.
$geocode_key = 'AIzaSyDb7y3C_FMhRZ_RcamKmYW_GfN2ICfkbWU';
$browser_file = ($internal)? "events_for_browser_internal.html":"events_for_browser.html";
$table_width = ($internal)? "500px":"850px";
$colspan = ($internal)? 4 : 7;
$active = ($internal)? "Date" : "Active Period";
$now = date("m/d/y", strtotime("now"));

$html = "<html>";
$html.= "<head>";
$html .= "<style>";
$html .= 'table {width:'.$table_width.'; border:2px solid black; border-collapse:collapse;}';
$html .= 'tr,td,th {text-align: center; border:1px solid black;}';
$html .= 'td,th {padding-left:5px; padding-right:5px;}';
$html .= '#title {font-weight: bold;}';
$html .= 'tr:nth-child(even) {background-color: #f2f2f2;} tr:hover {background-color: #ddd;}';
$html .= '.rj {text-align: right;} .lj {text-align: left;}';
// Page break in a table doesn't work for Chrome.
$html .= '@media print {#pagetop {page-break-before:always;height:0;} #tobrowse {display:none;}}';
$html .= "</style>";
$html .= "</head>";
$html .= "<body>";
$html .= '<table style="border-style:solid;border-width:1px;">';
$html .= '<tr style="background-color:LightGreen">';
$html .= '<td colspan="'.$colspan.'">';
$html .= '<span id="title">NLM PEOPLE LOCATOR<sup>&reg;</sup> EVENTS ('.$now.')</span>';
if (!$browser) {
  $html .= '<br><span id="tobrowse">View (or print) this report <a href="https://plstage.nlm.nih.gov/tmp/ga_cache/'.$browser_file.'">in your browser</a>.</span>';
}
$html .= '</td>';
$html .= '</tr>';
$header_row = '<tr><th>'.$active.'</th><th>Event Description</th><th>Country</th><th>Type</th>';
if ($internal) {
   $header_row .= '</tr>';
} else {
   $header_row .= '<th>Persons</th><th>Animals</th><th>Photos</th></tr>';
}
$html .= $header_row;

$events = [];
// Fetch all events.
$sql = "SELECT incident_id,name,date,updated,closed,latitude,longitude,type FROM incident ORDER BY date DESC";
$result = $global['db']->Execute($sql);
while ($row = $result->FetchRow()) {
  $events[$row['incident_id']] = array($row['name'],$row['date'],$row['updated'],$row['closed'],$row['latitude'],$row['longitude'],$row['type']);
}

$events_count = 0;
$reports_count = 0;
$animals_count = 0;
$images_count = 0;
foreach ($events as $incident_id=>$event) {
  $long = str_replace('"', '', $event[0]);;
  if (preg_match("/GCI|SKIN|Google|Test|Unlisted|Shankbone/", $long)) continue;

  $startdate = date("m/d/y", strtotime($event[1]));
  $closed = $event[3];
  $enddate = ($closed)? date("m/d/y", strtotime($event[2])) : '';
  $date = ($internal)? date("m/y", strtotime($event[1])) : $startdate."-".$enddate;

  // Get person reports for this event.
  $sql = "SELECT COUNT(*) AS reports FROM person_uuid pu where pu.incident_id=$incident_id AND pu.animal Is NULL";
  $report_result = $global['db']->Execute($sql);
  if ($report_result === false) {
    $errchk = $global['db']->ErrorMsg();
    die("Error getting event reports: " . $errchk);
  }
  $reports = $report_result->FetchRow()['reports'];
  $reports_count += $reports;
  // Only show events with reports.
  if ($reports == 0) continue;

  // Get animal reports for this event.
  $sql = "SELECT COUNT(*) AS animals FROM person_uuid pu where pu.incident_id=$incident_id AND pu.animal Is NOT NULL";
  $animal_result = $global['db']->Execute($sql);
  if ($animal_result === false) {
    $errchk = $global['db']->ErrorMsg();
    die("Error getting event reports: " . $errchk);
  }
  $animals = $animal_result->FetchRow()['animals'];
  $animals_count += $animals;

  // Get image stats for this event.
  $sql = "SELECT COUNT(*) AS images FROM person_uuid pu, image i where i.p_uuid = pu.p_uuid AND pu.incident_id=$incident_id";
  $images_result = $global['db']->Execute($sql);
  if ($images_result === false) {
    $errchk = $global['db']->ErrorMsg();
    die("Error getting event images: " . $errchk);
  }
  $images = $images_result->FetchRow()['images'];
  $images_count += $images;
  if ($long == "Jammu Kashmir Floods"){
    // Google won't show Jammu Kashmir as being in India for some reason.
    $country = "India";
  } else if ($long == "Global Reporting Event"){
    $country = "Global";
  } else {
    $country = getGeoCountry($event[4].",".$event[5], $geocode_key);
  }

  // Get real or drill.
  $type = ($event[6] == "REAL")? "real" : "drill";
  $type = ($long=="LA County Event" || $long=="Global Reporting Event")? "perm" : $type;
  $type = ($long=="Halloween Party 2014")? "test" : $type;

  $html .= '<tr><td class="lj">'.$date.'</td><td class="lj">'.$long.'</td><td>'.$country.'</td><td>'.$type.'</td>';
  if ($internal) {
    $html .= '</tr>';
  } else {
    $html .= '<td class="rj">'.number_format($reports).'</td><td class="rj">'.number_format($animals).'</td><td class="rj">'.number_format($images).'</td></tr>';
  }

  if ($events_count == 38) {
    $html .= '<tr id="pagetop"></tr>';
    $html .= $header_row;
  }

  $events_count++;
}
$html .= '<tr><td><b>Total</b></td><td class="lj"><b>'.$events_count.'</b></td><td></td><td></td>';
if ($internal) {
  $html .= '</tr>';
} else {
  $html .= '<td class="rj"><b>'.number_format($reports_count).'</b></td><td class="rj"><b>'.$animals_count.'</b></td><td class="rj"><b>'.number_format($images_count).'</b></td></tr>';
}
$html .= '</table>';
$html .= '</body>';
$html .= '</html>';

echo $html;

function getGeoCountry($latlng, $key) {
  $request = 'https://maps.googleapis.com/maps/api/geocode/json?key='.$key.'&latlng='.$latlng.'&sensor=false'; 
  $file_contents = file_get_contents($request);
  $json_decode = json_decode($file_contents);
  if($json_decode->status != 'OK') { // if Status Code is ZERO_RESULTS, OVER_QUERY_LIMIT, REQUEST_DENIED or INVALID_REQUEST
    // This will print instead of "country" so we get an alert.
    return $json_decode->status;
  }
  if(isset($json_decode->results[0])) {
    foreach($json_decode->results[0]->address_components as $addressComponet) {
      if(in_array('country', $addressComponet->types)) {
        return $addressComponet->long_name; 
      }
    }
  }
}
