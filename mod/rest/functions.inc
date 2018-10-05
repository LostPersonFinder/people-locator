<?
/**
 * @name     REST services
 * @author   pl@miernicki.com
 * @about    Developed by the U.S. National Library of Medicine
 * @link     https://gitlab.com/tehk/people-locator
 * @license  https://gitlab.com/tehk/people-locator/blob/master/LICENSE
 */

// report
function rest_report() {
  global $global, $conf;
  $format = isset($global['jsin']['format']) ? (string)$global['jsin']['format'] : (string)'';
  $short  = isset($global['jsin']['short'])  ? (string)$global['jsin']['short']  : (string)'';
  $notme  = isset($global['jsin']['notme'])  ? true : false;
  $global['jsout']['uuid']  = '';
  // check event auth
  $a = rest_checkEventToken($global['jsin']['token'], $short);
  if((int)$a['status'] !== 0) { $global['jsout']['error'] = (int)$a['status']; return; }
  // create new person/animal
  $p = new person();
  $p->init();
  if($notme) { $p->reporting_user = 3; } // anon
  else       { $p->reporting_user = $a['uid']; }
  $p->setEvent($short);
  if(!$p->isEventOpen()) { $global['jsout']['error'] = (int)8001; return; }
  $p->theString = $global['jsin'];
  $p->payloadFormat = $format;
  $ecode = $p->process(false); // false // not an update
  if((int)$ecode === 0) {
    $p->insert($a['uid'], $a['gid']);
    $global['jsout']['uuid'] = (string)$p->p_uuid;
  } else { $global['jsout']['error'] = (int)$ecode; }
}

// revise
function rest_revise() {
  global $global, $conf;
  $uuid   = isset($global['jsin']['uuid'])   ? (string)$global['jsin']['uuid']   : (string)'';
  $format = isset($global['jsin']['format']) ? (string)$global['jsin']['format'] : (string)'';
  $short  = isset($global['jsin']['short'])  ? (string)$global['jsin']['short']  : (string)'';
  // check event auth
  $a = rest_checkEventToken($global['jsin']['token'], $short);
  if((int)$a['status'] !== 0) { $global['jsout']['error'] = (int)$a['status']; return; }
  // check record exists
  $q = "
    SELECT  count(*)
    FROM   `person_uuid`
    WHERE  `p_uuid` = ".$global['db']->qstr($uuid).";
  ";
  $result = $global['db']->Execute($q);
  if($result === false) { daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "rest_revise ".$q); }
  // record does not exist
  if((int)$result->fields['count(*)'] !== 1) { $global['jsout']['error'] = (int)2100; return; }
  // load record
  $p = new person();
  $p->p_uuid = $uuid;
  $p->load();
  $p->loadExtraData($a['uid'], $a['gid']);
  // revision permission
  if(!$p->canRevise($a['uid'], $a['gid'])) { $global['jsout']['error'] = (int)2100; return; }
  // cannot revise an expired record
  if($p->expiry_date !== null && time() > strtotime($p->expiry_date)) { $global['jsout']['error'] = (int)2300; return; }
  $p->update_uid = $a['uid'];
  $p->setEvent($short);
  if(!$p->isEventOpen()) { $global['jsout']['error'] = (int)8001; return; }
  $p->theString = $global['jsin'];
  $p->payloadFormat = $format;
  $ecode = $p->process(true);
  // cannot revise person to animal or animal to person // PL-1852
  if( // person to animal
      (($p->animal == null || strlen($p->animal) < 2) && $global['jsin']['pa'] == 1) ||
      // animal to person
      (($p->animal !== null || strlen($p->animal) > 1) && $global['jsin']['pa'] == 0)) {
      $global['jsout']['error'] = (int)1006;
  // no processing errors
  } elseif((int)$ecode === 0) { 
    $p->update();
    rest_sendRevisionMessage($uuid);
  } else { $global['jsout']['error'] = (int)$ecode; }
}

