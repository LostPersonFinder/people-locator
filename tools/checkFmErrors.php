#!/usr/bin/php
<?php
// check for FaceMatch errors and send e-mail if found
error_reporting(E_ALL);
ini_set("display_errors", "stdout");
require_once("../conf/taupo.conf");
require_once("../inc/lib_includes.inc");
$email = "@";
$subject = "FaceMatch errors detected";
if (!taupo_facematch_status()) {
  $msg = "A daily PL scan has detected FaceMatch is down on ".$conf['db_name'];
  mail($email, $subject, $msg);
} else if (!taupo_facematch_log_status()) {
  $msg = "A daily PL scan has detected FaceMatch log errors on ".$conf['db_name'];
  mail($email, $subject, $msg);
}
