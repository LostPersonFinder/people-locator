<?
/**
 * @name     unit test // dao log // onerror log
 * @author   pl@miernicki.com
 * @about    Developed by the U.S. National Library of Medicine
 * @link     https://gitlab.com/tehk/people-locator
 * @license  https://gitlab.com/tehk/people-locator/blob/master/LICENSE
 */

error_reporting(E_ALL);
ini_set("display_errors", "stdout");

function taupo_unit_default() {
  global $global, $conf;
  // admin or api key required
  if((int)$_SESSION['gid'] === 1) {} elseif(!isset($_REQUEST['key']) || $_REQUEST['key'] !== $conf['service_key']) {
    http_response_code(404);
    echo '404';
    die();
  }
  $dao = '';
  $q = "SELECT * FROM `dao_error_log`;";
  $result = $global['db']->Execute($q);
  while (!$result == NULL && !$result->EOF) {
    $dao .= 
    $result->fields['time']." // uid:".$result->fields['uid']."\n".
    $result->fields['file'].":".$result->fields['line']."\n".
    $result->fields['error_message']."\n".
    $result->fields['other']."\n\n\n\n";
    $result->MoveNext();
  }
  $onerror = '';
  $q = "SELECT * FROM `onerror`;";
  $result = $global['db']->Execute($q);
  // use stack or message
  if(trim($result->fields['stack']) !== '') { $stack = $result->fields['stack']; }
  else { $stack = $result->fields['message']; }
  $loca = json_encode(json_decode($result->fields['location']), JSON_PRETTY_PRINT);
  $loca = str_replace('\\', '', $loca);
  while (!$result == NULL && !$result->EOF) {
    $onerror .= 
    $result->fields['at_time']." // uid:".$result->fields['uid']." // gid:".$result->fields['gid']." // ".$result->fields['ip']."\n".
    $result->fields['os']." // ".$result->fields['browser']."\n".
    $result->fields['file'].":".$result->fields['line']."/".$result->fields['column']."\n".
    'window.location '.$loca."\n".
    $stack."\n\n\n\n\n";
    $result->MoveNext();
  }
  $subject = $conf['site_name'].' :: DAOLOG && ONERROR';
  $body =
    "DAOLOG\n".
    "--------------------------------------------------------------\n".
    "\n".
    "$dao \n".
    "\n".
    "\n".
    "ONERROR\n".
    "--------------------------------------------------------------\n".
    "\n".
    "$onerror \n";
  // PL-1777
  $p = new email();
  $p->sendMessage('@', $subject, null, $body, '@');
  $p = new email();
  $p->sendMessage('@', $subject, null, $body, '@');
  // clear logs
  $q = "TRUNCATE dao_error_log;";
  $result = $global['db']->Execute($q);
  $q = "TRUNCATE onerror;";
  $result = $global['db']->Execute($q);
  echo 'DONE @ '.date("Y-m-d H:i:s");
}
