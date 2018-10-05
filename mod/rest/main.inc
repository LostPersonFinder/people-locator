<?
/**
 * @name     REST services
 * @author   pl@miernicki.com
 * @about    Developed by the U.S. National Library of Medicine
 * @link     https://gitlab.com/tehk/people-locator
 * @license  https://gitlab.com/tehk/people-locator/blob/master/LICENSE
 */

// default
function taupo_rest_default() {
  http_response_code(404);
  echo '404';
  die(); 
}

// rest endpoint controller
function rest_endpoint() {
  global $global, $conf;
  $do_log = true;
  error_reporting(0);
  require_once($conf['approot'].'mod/rest/functions.inc');
  ob_start("ob_gzhandler"); // enable gzip compression
  header('Content-Type: application/json'); // json response
  // get / set defaults
  $raw = file_get_contents('php://input');
  $global['jsin'] = json_decode($raw, true);
  if(!isset($global['jsin']['call']))  { (string)$global['jsin']['call']  = null; }
  if(!isset($global['jsin']['user']))  { (string)$global['jsin']['user']  = null; }
  if(!isset($global['jsin']['token'])) { (string)$global['jsin']['token'] = null; }
  // debug cache in
  if(isset($conf['debug_cache']) && $conf['debug_cache'] == true && (string)$global['jsin']['call'] !== 'ping') {
    $path = $conf['approot'].'www/tmp/debug_cache'.'/'.date('Ymd_His.'.getMicrotimeComponent())."_".get_client_ip().'_REST_'.$global['jsin']['call'].'_';
    ob_start();
    file_put_contents($path."IN.json", $raw);
    chmod($path."IN.json", 0777);
  }
  // init out
  $global['jsout'] = array('error'=>(int)0);
  // branch based on call
  switch($global['jsin']['call']) {
    case 'report':
      rest_report();
      break;
    case 'revise':
      rest_revise();
      break;
    case 'search':
      rest_search();
      break;
    case 'autocomplete':
      rest_autocomplete();
      break;
    case 'user':
      rest_user();
      break;
    case 'anon':
      rest_anon();
      break;
    case 'register':
      rest_register();
      break;
    case 'events':
      rest_events();
      break;
    case 'abuse':
      rest_abuse();
      break;
    case 'delete':
      rest_delete();
      break;
    case 'logout':
      rest_logout();
      break;
    case 'forgot':
      rest_forgot();
      break;
    case 'reset':
      rest_reset();
      break;
    case 'change':
      rest_change();
      break;
    case 'confirm':
      rest_confirm();
      break;
    case 'following':
      rest_following();
      break;
    case 'follow':
      rest_follow();
      break;
    case 'push':
      rest_push();
      break;
    case 'comment':
      rest_comment();
      break;
    case 'ping':
      rest_ping();
      $do_log = false;
      break;
    case 'pref':
      rest_pref();
      break;
    case 'messages':
      rest_messages();
      break;
    default:
      fail2ban();
      http_response_code(404);
      die();
  }
  // out
  echo json_encode($global['jsout']);
  // debug cache out
  if(isset($conf['debug_cache']) && $conf['debug_cache'] == true && (string)$global['jsin']['call'] !== 'ping') {
    file_put_contents($path."OUT.json", ob_get_contents());
    chmod($path."OUT.json", 0777);
  }
  ob_end_flush();
  // log
  if($do_log){ rest_log($global['jsin']['call'], $global['jsin']['user'], $global['jsin']['token']); }
}

// max string length
function substrG($string, $length) {
  return (strlen($string) > $length) ? substr($string, 0, $length-3).'...' : $string;
}

