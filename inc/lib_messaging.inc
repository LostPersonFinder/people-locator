<?
/**
 * @name    Taupo Messaging Library
 * @author  pl@miernicki.com
 * @about   Developed by the U.S. National Library of Medicine
 * @link    https://gitlab.com/tehk/people-locator
 * @license	https://gitlab.com/tehk/people-locator/blob/master/LICENSE
 */

// send fcm message 
function taupo_message_send($msg, $ids, $goto=null) {
  global $global, $conf;
  $fcm_split = 999; // limit of registration_ids in each fcm call
  $r = array();
  $r['success'] = (int)0;
  $r['failure'] = (int)0;
  // per site config
  if($conf['site_name'] == 'NLM PEOPLE LOCATOR') {
    $icon  = $conf['base_url'].'/assets/images/logo/logo-white-128.png';
    $badge = $conf['base_url'].'/assets/images/logo/logo-white-transparent-128.png';
  } elseif($conf['site_name'] == 'PL STAGE') {
    $icon  = $conf['base_url'].'/assets/images/logo/logo-white-128-stage.png';
    $badge = $conf['base_url'].'/assets/images/logo/logo-white-transparent-128-stage.png';
  } else {
    $icon  = $conf['base_url'].'/assets/images/logo/logo-white-128-dev.png';
    $badge = $conf['base_url'].'/assets/images/logo/logo-white-transparent-128-dev.png';
  }
  // default goto location
  if($goto == null) { $goto = $conf['base_url'].'/'; }
  // chunkify
  $ids_chunks = array_chunk($ids, $fcm_split); // split into chunks
  // send one chunk at a time
  foreach($ids_chunks as $ids_chunk) {
    // push payload
    $payload = array(
      'registration_ids' => $ids_chunk,
      'data' => array(
        'icon'    => $icon,
        'badge'   => $badge,
        'goto'    => $goto
      ),
      'notification' => array(
        'body'              => $msg,
        'title'             => $conf['site_name'],
        'content_available' => (int)1
      ),
      'priority' => 'high'
    );
    // send payload to chunk
    $headers = array('Authorization: key=***','Content-Type: application/json'); // set headers (authentication and type)
    $ch = curl_init(); // init curl handle
    curl_setopt($ch, CURLOPT_URL, 'https://fcm.googleapis.com/fcm/send'); // fcm endpoint
    curl_setopt($ch, CURLOPT_POST, true); // set request method to POST
    if($conf['base_uuid'] == 'pl.hostname.gov/') { curl_setopt($ch, CURLOPT_PROXY, 'lhc-pub-proxy:3128'); }
    curl_setopt($ch, CURLOPT_HTTPHEADER, $headers); // set our custom headers
    curl_setopt($ch, CURLOPT_RETURNTRANSFER, true); // get the response back as a string instead of printing it
    curl_setopt($ch, CURLOPT_SSL_VERIFYPEER, false );
    curl_setopt($ch, CURLOPT_POSTFIELDS, json_encode($payload)); // set post data as JSON
    //error_log(json_encode($payload));
    $curl_out = curl_exec($ch); // post!
    $ce = (int)curl_errno($ch); // grab error code // PL-1797
    // retry up to 10 times on error
    $retry = 0;
    $delay = 1; // retry delay seconds
    while($ce > 0 && $retry < 8) {
      daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, 'FCM CURL Error! (retry:'.$retry.')', curl_error($ch)); // log error
      $delay = $delay * 2; // double delay period
      sleep($delay);
      $curl_out = curl_exec($ch);
      $ce = (int)curl_errno($ch);
      $retry++;
    }
    // success
    if($ce === 0) {
      $a_out = json_decode($curl_out, true);
      $r['success'] += (int)$a_out['success'];
      $r['failure'] += (int)$a_out['failure'];
      error_log('FCM +'.$a_out['success'].' -'.$a_out['failure']);
      // prune all invalid and deregistered tokens
      $count = count($ids_chunk);
      for($i = 1; $i <= $count; $i++) {
        if(isset($a_out['results'][$i-1]['error']) && (
          $a_out['results'][$i-1]['error'] == 'NotRegistered' ||
          $a_out['results'][$i-1]['error'] == 'InvalidRegistration' ||
          $a_out['results'][$i-1]['error'] == 'MismatchSenderId'
        )) {
          $q = "DELETE FROM `push` WHERE `registration_id` = ".$global['db']->qstr($ids_chunk[$i-1]).";";
          $result = $global['db']->Execute($q);
          if($result === false) { daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "not 2 ((".$q."))"); }
        }
      }
    }
    curl_close($ch); // close curl handle
    $ch = null;
  }
  // log
  if(isset($_SESSION['user'])) { $user = $global['db']->qstr((string)$_SESSION['user']); }
  else                         { $user = "'system'"; }
  $q = "INSERT INTO `notifications` (`not_message`,`user`,`sent_to`,`success`,`failure`) VALUES (
      ".$global['db']->qstr((string)$msg).",
      ".$user.",
      ".$global['db']->qstr((int)count($ids)).",
      ".$global['db']->qstr((int)$r['success']).",
      ".$global['db']->qstr((int)$r['failure']).");
  ";
  $result = $global['db']->Execute($q);
  if($result === false) { daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "not 3 ((".$q."))"); }
  $r['retry'] = $retry;
  $r['error'] = $ce;
  $rj = json_encode($r);
  return $rj;
}