// search
function rest_search() {
  global $global, $conf;
  $short = isset($global['jsin']['short']) ? (string)$global['jsin']['short'] : (string)'';
  $query = isset($global['jsin']['query']) ? (string)$global['jsin']['query'] : (string)'';
  $photo = isset($global['jsin']['photo']) ? (string)$global['jsin']['photo'] : (string)'';
  $ecode = (int)0;
  // determine client
  $source = getPlatformSource();
  // check event auth
  $a = rest_checkEventToken($global['jsin']['token'], $short) ;
  if($a['status'] != 0) { $ecode = (int)$a['status']; }
  // check search by image enabled
  elseif($conf['image_search'] !== true && (strlen($photo) > 3)) { $ecode = (int)1100; }
  // fm is down
  elseif(strlen($photo) > 0 && taupo_facematch_status() != 1) { $ecode = (int)1101; }
  // validate filters
  else {
    $searchMode     = $conf['SOLR_on'] == true ? 'solr' : 'sql';
    $pageStart      = !isset($global['jsin']['pageStart'])      ?  (int)0  : (int)$global['jsin']['pageStart'];
    $perPage        = !isset($global['jsin']['perPage'])        ?  (int)50 : (int)$global['jsin']['perPage'];
    $personAnimal   = !isset($global['jsin']['personAnimal'])   ?  (int)2  : (int)$global['jsin']['personAnimal'];
    $since          = !isset($global['jsin']['since'])          ?  null    : (string)$global['jsin']['since'];
    $sortBy         = !isset($global['jsin']['sortBy'])         ?  ''      : (string)$global['jsin']['sortBy'];
    $sImage         = !isset($global['jsin']['hasImage'])       ? 'false'  : (($global['jsin']['hasImage']       == true) ? 'true' : 'false');
    $sexMale        = !isset($global['jsin']['sexMale'])        ? 'false'  : (($global['jsin']['sexMale']        == true) ? 'true' : 'false');
    $sexFemale      = !isset($global['jsin']['sexFemale'])      ? 'false'  : (($global['jsin']['sexFemale']      == true) ? 'true' : 'false');
    $sexOther       = !isset($global['jsin']['sexOther'])       ? 'false'  : (($global['jsin']['sexOther']       == true) ? 'true' : 'false');
    $sexUnknown     = !isset($global['jsin']['sexUnknown'])     ? 'false'  : (($global['jsin']['sexUnknown']     == true) ? 'true' : 'false');
    $ageChild       = !isset($global['jsin']['ageChild'])       ? 'false'  : (($global['jsin']['ageChild']       == true) ? 'true' : 'false');
    $ageAdult       = !isset($global['jsin']['ageAdult'])       ? 'false'  : (($global['jsin']['ageAdult']       == true) ? 'true' : 'false');
    $ageUnknown     = !isset($global['jsin']['ageUnknown'])     ? 'false'  : (($global['jsin']['ageUnknown']     == true) ? 'true' : 'false');
    $statusMissing  = !isset($global['jsin']['statusMissing'])  ? 'false'  : (($global['jsin']['statusMissing']  == true) ? 'true' : 'false');
    $statusAlive    = !isset($global['jsin']['statusAlive'])    ? 'false'  : (($global['jsin']['statusAlive']    == true) ? 'true' : 'false');
    $statusInjured  = !isset($global['jsin']['statusInjured'])  ? 'false'  : (($global['jsin']['statusInjured']  == true) ? 'true' : 'false');
    $statusDeceased = !isset($global['jsin']['statusDeceased']) ? 'false'  : (($global['jsin']['statusDeceased'] == true) ? 'true' : 'false');
    $statusUnknown  = !isset($global['jsin']['statusUnknown'])  ? 'false'  : (($global['jsin']['statusUnknown']  == true) ? 'true' : 'false');
    $statusFound    = !isset($global['jsin']['statusFound'])    ? 'false'  : (($global['jsin']['statusFound']    == true) ? 'true' : 'false');
    // handle search by image
    if(strlen($photo) > 3) {
      // save image to disk for facematch searching
      $photo = base64_decode($photo);
      $filename = 'searchByImage_'.date('Y-md-Hi').'_'.rand(100000,999999);
      $path = $conf['approot']."www/tmp/rest_cache/".$filename; // path is now like /opt/pl/www/tmp/rest_cache/searchByImage_2014-0703-1233_942781
      // save to disk
      file_put_contents($path, $photo);
      // make the file world writeable for other users/applications and to handle deletes
      chmod($path,  0777);
      // get image information from saved file
      try { $info = getimagesize($path); }
      catch (Exception $e) { $ecode = (int)1102; }
      if($ecode == 0) {
        list(,$mime) = explode("/",$info['mime']);
        $mime = strtolower($mime);
        if(stripos($mime,"png") !== FALSE) { $ext = ".png"; }
        elseif(stripos($mime,"gif") !== FALSE) { $ext = ".gif"; }
        elseif(stripos($mime,"jpeg") !== FALSE) { $ext = ".jpg"; }
        else { $ecode = (int)1102; }
      }
      // invalid image
      if($ecode != 0) { rename($path, $path.'_INVALID'); }
      else { 
        rename($path, $path.$ext);
        $filename = $filename.$ext;
        $query = "tmp/rest_cache/".$filename;
      }
      // default sort for image search
      $sortBy = (strlen($sortBy) < 3) ? "similarity DESC" : $sortBy; // lack of explicit sort order defaults to similarity
    } else {
      // default sort order for text search
      if($searchMode == "solr") { $sortBy = (strlen($sortBy) < 3) ? "" : $sortBy; } // use default SOLR ordering
      else { $sortBy = (strlen($sortBy) < 3) ? "last_updated DESC" : $sortBy; } // lack of explicit sort order defaults to last updated
    }
    if($ecode == 0) {
      // formulate search strings
      $sStatus       = $statusMissing.";".$statusAlive.";".$statusInjured.";".$statusDeceased.";".$statusUnknown.";".$statusFound;
      $sSex          = $sexOther.     ";".$sexMale.    ";".$sexFemale.    ";".$sexUnknown;
      $sAge          = $ageChild.     ";".$ageAdult.   ";".$ageUnknown;
      $sPageControls = $pageStart.    ";".$perPage.    ";".$sortBy.       ";true"; // use non-interactive mode ~ last true
      // perform search
      $timeElapsed = 0;
      $search = new SearchDB($searchMode, $short, $query, $sStatus, $sSex, $sAge, $sImage, '', $sPageControls, $source, $since, $personAnimal);
      try {
        $timeStart = microtime(true);
        $search->executeSearch();
        $total = (int)$search->numRowsFound;
        $timeElapsed = microtime(true) - $timeStart;
      } catch (Exception $e) { $ecode = (int)9999; }
    }
  }
  $r = array();
  // package results
  if($ecode == 0) {
    if(count($search->results) > 0) {
      foreach($search->results as $result) {
        $p = new person();
        $p->p_uuid = $result['p_uuid'];
        $p->load();
        $p->loadExtraData($a['uid'], $a['gid']);
        $r[] = $p->makeArrayObject();
      }
    }
    $global['jsout']['results'] = $r;
    $global['jsout']['total']   = (int)$total;
    $global['jsout']['error']   = (int)0;
  // error
  } else {
    $global['jsout']['results'] = $r;
    $global['jsout']['total']   = (int)0;
    $global['jsout']['error']   = (int)$ecode;
  }
}

