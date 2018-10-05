<?
/**
 * @name     notify
 * @author   pl@miernicki.com
 * @about    Developed by the U.S. National Library of Medicine
 * @link     https://gitlab.com/tehk/people-locator
 * @license  https://gitlab.com/tehk/people-locator/blob/master/LICENSE
 */

$global['xajax_functions'] = array();
array_push($global['xajax_functions'], 'notify_send');

// send mass notification
function notify_send($msg=null, $admin=0, $goto=null) {
  global $global, $conf;
  $rj = taupo_message_notify($msg, $admin, $goto); // send notification
  $r  = json_decode($rj, true);
  taupo_acl_log('PUSH_NOTIFY', 'user '.$_SESSION['user'].' push notified '.$r['success'].' devices');
  $out = 'Push message sent. Received: '.$r['success'].'. Failed: '.$r['failure'].'.';
  if((int)$r['error'] === 0) {
    if((int)$r['retry'] === 0) { $global['xajax_res']->addScript("toast3('".$out."');"); }
    else                       { $global['xajax_res']->addScript("toast2('".$out." Retries: ".$r['retry']."."."');"); }
  } else { $global['xajax_res']->addScript("toast('".$out." Retries: ".$r['retry'].". ERROR: ".$r['error']."."."');"); }
  return $global['xajax_res']->getXML();
}
