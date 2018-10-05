<?
/**
 * @name     REST services library
 * @author   pl@miernicki.com
 * @about    Developed by the U.S. National Library of Medicine
 * @link     https://gitlab.com/tehk/people-locator
 * @license  https://gitlab.com/tehk/people-locator/blob/master/LICENSE
 */

// new cookie/php session
function rest_cookie($token, $uid, $gid, $user) {
  global $global, $conf;
  if(isset($_COOKIE['PL_SESSION'])) { unset($_COOKIE['PL_SESSION']); }
  session_id($token);
  session_name('PL_SESSION');
  session_start();
  $_SESSION['uid']   = $uid;
  $_SESSION['gid']   = $gid;
  $_SESSION['user']  = $user;
  $_SESSION['token'] = $token;  
}

// log access
function rest_log($call=null, $user=null, $token=null) {
  global $global;
  // clean vars
  if($call  === null) { $call  = 'DEFAULT'; } else { $call  = $global['db']->qstr($call); }
  if($user  === null) { $user  = 'DEFAULT'; } else { $user  = $global['db']->qstr($user); }
  if($token === null) { $token = 'DEFAULT'; } else { $token = $global['db']->qstr(substr(trim($token),0,128)); }
  // compute app name
  if(isset($_SERVER['HTTP_USER_AGENT'])) {
    $agent  = explode(' ', $_SERVER['HTTP_USER_AGENT']);
    $appver = explode('/', $agent[0]);
    $app    = isset($appver[0]) ? $appver[0] : '';
    if($app == 'Mozilla') { $app = 'Browser'; }
    $app = $global['db']->qstr($app);
  } else { $app = $global['db']->qstr(''); }
  $ip = $global['db']->qstr(get_client_ip());
  $dt = $global['db']->qstr(date('Y_md_H:i:s.'.getMicrotimeComponent()));
  $q = '
    INSERT INTO rest_log (rest_call,      dt,      ip,      app,      token,      user  )
    VALUES               (  '.$call.', '.$dt.', '.$ip.', '.$app.', '.$token.', '.$user.');
  ';
  $result = $global['db']->Execute($q);
  if($result === false) { daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "rest log access ((".$q."))"); }
}

// authentication
function rest_authCheck($user, $pass) {
  global $global, $conf;
  $q = "
    SELECT `salt`, `status`
    FROM   `users`
    WHERE  `user` = ".$global['db']->qstr($user).";
  ";
  $res = $global['db']->Execute($q);
  if($res === false) { daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "rest auth check 1 ((".$q."))"); }
  // invalid user
  if(($res == null) || ($res->EOF)) { fail2ban(); return 1; }
  // inactive user
  if((string)$res->fields['status'] === 'inactive') { return 3; }
  // check pass
  $salt = $res->fields['salt'];
  $pwd  = substr($pass, 0, 4) .$salt. substr($pass, 4);
  // create digest of the pass collected from the challenge
  $pass_digest = md5(trim($pwd));
  $q2 = "
    SELECT *
    FROM   `users`
    WHERE  `user` = ".$global['db']->qstr($user)."
    AND    `pass` = '".$pass_digest."'
    AND    `salt` = '".$salt."';
  ";
  $res2 = $global['db']->Execute($q2);
  if($res2 === false) { daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "rest auth check 2 ((".$q2."))"); }
  // invalid pass
  if(($res2 == null) || ($res2->EOF)) {
    $cip = fail2ban();
    // if >= threshold/2 failures report that user accountis locked (pretend)
    if((int)$cip >= (int)(MAX_AUTH_FAILURES/2)) { return 2; }
    else                                        { return 1; }
  }
  // successful auth
  return 0;
}

// verify validity of a token // it exists
function rest_validToken($token) {
  global $global, $conf;
  rest_removeExpiredTokens();
  $q = "
    SELECT *
    FROM   `tokens`
    WHERE  `sha_512_token` = ".$global['db']->qstr($token).";
  ";
  $res = $global['db']->Execute($q);
  if($res === false) { daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "rest valid token ((".$q."))"); }
  // invalid token
  if(($res == null) || ($res->EOF)) { return false; }
  // valid token
  else {
    // update anon token expiry // keep alive
    if($res->fields["gid"] == 3) { rest_keepAliveAnonToken($token); }
    return true;
  }
}