// get search autocompletes
function rest_autocomplete() {
  global $global, $conf;
  // for dev return test results
  if($conf['SOLR_on'] == false) {
    $names = array();
    $names[] = 'aaaaaaaaaaaaaaaaaaaaa';
    $names[] = 'bbbbbbbbbbbbbbbbbbbbb';
    $names[] = 'ccccccccccccccccccccc';
    $r = array();
    $r = $names;
    $global['jsout']['results'] = $r;
    $global['jsout']['error']   = (int)0;
    return;
  }
  // actual solr implementation
  $short  = isset($global['jsin']['short'])  ? (string)$global['jsin']['short']  : (string)'';
  $prefix = isset($global['jsin']['prefix']) ? (string)$global['jsin']['prefix'] : (string)'';
  $max    = isset($global['jsin']['max'])    ? (string)$global['jsin']['max']    : (string)'';
  $ecode = (int)0;
  // check event auth
  $a = rest_checkEventToken($global['jsin']['token'], $short);
  $fieldName = 'full_name_no_ws';
  if($a['status'] != 0) { $ecode = (int)$a['status']; }
  else {
    $tmpSOLRquery = $conf["SOLR_root"]
                    . "select/?rows=0"
                    . "&q=*:*&fq=shortname:".$short
                    . "&fq=-expiry_date:[*%20TO%20NOW]"
                    . "&facet=true"
                    . "&facet.mincount=1"
                    . "&facet.limit=".$max
                    . "&facet.sort=index"
                    . "&facet.prefix=".urlencode($prefix)
                    . "&facet.field=".$fieldName
                    . "&wt=json";
    $ch = curl_init();
    curl_setopt($ch, CURLOPT_URL, $tmpSOLRquery);
    curl_setopt($ch, CURLOPT_RETURNTRANSFER, 1);
    curl_setopt($ch, CURLOPT_PORT, $conf['SOLR_port']);
    $tempSOLRjson = json_decode(curl_exec($ch));
    curl_close($ch);
    // facet array is of the form [facet, count, facet, count, ...] // we only want the facet names // so we skip every other one
    $facets = &$tempSOLRjson->facet_counts->facet_fields->$fieldName;
    $names = array();
    $i = 0;
    foreach($facets as $item) {
      // if odd, continue
      if($i++&1) continue;
      $names[] = $item;
    }
  }
  $r = array();
  // package results
  if($ecode == 0) {
    if(count($names) > 0) { $r = $names; }
    $global['jsout']['results'] = $r;
    $global['jsout']['error']   = (int)0;
  // error
  } else {
    $global['jsout']['results'] = $r;
    $global['jsout']['error']   = (int)$ecode;
  }
}

// user auth
function rest_user() {
  global $global, $conf;
  $user  = isset($global['jsin']['user']) ? $global['jsin']['user'] : '';
  $pass  = isset($global['jsin']['pass']) ? $global['jsin']['pass'] : '';
  $ecode = (int)0;
  // check auth
  $status = rest_authCheck($user, $pass);
  if($status == 0) {
    $q = "
      SELECT *
      FROM   `users`
      WHERE  `user` = ".$global['db']->qstr((string)$user).";
    ";
    $result = $global['db']->Execute($q);
    if($result === false) { daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "rest_user ".$q); }
    $token = rest_newToken($result->fields["uid"], $result->fields["gid"]);
    $global['jsout']['token'] = (string)$token;
    $global['jsout']['gid']   = (int)$result->fields["gid"];
    $global['jsout']['error'] = (int)0;
    rest_cookie($token, $result->fields["uid"], $result->fields["gid"], $user);
  } else {
    fail2ban();
    $global['jsout']['token'] = (string)'';
    $global['jsout']['gid']   = (int)0;
    $global['jsout']['error'] = (int)$status;
  }
}

