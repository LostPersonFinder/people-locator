<?
/**
 * @name     LOADer
 * @author   pl@miernicki.com
 * @about    Developed by the U.S. National Library of Medicine
 * @link     https://gitlab.com/tehk/people-locator
 * @license  https://gitlab.com/tehk/people-locator/blob/master/LICENSE
 */

// default
function taupo_load_default() {
  http_response_code(404);
  echo '404';
  die(); 
}

// show system status
function taupo_load_systemstatus() {
  require_once('system_status.php');
}

// show system status
function taupo_load_fmstatus() {
  require_once('fm_status.php');
}

// show sitemap
function taupo_load_sitemap() {
  require_once('sitemap.php');
}

// show /manifest.json
function taupo_load_manifest_json() {
  require_once('manifest_json.php');
}

// show /service-worker.js
function taupo_load_sworker_js() {
  require_once('service_worker_js.php');
}

// show /robots.txt
function taupo_load_robots_txt() {
  header('Content-Type:text/plain');
?>Sitemap: https://pl.nlm.nih.gov/sitemap.xml
User-agent: *
Disallow: /rest_api<?
}

// show /browserconfig.xml
function taupo_load_browserconfig() {
  global $global, $conf;
  header("Content-type: text/xml");
  echo file_get_contents($conf['approot']."/mod/load/browserconfig.xml");
}