// get token gid
function rest_gidToken($token) {
  global $global, $conf;
  $q = "
    SELECT *
    FROM   `tokens`
    WHERE  `sha_512_token` = ".$global['db']->qstr($token)."
    LIMIT  1;
  ";
  $res = $global['db']->Execute($q);
  if($res === false) { daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "rest token id ((".$q."))"); }
  // invalid token // gid 3:anonymous
  if(($res == null) || ($res->EOF)) { return 3; }
  // valid token
  else { return $res->fields["gid"]; }
}

// auth for token
function rest_checkToken($token) {
  global $global;
  rest_removeExpiredTokens();
  $a = array();
  $q = "
    SELECT *
    FROM   `tokens` t, `users` u
    WHERE  t.uid = u.uid
    AND    sha_512_token = ".$global['db']->qstr($token).";
  ";
  $res = $global['db']->Execute($q);
  if($res === false) { daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "rest check token ((".$q."))"); }
  // invalid token
  if(($res == null) || ($res->EOF)) {
    $a['status'] = 9000;
    $a['uid']  = null;
    $a['gid']  = null;
    $a['user'] = null;
    fail2ban();
  // valid token
  } else {
    $a['status'] = 0;
    $a['uid']  = $res->fields['uid'];
    $a['gid']  = $res->fields['gid'];
    $a['user'] = $res->fields['user'];
    // update anon token expiry
    if($res->fields["gid"] == 3) { rest_keepAliveAnonToken($token); }
  }
  return $a;
}

// validate token for event
function rest_checkEventToken($token, $short) {
  global $global, $conf;
  rest_keepAliveAnonToken($token);
  rest_removeExpiredTokens();
  $a = array();
  $q = "
    SELECT *
    FROM   `tokens`
    WHERE  `sha_512_token` = ".$global['db']->qstr($token).";
  ";
  $res = $global['db']->Execute($q);
  if($res === false) { daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "rest check event token 1 ((".$q."))"); }
  // invalid token
  if(($res == null) || ($res->EOF)) {
    $a['status'] = 9000;
    $a['uid'] = null;
    $a['gid'] = null;
    fail2ban();
  // valid token
  } else {
    $a['status'] = 0;
    $a['uid'] = $res->fields["uid"];
    $a['gid'] = $res->fields["gid"];
    // gid event access check
    $q = "
      SELECT *
      FROM   incident
      WHERE  shortname = ".$global['db']->qstr($short)."
      LIMIT  1;
    ";
    $result = $global['db']->Execute($q);
    if($res === false) { daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "rest check event token 2 ((".$q."))"); }
    // invalid event
    if($result == null || $result->EOF) { $a['status'] = 8000; }
    else {
      $gid = $result->fields["private_group"];
      // if admin access all
      if((int)$a['gid'] === 1) {}
      // public event
      elseif($gid == null) {}
      // token cannot access event
      else { 
        $a['status'] = 9001;
        fail2ban();
      }
    }
  }
  return $a;
}

// anon token keep alive
function rest_keepAliveAnonToken($token) {
  global $global, $conf;
  $q = "
    UPDATE `tokens`
    SET    `expiry` = '".date("Y-m-d G:i:s", time()+MAX_ALIVE)."'
    WHERE  `sha_512_token` = ".$global['db']->qstr($token)."
    AND    `gid` = '3';
  ";
  $r = $global['db']->Execute($q);
  if($r === false) { daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "rest keep alive anon token ((".$q."))"); }
}

// remove all expired tokens
function rest_removeExpiredTokens() {
  global $global, $conf;
  // leave the null tokens and purge expired ones
  $q = '
    DELETE FROM `tokens`
    WHERE `expiry` < NOW()
  ';
  $res = $global['db']->Execute($q);
  if($res === false) { daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "rest removeExpiredTokens ((".$q."))"); }
}