// anon auth
function rest_anon() {
  global $global, $conf;
  $token = rest_newToken(3, 3, date("Y-m-d G:i:s", time()+MAX_ALIVE_ANON));
  $global['jsout']['token'] = (string)$token;
  $global['jsout']['error'] = (int)0;
  rest_cookie($token, 3, 3, null);
}

// register user
function rest_register() {
  global $global, $conf;
  $email    = isset($global['jsin']['email'])    ? $global['jsin']['email']    : '';
  $pass     = isset($global['jsin']['pass'])     ? $global['jsin']['pass']     : '';
  $captcha  = isset($global['jsin']['captcha'])  ? $global['jsin']['captcha']  : '';
  $inactive = isset($global['jsin']['inactive']) ? $global['jsin']['inactive'] : false;
  $locale   = isset($global['jsin']['locale'])   ? $global['jsin']['locale']   : 'en';
  $ecode   = (int)0;
  $token2  = (string)'';
  // invalid token
  if(!rest_validToken($global['jsin']['token'])) { $global['jsout']['error'] = (int)9000; return; }
  // email validation
  if(!validateAddress($email)) { $global['jsout']['error'] = (int)2001; return; }
  // email collision
  if(taupo_is_user($email)) { $global['jsout']['error'] = (int)2000; return; }
  // pass policy
  if(!taupo_pass_policy($pass, $email)) { $global['jsout']['error'] = (int)2002; return; }
  // validate locale preference
  if(!in_array($locale,$conf['i18n_on']) && !in_array($locale,$conf['i18n_off'])) { $locale = 'en'; }
  // inactive registration
  if($inactive == true) {
    // create new inactive user
    if(!taupo_add_user($email, $pass, 'inactive', $locale)) { $global['jsout']['error'] = (int)9999; return; }
    $nullToken = true;
  // captcha validation
  } else {
    // passed captcha
    if($captcha == '6Lfse_8SAAAAAK6TCQ3pwqNfaHDVOxMyvflvREhQ') {
      // create pending new user
      if(!taupo_add_user($email, $pass, 'pending', $locale)) { $global['jsout']['error'] = (int)9999; return; }
    // failed captcha
    } else { $global['jsout']['error'] = (int)9999; return; }
    $nullToken = false;
  }
  // email new user
  $q = "
    SELECT *
    FROM   `users`
    WHERE  `user` = ".$global['db']->qstr((string)$email).";
  ";
  $result = $global['db']->Execute($q);
  if($result === false) { daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "rest_register ".$q); }
  rest_emailNewUser($email, $result->fields['confirmation'], $result->fields['locale']);
  if($nullToken === false) {
    $token2 = rest_newToken($result->fields['uid'], $result->fields['gid']);
    rest_cookie($token2,    $result->fields['uid'], $result->fields['gid'], $email);
  } else { $token2 = null; }
  $global['jsout']['error']  = $ecode;
  $global['jsout']['token2'] = $token2;
}

// event data
function rest_events($return = false) {
  global $global, $conf;
  $gid = (int)rest_gidToken($global['jsin']['token']);
  $global['jsout'] = rest_getEvents($gid);
}

// report abuse
function rest_abuse() {
  global $global, $conf;
  $uuid = isset($global['jsin']['uuid']) ? (string)$global['jsin']['uuid'] : (string)'';
  $type = isset($global['jsin']['type']) ?    (int)$global['jsin']['type'] : (int)9999;
  // validate type
  if($type < 0 || $type > 4) { $global['jsout']['error'] = (int)2101; return; } // invalid type 
  // check record exists
  $q = "
    SELECT *
    FROM `person_uuid` p, `incident` i
    WHERE p.p_uuid = ".$global['db']->qstr($uuid)."
    AND p.incident_id = i.incident_id
    LIMIT 1;
  ";
  $result = $global['db']->Execute($q);
  if($result === false) { daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "rest_abuse ".$q); }
  // record doesn't exist
  if($result == null || $result->EOF) { $global['jsout']['error'] = (int)2100; return; }
  // check auth
  $a = rest_checkEventToken($global['jsin']['token'], $result->fields['shortname']);
  // cannot access this event // return invalid uuid
  if((int)$a['status'] !== 0) { $global['jsout']['error'] = (int)$a['status']; return; }
  // flag record if not already flagged
  $q = "
    INSERT INTO `abuse` (p_uuid, type)
    SELECT * FROM (SELECT ".$global['db']->qstr($uuid).", ".$global['db']->qstr($type).") AS tmp
    WHERE NOT EXISTS (
      SELECT p_uuid FROM `abuse` WHERE p_uuid = ".$global['db']->qstr($uuid)."
    ) LIMIT 1;
  ";
  $result = $global['db']->Execute($q);
  if($result === false) { daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "rest_abuse 2 ".$q); }
}

