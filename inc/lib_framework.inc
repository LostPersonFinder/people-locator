<?
/**
 * @name     Taupo Framework Library
 * @author   pl@miernicki.com
 * @about    Developed by the U.S. National Library of Medicine
 * @link     https://gitlab.com/tehk/people-locator
 * @license  https://gitlab.com/tehk/people-locator/blob/master/LICENSE
 */

// clean post and get
function cleanPostGet() {
  global $conf;
  $config = HTMLPurifier_Config::createDefault();
  $config->set('Core.Encoding', 'UTF-8');
  $config->set('HTML.Doctype', 'XHTML 1.0 Transitional');
  $config->set('Cache.SerializerPath', $conf['approot'].'www/tmp/htmlpurifier');
  $purifier = new HTMLPurifier($config);
  foreach($_POST as $key=>$val) {
    if(!is_array($_POST[$key])) {
      $val = $purifier->purify($val);
      $_POST[$key] = $val;
    }
  }
  foreach($_GET as $key=>$val) {
    if(!is_array($_GET[$key])) {
      $val = $purifier->purify($val);
      $_GET[$key] = $val;
    }
  }
}

// create unique identifier
function taupo_create_uuid($type='person') {
  global $conf, $global;
  switch ($type){
    case 'record':
      $gen_id = $conf['base_uuid'].'record.'.taupo_gen_id('person');
      break;
    case 'pfif_note':
      $gen_id = $conf['base_uuid'].'note.'.taupo_gen_id('pfif_note');
      break;
    default :
      $gen_id = null;
      break;
  }
  return $gen_id;
}

// gen id from storage engine
function taupo_gen_id($type='person') {
  global $global;
  $type .= "_seq";
  //$global['db']->CreateSequence($type, 1);
  //error_log('new uuid >> '.$global['db']->GenID($type));
  return $global['db']->GenID($type);
}

// validate e-address
function validateAddress($address) {
  $isValid = true;
  $atIndex = strrpos($address, "@");
  if(is_bool($atIndex) && !$atIndex) { $isValid = false; }
  else {
    $domain    = substr($address, $atIndex+1);
    $local     = substr($address, 0, $atIndex);
    $localLen  = strlen($local);
    $domainLen = strlen($domain);
    if($localLen < 1 || $localLen > 64) { $isValid = false; } // local part length exceeded
    else if($domainLen < 1 || $domainLen > 255) {$isValid = false; } // domain part length exceeded
    else if($local[0] == '.' || $local[$localLen-1] == '.') { $isValid = false; } // local part starts or ends with '.'
    else if(preg_match('/\\.\\./', $local)) { $isValid = false; } // local part has two consecutive dots
    else if(!preg_match('/^[A-Za-z0-9\\-\\.]+$/', $domain)) { $isValid = false; } // character not valid in domain part
    else if(preg_match('/\\.\\./', $domain)) { $isValid = false; } // domain part has two consecutive dots
    else if(!preg_match('/^(\\\\.|[A-Za-z0-9!#%&`_=\\/$\'*+?^{}|~.-])+$/', str_replace("\\\\","",$local))) {
      if (!preg_match('/^"(\\\\"|[^"])+"$/', str_replace("\\\\","",$local))) { $isValid = false; } // local part is quoted 
    }
    if($isValid && !(checkdnsrr($domain,"MX") || checkdnsrr($domain,"A"))) { $isValid = false; } // domain not found in DNS
  }
  return $isValid;
}

// Function to get the client IP address
function get_client_ip() {
  $ipaddress = '';
  if(getenv('HTTP_CLIENT_IP')) $ipaddress = getenv('HTTP_CLIENT_IP');
  elseif(getenv('HTTP_X_FORWARDED_FOR')) $ipaddress = getenv('HTTP_X_FORWARDED_FOR');
  elseif(getenv('HTTP_X_FORWARDED')) $ipaddress = getenv('HTTP_X_FORWARDED');
  elseif(getenv('HTTP_FORWARDED_FOR')) $ipaddress = getenv('HTTP_FORWARDED_FOR');
  elseif(getenv('HTTP_FORWARDED')) $ipaddress = getenv('HTTP_FORWARDED');
  elseif(getenv('REMOTE_ADDR')) $ipaddress = getenv('REMOTE_ADDR');
  else { $ipaddress = '255.255.255.255'; }
  // remove all invalid characters
  $chars = "[^0123456789abcdef.:]";
  $pattern = "/[^".preg_quote($chars, "/")."]/";  
  $ipaddress = preg_replace($pattern, '', $ipaddress);
  if(inet_pton($ipaddress) !== false) { return $ipaddress; }
  else { return $ipaddress.'--INVALID'; }
}

