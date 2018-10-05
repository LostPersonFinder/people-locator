<?
/**
 * @name     Access Control List
 * @author   pl@miernicki.com
 * @about    Developed by the U.S. National Library of Medicine
 * @link     https://gitlab.com/tehk/people-locator
 * @license  https://gitlab.com/tehk/people-locator/blob/master/LICENSE
 */

// audit log
function taupo_acl_log($type, $notes) {
  global $global, $conf;
  $q = "
    INSERT INTO `audit` (type, ip, notes)
    VALUES(".$global['db']->qstr($type).", ".$global['db']->qstr(get_client_ip()).", ".$global['db']->qstr($notes).");
  ";
  $res = $global['db']->Execute($q);
  if($res === false) { daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), 'lib acl audit ('.$q.')'); }
}

// check the existence of an user
function taupo_is_user($user) {
  global $global;
  $q = "
    SELECT *
    FROM `users`
    WHERE user = ".$global['db']->qstr($user).";
  ";
  $res = $global['db']->Execute($q);
  if(($res == null) || ($res->EOF)) {
    return false;
  } else {
    return true;
  }
}

// adds a new user
function taupo_add_user($user, $pass, $status='pending', $locale='en') {
  global $global, $conf;
  if($user == null) {
    return false;
  }
  if(taupo_is_user($user)) {
    return false;
  }
  // create pass hash
  $salt   = generateSalt();
  $pass   = substr($pass, 0, 4).$salt.substr($pass, 4);
  $digest = md5(trim($pass));
  $time   = time();
  $confirmation = hash('sha256', openssl_random_pseudo_bytes(256)); // code used to activate account
  // insert user
  $q = "
    INSERT INTO `users` (user, pass, salt, changed, status, confirmation, locale)
    values(
      ".$global['db']->qstr($user).",
      ".$global['db']->qstr($digest).",
      ".$global['db']->qstr($salt).",
      ".$time.",
      ".$global['db']->qstr($status).",
      ".$global['db']->qstr($confirmation).",
      ".$global['db']->qstr($locale)."
    );
  ";
  $res = $global['db']->Execute($q);
  if($res === false) { daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "add user 02 ((".$q."))"); }
  taupo_acl_log('NEW_USER', 'new user registered: '.$user);
  return true;
}

// change user pass
function taupo_change_pass($user, $new) {
  global $global;
  date_default_timezone_set('America/New_York');
  $time   = time();
  $salt   = generateSalt();
  $new2   = substr($new, 0, 4).$salt.substr($new, 4);
  $digest = md5(trim($new2));
  $q = "
    UPDATE `users`
    SET 
      `pass`     = ".$global['db']->qstr($digest).",
      `salt`     = ".$global['db']->qstr($salt).",
      `changed`  = ".$global['db']->qstr($time)."
    WHERE `user` = ".$global['db']->qstr($user).";
  ";
  $res = $global['db']->Execute($q);
  if($res === false) { daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "change pass ((".$q."))"); }
}

// check pass matches policy
function taupo_pass_policy($pass, $user) {
  global $global, $conf;
  if(isset($pass)){
    if(strlen($pass) < PASS_MIN_CHARS) {
      return LESS_MIN_CHARS;
    }
    if(strlen($pass) > PASS_MAX_CHARS) {
      return MORE_MAX_CHARS;
    }
    if(PASS_HAS_UPPER) {
      if(!preg_match('/[A-Z]/', $pass)) {
        return REQ_UPPERCASE;
      }
    }
    if(PASS_HAS_LOWER) {
      if(!preg_match('/[a-z]/', $pass)) {
        return REQ_LOWERCASE;
      }
    }
    if(PASS_HAS_DIGIT) {
      if(!preg_match('/[0-9]/', $pass)) {
        return REQ_NUMBERS;
      }
    }
    if(PASS_HAS_SPECL) {
      $sp_chars_str = "!@,.#$%\^&\*/?`~+=|:;()-\"'[]{}<>";
      $has_sp_char = false;
      for($i=0; $i < strlen($sp_chars_str); $i++) {
        $sp_chr = substr($sp_chars_str,$i,1);
        if(stripos($pass,$sp_chr) !== FALSE) {
          $has_sp_char = true;
          break;
        }
      }
      if($has_sp_char == false) {
        return REQ_SPECIAL_CHARS;
      }
    }
    if(PASS_HAS_USRNM) {
      if(stripos($pass,$user) !== FALSE){
        return HAS_USER;
      }
    }
    // nothing wrong if control comes here.
    return true;
  } else {
    // no pass to match
    return false;
  }
}

// get error message for failed pass policy match
function taupo_pass_policy_message($error_code) {
  global $conf;
  $msg = "";
  switch($error_code) {
    case LESS_MIN_CHARS:
      $msg = sprintf("The minimum pass length is %d",PASS_MIN_CHARS);
      break;
    case MORE_MAX_CHARS:
      $msg = sprintf("The maximum pass length is %d.",PASS_MAX_CHARS);
      break;
    case REQ_UPPERCASE:
      $msg = "The pass should have at least one uppercase character.";
      break;
    case REQ_LOWERCASE:
      $msg = "The pass should have at least one lowercase character.";
      break;
    case REQ_NUMBERS:
      $msg = "The pass should have at least one numeric character."  ;
      break;
    case REQ_SPECIAL_CHARS:
      $msg = "The pass should have at least one special character. eg. !,@.#$%\^&\*/?`~+=|:;-\"'";
      break;
    case HAS_USERNAME:
      $msg = "The user cannot be a part of the pass.";
      break;
  }
  return $msg;
}

// gen random pass
function randomPass() {
  $alphabet = "abcdefghijklmnopqrstuwxyzABCDEFGHIJKLMNOPQRSTUWXYZ0123456789";
  $pass = array();
  $alphaLength = strlen($alphabet) - 1;
  for($i = 0; $i < 12; $i++) {
    $n = rand(0, $alphaLength);
    $pass[] = $alphabet[$n];
  }
  return implode($pass);
}