// delete record
function rest_delete() {
  global $global, $conf;
  $uuid = isset($global['jsin']['uuid']) ? (string)$global['jsin']['uuid'] : (string)'';
  // check record exists
  $q = "
    SELECT *
    FROM `person_uuid` p, `incident` i
    WHERE p.p_uuid = ".$global['db']->qstr($uuid)."
    AND p.incident_id = i.incident_id
    LIMIT 1;
  ";
  $result = $global['db']->Execute($q);
  if($result === false) { daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "rest_delete ".$q); }
  // record doesn't exist
  if($result == null || $result->EOF) { $global['jsout']['error'] = (int)2200; return; }
  // check auth
  $a = rest_checkEventToken($global['jsin']['token'], $result->fields['shortname']);
  // cannot access this event
  if((int)$a['status'] !== 0) { $global['jsout']['error'] = (int)$a['status']; return; }
  // admin // delete record immediately
  if($a['gid'] == 1) {
    $p = new person();
    $p->p_uuid = $uuid;
    $p->load();
    $p->update_uid = $a['uid'];
    $p->delete();
  // expire if owner
  } else {
    $p = new person();
    $p->p_uuid = $uuid;
    $p->load();
    $p->update_uid = $a['uid'];
    if($p->isOwner($a['uid'])) { $p->expire(); }
    // bad request
    else { $global['jsout']['error'] = (int)2200; }
  }
}

// logout user // expire 1 or all tokens
function rest_logout() {
  global $global, $conf;
  $single = isset($global['jsin']['all']) ? (boolean)$global['jsin']['single'] : true;
  // check auth
  $a = rest_checkToken($global['jsin']['token']);
  // anon cannot purge token
  if((int)$a['gid'] == 3 || (int)$a['status'] !== 0) {
    fail2ban();
    $global['jsout']['error'] = (int)1;
    return;
  }
  rest_removeExpiredTokens();
  // purge all tokens
  if(!$single) {
    $q = "
      DELETE FROM `tokens`
      WHERE uid = ".$global['db']->qstr($a['uid']).";
    ";
    $result = $global['db']->Execute($q);
    if($result === false) { daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "rest_logout ".$q); }
  // purge current token only
  } else {
    $q = "
      DELETE FROM `tokens`
      WHERE `sha_512_token` = ".$global['db']->qstr($global['jsin']['token']).";
    ";
    $result = $global['db']->Execute($q);
    if($result === false) { daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "rest_logout 2 ".$q); }
  }
  $token2 = rest_newToken(3, 3, date("Y-m-d G:i:s", time()+MAX_ALIVE_ANON));
  $global['jsout']['token2'] = (string)$token2;
  rest_cookie($token2, 3, 3, null);
}

// forgot password
function rest_forgot() {
  global $global, $conf;
  $email = isset($global['jsin']['email']) ? (string)$global['jsin']['email'] : '';
  // check auth
  $a = rest_checkToken($global['jsin']['token']);
  // invalid token
  if((int)$a['status'] !== 0) { $global['jsout']['error'] = (int)$a['status']; return; }
  // email validation
  if(!validateAddress($email)) { $global['jsout']['error'] = (int)2001; return; }
  // valid user
  if(taupo_is_user($email)) {
    // gen confirm code
    $confirmation = hash('sha256', openssl_random_pseudo_bytes(256));
    $q = "
      UPDATE `users`
      SET    `confirmation` = '".$confirmation."',
             `changed` = ".$global['db']->qstr(time())."
      WHERE  `user` = ".$global['db']->qstr($email).";
    ";
    $res = $global['db']->Execute($q);
    if($res === false) { daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "rest_forgot ".$q); }
    // get user locale
    $q = "
      SELECT *
      FROM   `users`
      WHERE  `user` = ".$global['db']->qstr($email).";
    ";
    $result = $global['db']->Execute($q);
    if($result === false) { daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "rest_forgot 2 ".$q); }
    rest_emailForgotOne($email, $confirmation, $result->fields['locale']);
  }
}

