#!/usr/bin/php
<?php
// check for SOLR errors and e-mail if found
error_reporting(E_ALL);
ini_set("display_errors", "stdout");
require_once("../conf/taupo.conf");
require_once("../inc/lib_includes.inc");
$email = "@";
$subject = "SOLR errors detected";
if (!taupo_solr_status()) {
  $msg = "A daily PL scan has detected SOLR is down on ".$conf['db_name'];
  mail($email, $subject, $msg);
} else if (!taupo_solr_log_status(false, true)) {
  $msg = "A daily PL scan has detected SOLR log errors on ".$conf['db_name'];
  mail($email, $subject, $msg);
}
