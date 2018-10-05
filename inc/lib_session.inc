<?
/**
 * @name     session lib
 * @author   pl@miernicki.com
 * @about    Developed by the U.S. National Library of Medicine
 * @link     https://gitlab.com/tehk/people-locator
 * @license  https://gitlab.com/tehk/people-locator/blob/master/LICENSE
 */

// start user session
function initSession() {
  // remove old tokens
  rest_removeExpiredTokens();
  // no session exists, create one
  if(!isset($_COOKIE['PL_SESSION'])) {
    newSession();
  // validate session
  } else {
    $a = rest_checkToken($_COOKIE['PL_SESSION']);
    // invalid session
    if((int)$a['status'] !== 0) {
      newSession(); 
    // valid session
    } else {
      session_id($_COOKIE['PL_SESSION']);
      session_name('PL_SESSION');
      session_start();
      $_SESSION['uid']   = $a['uid'];
      $_SESSION['gid']   = $a['gid'];
      $_SESSION['user']  = $a['user'];
      $_SESSION['token'] = $_COOKIE['PL_SESSION'];
    }
  }
  // update cookie expiration
  setcookie(session_name(), session_id(), time()+MAX_ALIVE, '/');
}

function newSession() {
  $token = rest_newToken(3, 3, date("Y-m-d G:i:s", time()+MAX_ALIVE_ANON));
  session_id($token);
  session_name('PL_SESSION');
  session_start();
  $_SESSION['uid']   = 3;
  $_SESSION['gid']   = 3;
  $_SESSION['user']  = null;
  $_SESSION['token'] = $token;
}