// validate ip ~ returns false on invalid ip
function validateIP($ip) {
  return inet_pton($ipaddress) !== false;
}

// generate microtime string
function getMicrotimeComponent() {
  $z = microtime();
  $z = explode(" ", $z);
  $z = explode(".", $z[0]);
  return $z[1];
}

// generate a random string, using a cryptographically secure pseudorandom number generator
function random_str($length, $keyspace = '0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ') {
  $str = '';
  $max = mb_strlen($keyspace, '8bit') - 1;
  for ($i = 0; $i < $length; ++$i) { $str .= $keyspace[random_int(0, $max)]; }
  return $str;
}

// generate salt for // used for pass storage
function generateSalt() {
  $salt = hash('sha512', openssl_random_pseudo_bytes(512), false);
  return $salt;
}

// generate new session key
function genkey($sess_id){
  $salt = generateSalt();
  return md5(trim($salt.$sess_id));
}

// php script detection
function isCommandLineInterface() {
  return (php_sapi_name() === 'cli');
}

// update the arrival rate table...
function updateArrivalRate($person_uuid, $incident_id, $triagepic, $reunite, $website, $pfif, $vanilla_email) {
  global $global;
  $db = $global['db'];
  $b = "";
  $c = "";
  $d = "";
  $e = "";
  $f = "";
  if($triagepic) { $b="+1"; }
  if($reunite) { $c="+1"; }
  if($website) { $d="+1"; }
  if($pfif) { $e="+1"; }
  if($vanilla_email) { $f="+1"; }
  $db->StartTrans();
  $q = "
      SELECT @A:=source_all, @B:=source_triagepic, @C:=source_reunite, @D:=source_website, @E:=source_pfif, @F:=source_vanilla_email
      FROM arrival_rate 
      WHERE incident_id = '".$incident_id."'
      ORDER BY arrival_time DESC
      LIMIT 1;
  ";
  $result = $db->Execute($q);
  if($result === false) { daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $db->ErrorMsg(), $q); }
  $q = "  
      INSERT INTO arrival_rate (person_uuid, incident_id, arrival_time, source_all, source_triagepic, source_reunite, source_website, source_pfif, source_vanilla_email)
      VALUES (".$global['db']->qstr($person_uuid).", ".$global['db']->qstr((int)$incident_id).", '".date('Y-m-d H:i:s')."', @A+1, @B".$b.", @C".$c.", @D".$d.", @E".$e.", @F".$f.");
  ";
  $result = $db->Execute($q);
  if($result === false) { daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $db->ErrorMsg(), $q); }
  $db->HasFailedTrans() ? daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $db->ErrorMsg(), 'TRANSACTION FAILURE ARRIVAL RATE.') : null;
  $db->CompleteTrans();
}

// generates a url for a record
function makeRecordUrl($uuid, $locale = 'en') {
  global $global, $conf;
  $q = "
    SELECT *
    FROM   person_uuid p, incident i
    WHERE  p.p_uuid = ".$global['db']->qstr($uuid)."
    AND    p.incident_id = i.incident_id ;
  ";
  $result = $global['db']->Execute($q);
  if($result === false) { daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "lib_framework makeRecordUrl ((".$q."))"); }
  if($result != NULL && !$result->EOF) {
    $num = explode(".", (string)$uuid);
    return $conf['base_url'].'/'.$locale.'/'.$result->fields['shortname'].'/record#'.$num[sizeof($num)-1].'/view';
  } else { return 'invalid'; }
}

// determine client platform
function getPlatformSource() {
  global $global, $conf;
  require_once($conf['approot'].'3rd/mobile_detect/Mobile-Detect/Mobile_Detect.php');
  $detect     = new Mobile_Detect;
  $deviceType = ($detect->isMobile()    ? ($detect->isTablet() ? 'tablet' : 'phone') : 'computer');
  $ios        = ($detect->isiOS()       ? true: false );
  $android    = ($detect->isAndroidOS() ? true: false );
  if(array_key_exists('HTTP_USER_AGENT', $_SERVER)) { $hua = $_SERVER['HTTP_USER_AGENT']; }
  else { $hua = 'UNDEFINED'; }
  $darwin = stripos($hua, 'Darwin');
  $dalvik = stripos($hua, 'Dalvik');
  if($darwin === false) { $darwin = 9999; }
  else { $ios = true; }
  if($dalvik === false) { $dalvik = 9999; }
  if($dalvik < 9999 || $darwin < 9999) { $ru = 1; }
  else { $ru = 0; }
  if($ru === 1) {
    if($ios) { $source = REUNITE_IOS_SOURCE; }
    elseif($android) { $source = REUNITE_ANDROID_SOURCE; }
    else { $source = UNKNOWN_SOURCE; }
  } else { $source = WEBSITE_SOURCE; }
  return $source;
}
