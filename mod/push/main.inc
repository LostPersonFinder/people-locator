<?
/**
 * @name     Push Notification Weekly Test
 * @author   pl@miernicki.com
 * @about    Developed by the U.S. National Library of Medicine
 * @link     https://gitlab.com/tehk/people-locator
 * @license  https://gitlab.com/tehk/people-locator/blob/master/LICENSE
 */

// weekly push notification test
function taupo_push_default() {
  global $global, $conf;
  $msg = 'Push Notification Weekly Test.';
  // key required to email
  if(isset($_REQUEST['key']) && $_REQUEST['key'] == $conf['service_key']) { taupo_message_notify($msg, true); }
  else { http_response_code(404); echo '404'; die(); }
}