// create a new token
function rest_newToken($uid = 3, $gid = 3, $useThisExpiry = null) {
  global $global, $conf;
  rest_removeExpiredTokens();
  // delete user tokens (starting with the oldest) when the user has more than max tokens
  if($uid !== 3) {
    // we get a list of tokens and delete oldest ones in 2 transactions
    $q = "
      SELECT *
      FROM `tokens`
      WHERE `uid` = ".$global['db']->qstr($uid)."
      ORDER BY `creation_time` DESC LIMIT ".MAX_TOKENS.", 18446744073709551615 ;
    ";
    $result = $global['db']->Execute($q);
    if($result === false) { daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "rest new token 1 ((".$q."))"); }
    $count = 0;
    while($row = $result->FetchRow()) {
      $count++;
      if($count == 1) {
        $q2 = "
          DELETE 
          FROM `tokens`
          WHERE `sha_512_token` = ".$global['db']->qstr($row['sha_512_token'])."
        ";
      } else { $q2 .= " OR `sha_512_token` = ".$global['db']->qstr($row['sha_512_token']); }
    }
    if($count > 0) {
      $q2 .= " ;";
      $result2 = $global['db']->Execute($q2);
      if($result2 === false) { daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "rest new token 2 ((".$q2."))"); }
    }
  }
  $m = microtime();
  $sha_512_token = hash('sha512', $m, false);
  $token = $global['db']->qstr($sha_512_token);
  $gid = $global['db']->qstr($gid);
  if($uid == null) { $uid = "NULL"; }
  else             { $uid = $global['db']->qstr($uid); }
  if($useThisExpiry != null) { $expiry = "'".$useThisExpiry."'"; }
  elseif(MAX_ALIVE > 0)      { $expiry = "'".date("Y-m-d G:i:s", time()+MAX_ALIVE)."'"; }
  else                       { $expiry = "NULL"; }
  $q = "
    INSERT INTO `tokens` (
        `sha_512_token`,
        `uid`,
        `gid`,
        `expiry`
    ) VALUES (
        $token,
        $uid,
        $gid,
        ".$expiry."
    ) ON DUPLICATE KEY UPDATE `expiry` = ".$expiry.";
  ";
  $result = $global['db']->Execute($q);
  if($result === false) { daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "rest new token 3 ((".$q."))"); }
  return($sha_512_token);
}

// provide event data
function rest_getEvents($gid) {
  global $global, $conf;
  // build where clause for group access
  switch((int)$gid) {
    // admin can access all events
    case 1:
      $where = "WHERE `incident_id` > 0 ";
      break;
    // registered and anonymous users can only see public events
    case 2:
    case 3:
      $where = "WHERE `private_group` IS NULL AND `unlisted` = 0 ";
      break;
    default:
      die();
  }
  // 0 = all for user // 1 = all non-archived for user
  $r = array();
  $q = "
    SELECT *,
    (
      SELECT count(*)
      FROM person_uuid p
      WHERE p.incident_id = i.incident_id
      AND (expiry_date IS NULL OR expiry_date > CURDATE())
    ) AS unexpired,
    (
      SELECT count(*)
      FROM person_uuid p
      WHERE p.incident_id = i.incident_id
      AND expiry_date < CURDATE()
    ) AS expired,
    (
      SELECT count(*)
      FROM person_uuid p
      WHERE p.incident_id = i.incident_id
    ) AS originated
    FROM incident i
    ".$where."
    ORDER by `date` DESC ;
  ";
  $result = $global['db']->Execute($q);
  if($result === false) { daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "REST articles ((".$q."))"); }
  while($row = $result->FetchRow()) {
    $names    = array();
    $articles = array();
    $captions = array();
    $tagss    = array();
    foreach($conf['i18n_on'] as $locale) {
      $translations = json_decode($row['translations']);
      $lang = property_exists($translations, $locale) ? $locale : 'en';
      // cleanup
      $name    = $translations->$lang->name;
      $article = $translations->$lang->article;
      $caption = $translations->$lang->caption;
      $tags    = $translations->$lang->tags;
      // escape quotes
      $name    = str_replace('"', '&quot;', $name);
      $article = str_replace('"', '&quot;', $article);
      $caption = str_replace('"', '&quot;', $caption);
      $tags    = str_replace('"', '&quot;', $tags);
      // escape apostraphes
      $name    = str_replace("'", '&#39;', $name);
      $article = str_replace("'", '&#39;', $article);
      $caption = str_replace("'", '&#39;', $caption);
      $tags    = str_replace("'", '&#39;', $tags);
      // add to array
      $names[$lang]    = (string)$name;
      $articles[$lang] = (string)$article;
      $captions[$lang] = (string)$caption;
      $tagss[$lang]    = (string)$tags;
    }
    // build array
    $s = array(
      'archived'   => (boolean)$row['archived'],
      'articles'   => $articles,
      'captions'   => $captions,
      'closed'     => (boolean)$row['closed'],
      'date'       => (string)$row['date'],
      'default'    => (boolean)$row['default'],
      'expired'    => (int)$row['expired'],
      'group'      => (int)$row['private_group'],
      'id'         => (int)$row['incident_id'],
      'image'      => (string)$row['images'],
      'latitude'   => (float)$row['latitude'],
      'longitude'  => (float)$row['longitude'],
      'names'      => $names,
      'originated' => (int)$row['originated'],
      'short'      => (string)$row['shortname'],
      'tags'       => $tagss,
      'type'       => (string)$row['type'],
      'unexpired'  => (int)$row['unexpired'],
      'unlisted'   => (boolean)$row['unlisted'],
      'updated'    => (string)$row['updated']
    );
    $r[] = $s;
  }
  return($r);
}