// reset password
function rest_reset() {
  global $global, $conf;
  $confirmation = isset($global['jsin']['confirmation']) ? (string)$global['jsin']['confirmation'] : '';
  // check auth
  $a = rest_checkToken($global['jsin']['token']);
  // invalid token
  if((int)$a['status'] !== 0) { $global['jsout']['error'] = (int)$a['status']; return; }
  // validate confirmation code
  $q = "
    SELECT COUNT(*), `user`, `locale`
    FROM   `users`
    WHERE  `confirmation` = ".$global['db']->qstr($confirmation).";
  ";
  $result = $global['db']->Execute($q);
  if($result === false) { daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "rest_reset ".$q); }
  $row = $result->FetchRow();
  $c = (int)$row['COUNT(*)'];
  if((int)$c === 1) {
    // change user password & email
    $newPass = randomPass();
    taupo_change_pass($row['user'], $newPass);
    rest_emailForgotTwo($row['user'], $row['locale'], $newPass);
    // delete confirmation // activate user
    $q = "
      UPDATE `users`
      SET    `status` = 'active',
             `confirmation` = NULL
      WHERE  `user` = ".$global['db']->qstr($row['user']).";
    ";
    $r = $global['db']->Execute($q);
    if($r === false) { daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "rest_reset 2 ".$q); }
  } else { $global['jsout']['error'] = (int)2200; }
}

// change password
function rest_change() {
  global $global, $conf;
  $user = isset($global['jsin']['user']) ? (string)$global['jsin']['user'] : '';
  $old  = isset($global['jsin']['old'])  ? (string)$global['jsin']['old']  : '';
  $new  = isset($global['jsin']['new'])  ? (string)$global['jsin']['new']  : '';
  // check auth
  $a = rest_checkToken($global['jsin']['token']);
  // invalid token
  if((int)$a['status'] !== 0) { $global['jsout']['error'] = (int)$a['status']; return; }
  // new pass meets policy
  if(taupo_pass_policy($new, $user) !== true) { $global['jsout']['error'] = (int)2002; return; }
  // check old pass auth
  $status = rest_authCheck($user, $old);
  if((int)$status !== 0) {
    fail2ban();
    $global['jsout']['error'] = (int)$status;
    return;
  }
  taupo_change_pass($user, $new);
  // update user timestamp
  $q = "
    UPDATE `users`
    SET    `changed` = ".$global['db']->qstr(time())."
    WHERE  `user`    = ".$global['db']->qstr($user).";
  ";
  $result = $global['db']->Execute($q);
  if($result === false) { daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "rest_change ".$q); }
}

// confirm user
function rest_confirm() {
  global $global, $conf;
  $confirmation = isset($global['jsin']['confirmation']) ? (string)$global['jsin']['confirmation'] : '';
  // check auth
  $a = rest_checkToken($global['jsin']['token']);
  // invalid token
  if((int)$a['status'] !== 0) { $global['jsout']['error'] = (int)$a['status']; return; }
  // validate confirmation code
  $q = "
    SELECT COUNT(*)
    FROM   `users`
    WHERE  `confirmation` = ".$global['db']->qstr($confirmation).";
  ";
  $result = $global['db']->Execute($q);
  if($result === false) { daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "rest_confirm ".$q); }
  $row = $result->FetchRow();
  $c = (int)$row['COUNT(*)'];
  if((int)$c === 1) { rest_activateUser($confirmation); }
  else { $global['jsout']['error'] = (int)2200; }
}

// list of records user is following
function rest_following() {
  global $global, $conf;
  $r = array();
  // check auth
  $a = rest_checkToken($global['jsin']['token']);
  // validate/init locale
  $locale = isset($global['jsin']['locale']) ? (string)$global['jsin']['locale'] : (string)'en';
  if(!in_array($locale, $conf['i18n_on']) && !in_array($locale, $conf['i18n_off'])) { $locale = (string)'en'; }
  // valid token
  if((int)$a['status'] === 0) {
    $q = "
      SELECT *
      FROM   `following` f, `person_uuid` p, `incident` i
      WHERE  f.uid = '".$a['uid']."'
      AND    f.p_uuid = p.p_uuid
      AND    i.incident_id = p.incident_id;
    ";
    $result = $global['db']->Execute($q);
    if($result === false) { daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "rest_following ".$q); }
    while(!$result == NULL && !$result->EOF) {
      $name = (string)$result->fields['full_name'];
      if(trim($name) === '') { $name = 'unk'; }
      $s = array(
        'uuid'  => (string)$result->fields['p_uuid'],
        'name'  => (string)$name,
        'url'   => (string)makeRecordUrl($result->fields['p_uuid'], $locale),
        'short' => (string)$result->fields['shortname']
      );
      $r[] = $s;
      $result->MoveNext();
    }
  }
  $global['jsout'] = $r;
}

