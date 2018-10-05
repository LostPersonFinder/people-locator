#!/usr/bin/php
<?php
// check for import/export errors and e-mail the result 
error_reporting(E_ALL);
ini_set("display_errors", "stdout");
require_once("../conf/taupo.conf");
require_once("../inc/lib_includes.inc");
$email = "@";
$subject = "Import/export problem detected";

$msg = "";
// A down message overrides one about errors in log.
if (!check_db_end_times()) {
	$msg = "A daily PL scan has detected imports or exports are down on ".$conf['db_name'];
} else if (!check_error_files()) {
	$msg = "A daily PL scan has detected import or export log errors on ".$conf['db_name'];
}
if (!empty($msg)) {
	mail($email, $subject, $msg);
}

// Check that there are no non-empty error files
function check_error_files() {
   return (check_import_error_file() && check_export_error_file());
}

// Check that there are no import errors
function check_import_error_file() {
   $import_err = '/opt/pl/www/tmp/pfif_logs/import.err';
   if (filesize($import_err) > 0) {
      return 0;
   }
   return 1;
}

// Check that there are no export errors
function check_export_error_file() {
   $export_err = '/opt/pl/www/tmp/pfif_logs/export.err';
   if (filesize($export_err) > 0) {
      return 0;
   }
   return 1;
}

// Check that end_times in all active logs are current
function check_db_end_times() {
   global $global;
   // Check end times of all active person imports.
   $q = "SELECT
                (SELECT end_time FROM pfif_harvest_person_log ph WHERE ph.repository_id = pr.id ORDER BY log_index DESC LIMIT 1) AS end_time
                FROM pfif_repository pr
                WHERE role = 'source'
                AND resource_type = 'person'
                AND pr.sched_interval_minutes != 0
           ";
   $result = $global['db']->Execute($q);
   while ($row = $result->FetchRow()) {
      if (!check_end_time($row['end_time'])) {
         return 0;
      }
   }
   // Check end times of all active note imports.
   $q = "SELECT
                (SELECT end_time FROM pfif_harvest_note_log ph WHERE ph.repository_id = pr.id ORDER BY log_index DESC LIMIT 1) AS end_time
                FROM pfif_repository pr
                WHERE role = 'source'
                AND resource_type = 'note'
                AND pr.sched_interval_minutes != 0
           ";
   $result = $global['db']->Execute($q);
   while ($row = $result->FetchRow()) {
      if (!check_end_time($row['end_time'])) {
         return 0;
      }
   }
   // Check end times of all active exports.
   $q = "SELECT
                (SELECT end_time FROM pfif_export_log pe WHERE pe.repository_id = pr.id ORDER BY log_index DESC LIMIT 1) AS end_time
                FROM pfif_repository pr
                WHERE role = 'sink'
                AND pr.sched_interval_minutes != 0
           ";
   $result = $global['db']->Execute($q);
   while($row = $result->FetchRow()) {
      if (!check_end_time($row['end_time'])) {
         return 0;
      }
   }
   return 1;
}

// Is time of last activity within an acceptable delta
function check_end_time($time) {
  global $conf;
  $ts = strtotime($time);
  $delta = date("U") - $ts;
  $timeoutPeriod = 300;
  return (($delta < $timeoutPeriod) || $timeoutPeriod == 0);
}