// send message to admins
function taupo_message_admins($msg=null, $msge=null, $goto=null) {
  if($msg == null) { return; }
  global $global, $conf;
  // push to these admins
  $q = '
    SELECT * 
    FROM   push p, users u
    WHERE  p.uid = u.uid
    AND    u.gid = 1
    AND    u.admin_push = 1;
  ';
  $ids = array();
  $result = $global['db']->Execute($q);
  if($result === false) { daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "msg admins 1 ((".$q."))"); }
  while(!$result == NULL && !$result->EOF) {
    $ids[] = $result->fields["registration_id"];
    $result->MoveNext();
  }
  taupo_message_send($msg, $ids, $goto);
  // email to these admins
  $q = '
    SELECT * 
    FROM   users
    WHERE  gid = 1
    AND    admin_email = 1;
  ';
  $result = $global['db']->Execute($q);
  if($result === false) { daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "msg admins 2 ((".$q."))"); }
  while(!$result == NULL && !$result->EOF) {
    $e = new email();
    $subject  = $conf['site_name'].' Admin Message';
    if($msge == null) { $bodyHTML = $msg;  $bodyAlt  = $msg; } // send push body
    else              { $bodyHTML = $msge; $bodyAlt  = $msg; } // send email body for html
    $e->sendMessage($result->fields['user'], $subject, $bodyHTML, $bodyAlt, 'no-reply@');
    $result->MoveNext();
  }
}

// send notify module msg
function taupo_message_notify($msg=null, $admin=false, $goto=null) {
  global $global, $conf;
  if($admin == true) { $q = "SELECT * FROM `push` t, `users` u WHERE  t.uid = u.uid AND u.gid = 1;"; } // just admins
  else               { $q = "SELECT * FROM `push` t; "; } // all devices
  $ids = array();
  $result = $global['db']->Execute($q);
  if($result === false) { daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "not 1 ((".$q."))"); }
  while(!$result == NULL && !$result->EOF) {
    $ids[] = $result->fields["registration_id"];
    $result->MoveNext();
  }
  return taupo_message_send($msg, $ids, $goto);
}

// count all registered devices
function get_push_count_all() {
  global $global;
  $q = "SELECT count(*) FROM `push`;";
  $result = $global['db']->Execute($q);
  if($result === false) { daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "a0 ((".$q."))"); }
  $row = $result->FetchRow();
  return (int)$row['count(*)'];
}

// count registered web devices
function get_push_count_web() {
  global $global;
  $q = "SELECT count(*) FROM `push` WHERE `version_string` like 'web_browser%';";
  $result = $global['db']->Execute($q);
  if($result === false) { daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "a0 ((".$q."))"); }
  $row = $result->FetchRow();
  return (int)$row['count(*)'];
}

// count registered android devices
function get_push_count_android() {
  global $global;
  $q = "SELECT count(*) FROM `push` WHERE `version_string` not like 'web_browser%' AND `version_string` like '%ndroid%';";
  $result = $global['db']->Execute($q);
  if($result === false) { daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "a0 ((".$q."))"); }
  $row = $result->FetchRow();
  return (int)$row['count(*)'];
}

// count registered ios devices
function get_push_count_ios() {
  global $global;
  $q = "SELECT count(*) FROM `push` WHERE `version_string` not like 'web_browser%' AND `version_string` not like '%ndroid%';";
  $result = $global['db']->Execute($q);
  if($result === false) { daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "b0 ((".$q."))"); }
  $row = $result->FetchRow();
  return (int)$row['count(*)'];
}

// count registered admin devices
function get_push_count_admin() {
  global $global;
  $q = "SELECT count(*) FROM `push` p, `users` u WHERE  p.uid = u.uid AND u.gid = 1;";
  $result = $global['db']->Execute($q);
  if($result === false) { daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "a0 ((".$q."))"); }
  $row = $result->FetchRow();
  return (int)$row['count(*)'];
}