// follow a record
function rest_follow() {
  global $global, $conf;
  $uuid = isset($global['jsin']['uuid']) ?  (string)$global['jsin']['uuid'] : (string)'';
  $sub  = isset($global['jsin']['sub'])  ? (boolean)$global['jsin']['sub']  : false;
  // check auth
  $a = rest_checkToken($global['jsin']['token']);
  // invalid token
  if((int)$a['status'] !== 0) { $global['jsout']['error'] = (int)$a['status']; return; }
  // invalid event access
  if(!rest_gidRecord($a['gid'], $uuid)) { $global['jsout']['error'] = (int)2100; return; }
  // sub
  if($sub) {
    $q = "
      INSERT INTO `following` (uid, p_uuid)
      SELECT * FROM (SELECT ".$a['uid'].", ".$global['db']->qstr($uuid).") AS tmp
      WHERE NOT EXISTS (
        SELECT `p_uuid`, `uid` FROM `following` WHERE `p_uuid` = ".$global['db']->qstr($uuid)." AND `uid` = ".$a['uid']."
      ) LIMIT 1;
    ";
    $r = $global['db']->Execute($q);
    if($r === false) { daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "rest_follow ".$q); }
  // unsub
  } else {
    $q = "
        DELETE FROM `following`
        WHERE `uid` = '".$a['uid']."'
        AND `p_uuid` = ".$global['db']->qstr($uuid).";
    ";
    $r = $global['db']->Execute($q);
    if($r === false) { daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "rest_follow 2 ".$q); }
  }
}

// push notifications
function rest_push() {
  global $global, $conf;
  $rid   = isset($global['jsin']['rid'])   ?  (string)$global['jsin']['rid']   : (string)'';
  $about = isset($global['jsin']['about']) ?  (string)$global['jsin']['about'] : (string)'';
  $sub   = isset($global['jsin']['sub'])   ? (boolean)$global['jsin']['sub']   : false;
  // check auth
  $a = rest_checkToken($global['jsin']['token']);
  // invalid token
  if((int)$a['status'] !== 0) { $global['jsout']['error'] = (int)$a['status']; return; }
  // subscribe
  if($sub) {
    if(strlen($rid) > 2) {
      // begin transaction
      $global['db']->StartTrans();
      $q = "
        DELETE
        FROM  `push`
        WHERE `registration_id` = ".$global['db']->qstr($rid).";
      ";
      $result = $global['db']->Execute($q);
      if($result === false) { daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "rest_push ".$q); }
      $q = "
        INSERT INTO `push` (
          `registration_id`,
          `version_string`,
          `uid`
        ) VALUES (
          ".$global['db']->qstr((string)$rid).",
          ".$global['db']->qstr((string)$about).",
          ".(int)$a['uid']."
        );
      ";
      $result = $global['db']->Execute($q);
      if($result === false) {           daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "rest_push 2 ".$q); }
      $global['db']->HasFailedTrans() ? daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), 'rest_push 3 trans_fail') : null;
      $global['db']->CompleteTrans();
      // end transaction
    }
  // unsubscribe
  } else {
    if(strlen($rid) > 2) {
      $q = "
        DELETE
        FROM  `push`
        WHERE `registration_id` = ".$global['db']->qstr($rid).";
      ";
      $result = $global['db']->Execute($q);
      if($result === false) { daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "rest_push 4 ".$q);  }
    }
  }
}

// add comment
function rest_comment() {
  global $global, $conf;
  $uuid      = isset($global['jsin']['uuid'])      ?  (string)$global['jsin']['uuid']      : (string)'';
  $text      = isset($global['jsin']['text'])      ?  (string)$global['jsin']['text']      : (string)'';
  $stat      = isset($global['jsin']['stat'])      ?  (string)$global['jsin']['stat']      : (string)'';
  $latitude  = isset($global['jsin']['latitude'])  ?  (double)$global['jsin']['latitude']  : (double)0;
  $longitude = isset($global['jsin']['longitude']) ?  (double)$global['jsin']['longitude'] : (double)0;
  $photo     = isset($global['jsin']['photo'])     ?  (string)$global['jsin']['photo']     : (string)'';
  // check auth
  $a = rest_checkToken($global['jsin']['token']);
  // invalid token
  if((int)$a['status'] !== 0) { $global['jsout']['error'] = (int)$a['status']; return; }
  $p = new person();
  $p->p_uuid = $uuid;
  $p->load();
  // error loading record
  if($p->ecode >= 9900 && $p->ecode <= 9910) { $global['jsout']['error'] = (int)$p->ecode; return; } // all good // add comment
  else { 
    $p->addComment($text, $stat, json_encode(array('lat'=>$latitude, 'lon'=>$longitude)), $photo, $a['uid']);
    rest_sendRevisionMessage($uuid);
  }
}

// ping
function rest_ping() {
  global $global, $conf;
  $latitude  = isset($global['jsin']['latitude'])  ? (double)$global['jsin']['latitude']  : (double)0;
  $longitude = isset($global['jsin']['longitude']) ? (double)$global['jsin']['longitude'] : (double)0;
  // check auth
  $a = rest_checkToken($global['jsin']['token']);
  // invalid token
  if((int)$a['status'] !== 0) { $global['jsout']['error'] = (int)$a['status']; return; }
  $ip = (string)get_client_ip();
  $cachetoken = $global['cache']->getItem($global['jsin']['token']);
  // count is one for new token
  if(is_null($cachetoken->get())) {
    // add new to cache
    $cachetoken->set(1)->expiresAfter(60*60*24*7)->addTag('ping')->addTag('lat_'.$latitude.':lon_'.$longitude)->addTag('ip_'.$ip);
    $count = 1;
  // increment count
  } else {
    // increment and renew cache item
    $cachetoken->increment();
    $cachetoken->expiresAfter(60*60*24*7);
    $cachetoken->addTag('lat_'.$latitude.':lon_'.$longitude);
    $count = $cachetoken->get();
  }
  $global['cache']->save($cachetoken);
  $dto = $cachetoken->getExpirationDate();
}

