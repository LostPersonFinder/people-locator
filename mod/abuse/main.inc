<?
/**
 * @name     abuse
 * @author   pl@miernicki.com
 * @about    Developed by the U.S. National Library of Medicine
 * @link     https://gitlab.com/tehk/people-locator
 * @license  https://gitlab.com/tehk/people-locator/blob/master/LICENSE
 */

$global['use_stream'] = true; // use lib_stream
taupo_xajax_init();
require_once($conf['approot'].'mod/abuse/xajax.inc');
taupo_xajax_registerFunction('abuse_show');
taupo_xajax_registerFunction('abuse_disregard');
taupo_xajax_registerFunction('abuse_delete');

function taupo_abuse_default() {
  global $global;
  taupo_xajax_printJavascript();
  echo '<div id="adminHolder" class="adminHolder">';
  echo "<div id=\"rezMain\" class=\"mainArea\"></div><script> abuse_show(); </script>";
  echo '</div>';
}

function taupo_abuse_count() {
  global $global, $conf;
  $q = "
    SELECT count(*)
    FROM `abuse` ;
  ";
  $result = $global['db']->Execute($q);
  if($result === false) { daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "abuse count 1 ((".$q."))"); }
  if($result != null && !$result->EOF) { $aCount = $result->fields['count(*)']; }
  else { $aCount = 0; }
  return (int)$aCount;
}
