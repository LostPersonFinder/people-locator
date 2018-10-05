#!/usr/bin/php
<?php
// report challenge directory images
error_reporting(E_ALL);
ini_set("display_errors", "stdout");
$conf['approot'] = getcwd() . "/../";
require_once("../conf/taupo.conf");
require_once("../inc/lib_includes.inc");
$challengeDir = $conf['approot'].'www/tmp/challenge_cache/';
// are there challenge images from last 24 hours? (cron runs each night)
$files = array();
foreach (new DirectoryIterator($challengeDir) as $fileInfo) {
  // ignore all files beginning with period, including .htaccess file
  if(strpos($fileInfo->getFilename(), '.') === 0) continue;
  $ctime = $fileInfo->getCTime();
  if($ctime < strtotime('-1 day')) continue;
  $files[] = $fileInfo->getFilename();
}
if (!empty($files)) { send_challenge_msg($files); } // send out an alert

function send_challenge_msg($files) {
  global $conf;
  $headers = 'From: @';
  $host = $conf['db_name'];
  $msg = "A daily $host scan has detected one or more images for which no face could be found:";
  foreach ($files as $file) { $msg .= "\n\nhttps://$host.nlm.nih.gov/tmp/challenge_cache/$file"; }
  mail("@,@", "$host challenge image detected", $msg, $headers);
}