// user preferences
function rest_pref() {
  global $global, $conf;
  $locale = isset($global['jsin']['locale']) ? (string)$global['jsin']['locale'] : (string)'';
  // check auth
  $a = rest_checkToken($global['jsin']['token']);
  // invalid token (not valid admin or user token; valid anon token is invalid here)
  if((int)$a['status'] !== 0 || (int)$a['gid'] === 3) { $global['jsout']['error'] = (int)9000; return; }
  // validate locale preference
  if(!in_array($locale,$conf['i18n_on']) && !in_array($locale,$conf['i18n_off'])) { $global['jsout']['error'] = (int)3000; return; }
  // save user locale preference
  $q = "
    UPDATE `users`
    SET    `locale` = ".$global['db']->qstr($locale)."
    WHERE  `uid`    = ".$a['uid'].";
  ";
  $result = $global['db']->Execute($q);
  if($result === false) { daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "rest_pref ".$q); }
}

// messaging options
function rest_messages() {
  global $global, $conf;
  $record_push  = isset($global['jsin']['record_push'])  ? (boolean)$global['jsin']['record_push']  : (string)"UNCHANGED";
  $record_email = isset($global['jsin']['record_email']) ? (boolean)$global['jsin']['record_email'] : (string)"UNCHANGED";
  $event_push   = isset($global['jsin']['event_push'])   ? (boolean)$global['jsin']['event_push']   : (string)"UNCHANGED";
  $event_email  = isset($global['jsin']['event_email'])  ? (boolean)$global['jsin']['event_email']  : (string)"UNCHANGED";
  $admin_push   = isset($global['jsin']['admin_push'])   ? (boolean)$global['jsin']['admin_push']   : (string)"UNCHANGED";
  $admin_email  = isset($global['jsin']['admin_email'])  ? (boolean)$global['jsin']['admin_email']  : (string)"UNCHANGED";
  // check auth
  $a = rest_checkToken($global['jsin']['token']);
  // invalid token (not valid admin or user token; valid anon token is invalid here)
  if((int)$a['status'] !== 0 || (int)$a['gid'] === 3) { $global['jsout']['error'] = (int)9000; return; }
  // load current options
   $q = "
    SELECT *
    FROM `users`
    WHERE `uid` = ".$global['db']->qstr($a['uid'])."
    LIMIT 1;
  ";
  $result = $global['db']->Execute($q);
  if($result === false) { daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "rest_messages ".$q); }
  if($result == null || $result->EOF) { $global['jsout']['error'] = (int)9999; return; } // unknown error
  $_record_push  = (boolean)$result->fields['record_push'];
  $_record_email = (boolean)$result->fields['record_email'];
  $_event_push   = (boolean)$result->fields['event_push'];
  $_event_email  = (boolean)$result->fields['event_email'];
  $_admin_push   = (boolean)$result->fields['admin_push'];
  $_admin_email  = (boolean)$result->fields['admin_email'];
  // overwrite unchanged's
  if(is_string($record_push))  { $record_push  = $_record_push;  }
  if(is_string($record_email)) { $record_email = $_record_email; }
  if(is_string($event_push))   { $event_push   = $_event_push;   }
  if(is_string($event_email))  { $event_email  = $_event_email;  }
  if(is_string($admin_push))   { $admin_push   = $_admin_push;   }
  if(is_string($admin_email))  { $admin_email  = $_admin_email;  }
    // save user locale preference
  $q = "
    UPDATE `users`
    SET    `record_push`  = ".$global['db']->qstr((int)$record_push).",
           `record_email` = ".$global['db']->qstr((int)$record_email).",
           `event_push`   = ".$global['db']->qstr((int)$event_push).",
           `event_email`  = ".$global['db']->qstr((int)$event_email).",
           `admin_push`   = ".$global['db']->qstr((int)$admin_push).",
           `admin_email`  = ".$global['db']->qstr((int)$admin_email)."
    WHERE  `uid`    = ".$a['uid'].";
  ";
  $result = $global['db']->Execute($q);
  if($result === false) { daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "rest_messages 2 ".$q); }
  // out
  $global['jsout']['record_push']  = (boolean)$record_push;
  $global['jsout']['record_email'] = (boolean)$record_email;
  $global['jsout']['event_push']   = (boolean)$event_push;
  $global['jsout']['event_email']  = (boolean)$event_email;
  $global['jsout']['admin_push']   = (boolean)$admin_push;
  $global['jsout']['admin_email']  = (boolean)$admin_email;
}
