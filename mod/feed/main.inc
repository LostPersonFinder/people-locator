<?
/**
 * @name     feed
 * @author   pl@miernicki.com
 * @author   pl@miernicki.com
 * @about    Developed by the U.S. National Library of Medicine
 * @link     https://gitlab.com/tehk/people-locator
 * @license  https://gitlab.com/tehk/people-locator/blob/master/LICENSE
 */

// provide pfif feed
function taupo_feed_default() {
  global $global, $conf;
  require_once($conf['approot']."inc/lib_ga.inc");
  require_once($conf['approot']."mod/pfif/pfif.inc");
  error_reporting(0);
  // headers
  ob_start("ob_gzhandler"); // Enable gzip compression
  // check for valid key
  $key = isset($_GET['key']) ? $_GET['key'] : '';
  if (!isset($_GET['key']) || $_GET['key'] != $conf['feed_key']) {
    echo "<h3>Missing or invalid key.</h3>";
    ob_end_flush();
    exit;
  }
  // parameters
  $original = isset($_GET['original']) ? $_GET['original'] : 'false';
  $since    = isset($_GET['since'])    ? $_GET['since']    : date("Y")."-01-01 00:00:00";
  $before   = isset($_GET['before'])   ? $_GET['before']   : date("Y-m-d h:i:s");
  $max      = isset($_GET['max'])      ? $_GET['max']      : 0;
  $skip     = isset($_GET['skip'])     ? $_GET['skip']     : 0;
  // find name
  $sql = "
    SELECT *
    FROM incident
    WHERE closed = 'false'
    AND private_group IS NULL
    AND shortname = ".$global['db']->qstr($_GET['shortname']).";
  ";
  $res = $global['db']->GetRow($sql);
  $iid = $res['incident_id'];
  $pfif = new Pfif();
  $pfif->setIncidentId($iid);
  $pfif->loadFromDatabase($since, $before);
  $xml = $pfif->storeInXML(true, $original==='true', $max, $skip);
  if($xml == null) { echo "<h3>Sorry, there are no records matching your request.</h3>"; }
  else { 
    header("Content-type: text/xml");
    echo $xml;  
  }
  ob_end_flush();
  taupo_ga_trackPageView();
}