// undocumented rest function for js error reporting
function taupo_rest_error() {
  global $global, $conf;
  // headers
  ob_start("ob_gzhandler"); // enable gzip compression
  header('Content-Type: application/json'); // json response
  // only log on non dev instances
  if(1) { //$conf['dev_mode'] == false) {
    $os      = isset($_GET['os'])      ? substrG((string)base64_decode($_GET['os']),      32)    : '???';
    $browser = isset($_GET['browser']) ? substrG((string)base64_decode($_GET['browser']), 32)    : '???';
    $message = isset($_GET['message']) ? substrG((string)base64_decode($_GET['message']), 65535) : '???';
    $file    = isset($_GET['file'])    ? substrG((string)base64_decode($_GET['file']),    255)   : '???';
    $line    = isset($_GET['line'])    ? substrG((string)$_GET['line'],                   8)     : '???';
    $column  = isset($_GET['column'])  ? substrG((string)$_GET['column'],                 8)     : '???';
    $stack   = isset($_GET['stack'])   ? substrG((string)base64_decode($_GET['stack']),   65535) : '???';
    $uid     = isset($_GET['uid'])     ? (int)$_GET['uid']                                       : '???';
    $gid     = isset($_GET['gid'])     ? (int)$_GET['gid']                                       : '???';
    $ip      = get_client_ip();
    $lj      = isset($_GET['lj'])      ? substrG((string)base64_decode($_GET['lj']),      65535) : '';
    // deeper browser info
    $be  = explode(' ', $browser);
    $bn  = $be[0]; // browser name
    $bvs = $be[1]; // $browser version string
    $bve = explode('.', $bvs);
    $bv  = (int)$bve[0]; // major browser version #
    // skip logging these cases
    if( ($browser == '???') ||
        ($os      == '???') ||
        ($message == '???') ||
        ($file    == '???') ||
        ($stack   == '???') ||
        ($line    == '???') ||
        ($column  == '???') ||
        ($uid     == '???') ||
        ($uid     == 'undefined') ||
        ($gid     == 'undefined') ||
        ($gid     == '???') ||
        ($ip      == '???') ||
        (stripos($ip, '199.30.24.')  !== false) || // skip bing bot
        (stripos($ip, '199.30.25.')  !== false) || // skip bing bot
        (stripos($ip, '131.253.') !== false) || // skip bing bot
        (stripos($ip, '125.209.235') !== false) || // skip north korea
        (stripos($bn, 'Chrome')  !== false && $bv < 65) || // skip old chrome
        (stripos($bn, 'Firefox') !== false && $bv < 59) || // skip old firefox
        (stripos($bn, 'Edge')    !== false && $bv < 16) || // skip old edge
        (stripos($browser, 'WebKit 601.1.46') !== false && stripos($os, 'iOS 9') !== false) || // skip safari 9
        (stripos($browser, 'IE') !== false) || // skip ie
        (stripos($browser, 'Facebook') !== false) || // skip fb
        (trim($lj) == '' )) { // skip browsers that don't support href.location
      $r = array('msg'=>'NOT_LOGGED');
      /*
        (stripos($message, 'ReferenceError: Can\'t find variable: fetch') !== false) ||
        (stripos($message, 'Uncaught ReferenceError: fetch is not defined') !== false) ||
      */
    // log everything else
    } else {
      $q = "
        INSERT INTO onerror (
          `os`,
          `browser`,
          `message`,
          `file`,
          `line`,
          `column`,
          `stack`,
          `uid`,
          `gid`,
          `ip`,
          `location`)
        VALUES (
          ".$global['db']->qstr((string)$os     ).",
          ".$global['db']->qstr((string)$browser).",
          ".$global['db']->qstr((string)$message).",
          ".$global['db']->qstr((string)$file   ).",
          ".$global['db']->qstr((string)$line   ).",
          ".$global['db']->qstr((string)$column ).",
          ".$global['db']->qstr((string)$stack  ).",
          ".$global['db']->qstr((string)$uid    ).",
          ".$global['db']->qstr((string)$gid    ).",
          ".$global['db']->qstr((string)$ip     ).",
          ".$global['db']->qstr((string)$lj  ).");
      ";
      $result = $global['db']->Execute($q);
      if($result === false) { daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "rest_error"); }
      $r = array('msg'=>'LOGGED');
    }
  } else {
    $r = array('msg'=>'NOT_LOGGED');
  }
  $rj = json_encode($r);
  echo $rj;
  ob_end_flush();
}
