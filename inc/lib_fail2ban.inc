<?
/**
 * @name     auth FAIL library
 * @author   pl@miernicki.com
 * @about    Developed by the U.S. National Library of Medicine
 * @link     https://gitlab.com/tehk/people-locator
 * @license  https://gitlab.com/tehk/people-locator/blob/master/LICENSE
 */

// check if banned
function fail2banCheck() {
  global $global, $conf;
  if(isset($conf['fail2ban_on']) && ($conf['fail2ban_on'] === true) && is_banned(get_client_ip())) {
    http_response_code(403); // forbidden
    // Bigger font for mobile devices.
    echo '<style>
          h1 {font-size: 2em;}
          @media only screen and (min-device-width:480px) {
            h1 {font-size: .9em;}
          }
          </style>
          <body>
          <h1>TEMPORARY BAN DUE TO MANY FAILED AUTHENTICATION ATTEMPTS; COME BACK LATER...</h1>
          </body>';
    die();
  }
}

// ip ban check
function is_banned($ip) {
  global $global, $conf;
  $cacheip = $global['cache']->getItem($ip);
  // not in cache at all
  if(is_null($cacheip->get())) {
    return false;
  // check if beyond threshold
  } elseif((int)($cacheip->get()) >= MAX_AUTH_FAILURES) {
    return true;
  } else {
    return false;
  }
}

// incriminating ip incremented
function fail2ban() {
  global $global, $conf;
  $ip  = (string)get_client_ip();
  $cacheip = $global['cache']->getItem($ip);
  // count is one for new ip
  if(is_null($cacheip->get())) {
    // add new ip to cache
    $cacheip->set(1)->expiresAfter(BAN_SECONDS)->addTag('fail2ban');
    $count = 1;
  // increment existing ip
  } else {
    $cacheip->increment();
    $count = $cacheip->get();
  }
  $global['cache']->save($cacheip);
  $dto = $cacheip->getExpirationDate();
  if($count >= MAX_AUTH_FAILURES && isset($conf['fail2ban_on']) && $conf['fail2ban_on'] == true) {
    taupo_acl_log('FAIL2BAN_BAN', 'banned; expires: '.$dto->format('Y-m-d H:i:s.u'));
  }
  error_log('FAIL2BAN; ip:'.$ip.' count:'.$count.' max:'.MAX_AUTH_FAILURES.' expires:'.$dto->format('Y-m-d H:i:s.u'));
  return $count;
}

// clear ban cache
function clear_ban_list() {
  global $global, $conf;
  $cacheip = $global['cache']->clear();
  taupo_acl_log('FAIL2BAN_CLEAR', 'user '.$_SESSION['user'].' cleared the fail2ban cache');
}
