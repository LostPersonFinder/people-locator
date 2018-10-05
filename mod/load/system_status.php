<?
// This script serves as both the LHC F5 load balancer and the Site24x7 health monitor.
// It should report "SYSTEM IS UP" if healthy.

global $global, $conf;

error_reporting(0);
ob_start("ob_gzhandler"); // Enable gzip compression

function check_solr() {
  global $conf;
  $solr_url = $conf["SOLR_root"] . "admin/ping";
  $ch = curl_init();
  curl_setopt($ch, CURLOPT_URL, $solr_url);
  curl_setopt($ch, CURLOPT_PORT, $conf['SOLR_port']);
  curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
  $data = curl_exec($ch);
  curl_close($ch);
  if (!$data) {
    exit_down();
  }
}

function check_mysql() {
  global $conf;
  $mysqli = @new mysqli( $conf["db_host"], $conf["db_user"], $conf["db_pass"], $conf["db_name"], $conf["db_port"] );
  if ($mysqli->connect_errno) {
     exit_down();
  }
}

function exit_down() {
  http_response_code(503);
  echo "SYSTEM IS DOWN!";
  exit;
}

$conf['SOLR_on'] && check_solr();
check_mysql();
echo "SYSTEM IS UP";

ob_end_flush();