// can gid access a record
function rest_gidRecord($gid, $uuid) {
  global $global, $conf;
  // admin
  if((int)$gid === 1) { return true; }
  // anon && user
  $q = "
    SELECT *
    FROM   `incident` i, `person_uuid` p
    WHERE  p.p_uuid = ".$global['db']->qstr($uuid)."
    AND    p.incident_id = i.incident_id
    LIMIT  1;
  ";
  $result = $global['db']->Execute($q);
  if($result === false) { daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "rest gid record : ".$q); }
  if($result == null || $result->EOF) { return false; }
  // admin event
  if((int)$result->fields['private_group'] === 1) { return false; }
  // public event
  return true;
}

// changes user account status to active
function rest_activateUser($z) {
  global $global, $conf;
  $q = "
    UPDATE `users`
    SET    `status` = 'active',
           `confirmation` = NULL
    WHERE  `confirmation` = ".$global['db']->qstr($z).";
  ";
  $r = $global['db']->Execute($q);
  if($r === false) { daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "rest activate user ((".$q."))"); }
}

// send a localized record update to followers via push and/or email
function rest_sendRevisionMessage($uuid) {
  global $global, $conf;
  // push message
  $q = "
    SELECT *
    FROM   following f, users u, push p
    WHERE  f.p_uuid = ".$global['db']->qstr($uuid)."
    AND    f.uid = u.uid
    AND    p.uid = u.uid
    AND    u.record_push = 1;
  ";
  $result = $global['db']->Execute($q);
  if($result === false) { daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "rest revision msg((".$q."))"); }
  while(!$result == NULL && !$result->EOF) {
    $ids   = array();
    $ids[] = $result->fields['registration_id'];
    $goto  = makeRecordUrl($uuid, $result->fields['locale']);
    $obj   = json_decode(file_get_contents($conf['approot'].'www/assets/locales/'.$result->fields['locale'].'.json'));
    $msg   = $obj->emailRecordRevisionSubject->message;
    taupo_message_send($msg, $ids, $goto);
    $result->MoveNext();
  }
  // email message
  $q = "
    SELECT *
    FROM   following f, users u
    WHERE  f.p_uuid = ".$global['db']->qstr($uuid)."
    AND    f.uid = u.uid
    AND    u.record_email = 1;
  ";
  $result = $global['db']->Execute($q);
  if($result === false) { daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "msg admins 2 ((".$q."))"); }
  while(!$result == NULL && !$result->EOF) {
    $e   = new email();
    $obj = json_decode(file_get_contents($conf['approot'].'www/assets/locales/'.$result->fields['locale'].'.json'));
    $subject  = $conf['site_name'].' :: '.$obj->emailRecordRevisionSubject->message;
    $bodyHTML =
      $obj->emailRecordRevisionLine1->message.'<br>'.
      '<br>'.
      $obj->emailRecordRevisionLine2->message.' <a href="'.makeRecordUrl($uuid, $result->fields['locale']).'">'.makeRecordUrl($uuid, $result->fields['locale']).'</a><br>'.
      '<br>'.
      $obj->emailRecordRevisionLine3->message.'<br>';
    $bodyAlt =
      $obj->emailRecordRevisionLine1->message.'\n'.
      '\n'.
      $obj->emailRecordRevisionLine2->message.' '.makeRecordUrl($uuid, $result->fields['locale']).'\n'.
      '\n'.
      $obj->emailRecordRevisionLine3->message.'\n';
    $e->sendMessage($result->fields['user'], $subject, $bodyHTML, $bodyAlt, 'no-reply@host');
    $result->MoveNext();
  }
}

// email new user
function rest_emailNewUser($user, $confirmation, $locale) {
  global $global, $conf;
  $email   = new email();
  $url     = $conf['base_url'].'/'.$locale."/pages/confirm?z=".$confirmation;
  $obj     = json_decode(file_get_contents($conf['approot'].'www/assets/locales/'.$locale.'.json'));
  $subject = $conf['site_name'].' :: '.$obj->emailNewUserSubject->message;
  $bodyHTML =
    $obj->emailNewUserLine1->message.'<br>'.
    '<br>'.
    $obj->emailNewUserLine2->message.'<br>'.
    '<a href="'.$url.'">'.$url.'</a><br>'.
    '<br>'.
    $obj->emailNewUserLine3->message.'<br>';
  $bodyAlt =
    $obj->emailNewUserLine1->message.'\n'.
    '\n'.
    $obj->emailNewUserLine2->message.'\n'.
    $url.'\n'.
    '\n'.
    $obj->emailNewUserLine3->message.'\n';
  $email->sendMessage($user, $subject, $bodyHTML, $bodyAlt);
}

// email a link to reset pass
function rest_emailForgotOne($user, $confirmation, $locale) {
  global $global, $conf;
  $url   = $conf['base_url'].'/'.$locale."/pages/reset?z=".$confirmation;
  $email = new email();
  $obj   = json_decode(file_get_contents($conf['approot'].'www/assets/locales/'.$locale.'.json'));
  $subject = $conf['site_name'].' :: '.$obj->emailForgot1Subject->message;
  $bodyHTML =
    $obj->emailForgot1Line1->message.'<br>'.
    '<br>'.
    $obj->emailForgot1Line2->message.'<br>'.
    '<br>'.
    $obj->emailForgot1Line3->message.'<br>'.
    '<a href="'.$url.'">'.$url.'</a><br>'.
    '<br>'.
    $obj->emailForgot1Line4->message.'<br>';
  $bodyAlt =
    $obj->emailForgot1Line1->message.'\n'.
    '\n'.
    $obj->emailForgot1Line2->message.'\n'.
    '\n'.
    $obj->emailForgot1Line3->message.'\n'.
    $url.'\n'.
    '\n'.
    $obj->emailForgot1Line4->message.'\n';
  $email->sendMessage($user, $subject, $bodyHTML, $bodyAlt);
}

// email new password to the user
function rest_emailForgotTwo($user, $locale, $newPass) {
  global $global, $conf;
  $url   = $conf['base_url'].'/'.$locale;
  $email = new email();
  $obj   = json_decode(file_get_contents($conf['approot'].'www/assets/locales/'.$locale.'.json'));
  $subject = $conf['site_name'].' :: '.$obj->emailForgot2Subject->message;
  $bodyHTML =
    $obj->emailForgot2Line1->message.' <b>'.$newPass.'</b><br>'.
    '<br>'.
    $obj->emailForgot2Line2->message.' <a href="'.$url.'">'.$url.'</a><br>';
  $bodyAlt =
    $obj->emailForgot2Line1->message.' '.$newPass.'\n'.
    '\n'.
    $obj->emailForgot2Line2->message.' '.$url.'\n';
  $email->sendMessage($user, $subject, $bodyHTML, $bodyAlt);
}
