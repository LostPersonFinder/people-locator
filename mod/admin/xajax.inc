<?
/**
 * @name     Admin X
 * @author   pl@miernicki.com
 * @about    Developed by the U.S. National Library of Medicine
 * @link     https://gitlab.com/tehk/people-locator
 * @license  https://gitlab.com/tehk/people-locator/blob/master/LICENSE
 */

global $global;
require_once($conf['approot'].'mod/admin/users.inc');
// create an array if it does not already exist
$global['xajax_functions'] = array();
// publicly register accessible xajax funtions
array_push($global['xajax_functions'], 'admin_show_dashboard');
array_push($global['xajax_functions'], 'admin_clear_ban_list');
array_push($global['xajax_functions'], 'admin_show_users');
array_push($global['xajax_functions'], 'admin_edit_user');
array_push($global['xajax_functions'], 'admin_save_edit');
array_push($global['xajax_functions'], 'admin_del_user_do');

// show dashboard
function admin_show_dashboard() {
  global $global, $conf;
  // admin_show_rest(); // count hits
  $q = "
    SELECT COUNT(*)
    FROM `rest_log`
    LIMIT 1;
  ";
  $result = $global['db']->Execute($q);
  if($result === false) { daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "fm log 0 ((".$q."))"); }
  $hits = $result->fields['COUNT(*)'];
  // count sessions
  $q = "
    SELECT COUNT(*)
    FROM `tokens`
    LIMIT 1;
  ";
  $result = $global['db']->Execute($q);
  if($result === false) { daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "fm log 0 ((".$q."))"); }
  $sessions = $result->fields['COUNT(*)'];
  $out = '<div><paper-button class="nonUpper bold hover" onclick="">Web Services</paper-button></div>';
  // the table
  $out .= '
    <table>
      <tr><td class="center banRowEven">Hits</td><td class="center banRowEven">'.$hits.'</td></tr>
      <tr><td class="center banRowOdd">Sessions</td><td class="center banRowOdd">'.$sessions.'</td></tr>
    </table>
  ';
  $global['xajax_res']->addAssign('restDiv', 'innerHTML', $out);
  // pfif
  $out = '';
  require_once($conf['approot'].'mod/pfif/main.inc');
  if(!taupo_pfif_statusGood() && $conf['gpf_api_key'] != '') { $out .= '<paper-button class="hover bgRed" onclick="window.location = \'pfif\';">PFIF</paper-button>'; }
  else { $out .= '<paper-button class="hover" onclick="window.location = \'pfif\';">PFIF</paper-button>'; }
  $global['xajax_res']->addAssign('pfifDiv', 'innerHTML', $out);
  // solr
  $out = '';
  require_once($conf['approot'].'mod/solr/main.inc');
  if(!taupo_solr_status() && $conf['SOLR_on'] != false) { $out .= '<paper-button class="hover bgRed" onclick="window.location = \'solr\';">SOLR</paper-button>'; }
  else { $out .= '<paper-button class="hover" onclick="window.location = \'solr\';">SOLR</paper-button>'; }
  $global['xajax_res']->addAssign('solrDiv', 'innerHTML', $out);
  // emails
  $out = '';
  require_once($conf['approot'].'mod/email/main.inc');
  if(!taupo_email_statusGood()) { $out .= '<paper-button class="nonUpper bold hover bgRed" onclick="window.location=\'email\';">Emails</paper-button></center>'; }
  else { $out .= '<paper-button class="nonUpper bold hover" onclick="window.location=\'email\';">Emails</paper-button></center>'; }
  $q = "
    SELECT COUNT(*)
    FROM `emails`
    WHERE `time_sent` > NOW() - interval 1 day
    LIMIT 1;
  ";
  $result = $global['db']->Execute($q);
  if($result === false) { daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "arrivals simple >> ".$q); }
  $pastDay = $result->fields['COUNT(*)'];
  $q = "
    SELECT COUNT(*)
    FROM `emails`
    WHERE `time_sent` > NOW() - interval 1 week
    LIMIT 1;
  ";
  $result = $global['db']->Execute($q);
  if($result === false) { daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "arrivals simple >> ".$q); }
  $pastWeek = $result->fields['COUNT(*)'];
  $q = "
    SELECT COUNT(*)
    FROM `emails`
    WHERE `time_sent` > NOW() - interval 1 month
    LIMIT 1;
  ";
  $result = $global['db']->Execute($q);
  if($result === false) { daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "arrivals simple >> ".$q); }
  $pastMonth = $result->fields['COUNT(*)'];
  $q = "
    SELECT COUNT(*)
    FROM `emails`
    LIMIT 1;
  ";
  $result = $global['db']->Execute($q);
  if($result === false) { daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "arrivals simple >> ".$q); }
  $allTime = $result->fields['COUNT(*)'];
  $out .= '
    <table>
      <tr><td class="center banRowOdd">day</td><td class="center banRowOdd">'.$pastDay.'</td></tr>
      <tr><td class="center banRowEven">week</td><td class="center banRowEven">'.$pastWeek.'</td></tr>
      <tr><td class="center banRowOdd">month</td><td class="center banRowOdd">'.$pastMonth.'</td></tr>
      <tr><td class="center banRowEven">all</td><td class="center banRowEven">'.$allTime.'</td></tr>
    </table>
  ';
  $global['xajax_res']->addAssign('emailDiv', 'innerHTML', $out);
  // abuse
  $out = '';
  require_once($conf['approot'].'mod/abuse/main.inc');
  $qCount = taupo_abuse_count();
  if($qCount > 0) { $out .= '<paper-button class="noshadow hover bgRed" onclick="window.location = \'abuse\';">ABUSE</paper-button>'; }
  else { $out .= '<paper-button class="noshadow hover" onclick="window.location = \'abuse\';">ABUSE</paper-button>'; }
  $q = "
    SELECT COUNT(*)
    FROM `abuse`
    LIMIT 1;
  ";
  $result = $global['db']->Execute($q);
  if($result === false) { daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "arrivals simple >> ".$q); }
  $aaa = $result->fields['COUNT(*)'];
  $out .= '
    <table>
      <tr><td class="center banRowOdd">Requests</td><td class="center banRowOdd">'.$aaa.'</td></tr>
    </table>
  ';
  $global['xajax_res']->addAssign('abuseDiv', 'innerHTML', $out);
  // admin_show_fm();
  if(sizeof($conf['acl']['fm']) >= 1) {
    require_once($conf['approot'].'mod/fm/main.inc');
    // count ingests
    $q = "
      SELECT COUNT(*)
      FROM `facematch_log`
      WHERE `service` = 0;
    ";
    $result = $global['db']->Execute($q);
    if($result === false) { daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "fm log 0 ((".$q."))"); }
    $ingests = $result->fields['COUNT(*)'];
    // count queries
    $q = "
      SELECT COUNT(*)
      FROM `facematch_log`
      WHERE `service` = 1;
    ";
    $result = $global['db']->Execute($q);
    if($result === false) { daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "fm log 0 ((".$q."))"); }
    $queries = $result->fields['COUNT(*)'];
    // count removes
    $q = "
      SELECT COUNT(*)
      FROM `facematch_log`
      WHERE `service` = 2;
    ";
    $result = $global['db']->Execute($q);
    if($result === false) { daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "fm log 0 ((".$q."))"); }
    $removes = $result->fields['COUNT(*)'];
    // count local
    $q = "
      SELECT COUNT(*)
      FROM `facematch_log`
      WHERE `source` = 0;
    ";
    $result = $global['db']->Execute($q);
    if($result === false) { daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "fm log 0 ((".$q."))"); }
    $local = $result->fields['COUNT(*)'];
    // count website
    $q = "
      SELECT COUNT(*)
      FROM `facematch_log`
      WHERE `source` = 1;
    ";
    $result = $global['db']->Execute($q);
    if($result === false) { daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "fm log 0 ((".$q."))"); }
    $website = $result->fields['COUNT(*)'];
    // count ru ios
    $q = "
      SELECT COUNT(*)
      FROM `facematch_log`
      WHERE `source` = 2;
    ";
    $result = $global['db']->Execute($q);
    if($result === false) { daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "fm log 0 ((".$q."))"); }
    $ruios = $result->fields['COUNT(*)'];
    // count ru android
    $q = "
      SELECT COUNT(*)
      FROM `facematch_log`
      WHERE `source` = 3;
    ";
    $result = $global['db']->Execute($q);
    if($result === false) { daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "fm log 0 ((".$q."))"); }
    $ruandroid = $result->fields['COUNT(*)'];
    $tp = '';
    require_once($conf['approot'].'mod/fm/main.inc');
    if(!taupo_facematch_status()) { $out = '<center><paper-button class="nonUpper bold hover bgRed" onclick="window.location=\'facematch\';">Facematch</paper-button></center>'; }
    else { $out = '<center><paper-button class="nonUpper bold hover" onclick="window.location=\'facematch\';">Facematch</paper-button></center>'; }
    $out .= '
      <table>
        <tr><td class="center banRowEven">Ingests</td><td class="center banRowEven">'.$ingests.'</td></tr>
        <tr><td class="center banRowOdd">Queries</td><td class="center banRowOdd">'.$queries.'</td></tr>
        <tr><td class="center banRowEven">Removes</td><td class="center banRowEven">'.$removes.'</td></tr>
        <tr><td colspan=2>&nbsp;</td></tr>
        <tr><td class="center banRowEven">Local</td><td class="center banRowEven">'.$local.'</td></tr>
        <tr><td class="center banRowOdd">Website</td><td class="center banRowOdd">'.$website.'</td></tr>
        <tr><td class="center banRowEven">ReUnite iOS</td><td class="center banRowEven">'.$ruios.'</td></tr>
        <tr><td class="center banRowOdd">ReUnite Android</td><td class="center banRowOdd">'.$ruandroid.'</td></tr>
        '.$tp.'
      </table>
    ';
  } else { $out = ''; }
  $global['xajax_res']->addAssign('fmDiv', 'innerHTML', $out);
  // admin_show_notify();
  if(sizeof($conf['acl']['notify']) >= 1) {
    require_once($conf['approot'].'mod/notify/main.inc');
    $all = get_push_count_all();
    $web = get_push_count_web();
    $android = get_push_count_android();
    $ios = get_push_count_ios();
    $out = '
      <table>
        <tr>
          <td class="center banRowEven">Web</td>
          <td class="center banRowEven">'.$web.'</td>
        </tr>
        <tr>
          <td class="center banRowOdd">Android</td>
          <td class="center banRowOdd">'.$android.'</td>
        </tr>
        <tr>
          <td class="center banRowEven">iOS</td>
          <td class="center banRowEven">'.$ios.'</td>
        </tr>
        <tr>
          <td class="center banRowOdd">ALL</td>
          <td class="center banRowOdd">'.$all.'</td>
        </tr>
      </table>
    ';
  } else { $out = '<span style="color: #ddd !important;">- disabled -</span>'; }
  $global['xajax_res']->addAssign('notifyDiv', 'innerHTML', $out);
  // admin_show_services();
  $out2 = 'updated '.date("g:i:s a T");
  $global['xajax_res']->addScript("redTime('".$out2."');");
  // admin_show_arrivals();
  $q = "
    SELECT COUNT(*)
    FROM `arrival_rate`
    WHERE `arrival_time` > NOW() - interval 1 hour
    LIMIT 1;
  ";
  $result = $global['db']->Execute($q);
  if($result === false) { daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "arrivals simple >> ".$q); }
  $pastHour = $result->fields['COUNT(*)'];
  $q = "
    SELECT COUNT(*)
    FROM `arrival_rate`
    WHERE `arrival_time` > NOW() - interval 1 day
    LIMIT 1;
  ";
  $result = $global['db']->Execute($q);
  if($result === false) { daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "arrivals simple >> ".$q); }
  $pastDay = $result->fields['COUNT(*)'];
  $q = "
    SELECT COUNT(*)
    FROM `arrival_rate`
    WHERE `arrival_time` > NOW() - interval 1 week
    LIMIT 1;
  ";
  $result = $global['db']->Execute($q);
  if($result === false) { daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "arrivals simple >> ".$q); }
  $pastWeek = $result->fields['COUNT(*)'];
  $q = "
    SELECT COUNT(*)
    FROM `arrival_rate`
    WHERE `arrival_time` > NOW() - interval 1 month
    LIMIT 1;
  ";
  $result = $global['db']->Execute($q);
  if($result === false) { daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "arrivals simple >> ".$q); }
  $pastMonth = $result->fields['COUNT(*)'];
  $q = "
    SELECT COUNT(*)
    FROM `arrival_rate`
    LIMIT 1;
  ";
  $result = $global['db']->Execute($q);
  if($result === false) { daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "arrivals simple >> ".$q); }
  $allTime = $result->fields['COUNT(*)'];
  $out = '
    <table>
      <tr><td class="center banRowEven">hour</td><td class="center banRowEven">'.$pastHour.'</td></tr>
      <tr><td class="center banRowOdd">day</td><td class="center banRowOdd">'.$pastDay.'</td></tr>
      <tr><td class="center banRowEven">week</td><td class="center banRowEven">'.$pastWeek.'</td></tr>
      <tr><td class="center banRowOdd">month</td><td class="center banRowOdd">'.$pastMonth.'</td></tr>
      <tr><td class="center banRowEven">all</td><td class="center banRowEven">'.$allTime.'</td></tr>
    </table>  
  ';
  $global['xajax_res']->addAssign('arrivalsDiv', 'innerHTML', $out);
  // admin_show_ban_list();
  if(isset($conf['fail2ban_on']) && $conf['fail2ban_on'] == true) {
    $count = 0;
    $eo = 0;
    $out = '
      <table class="banTable">
        <tr>
          <td class="banRowEven"><b>IP</b></td>
          <td class="banRowEven noLeftPadding"><b>Expires</b></td>
        </tr>
    ';
    $ips = $global['cache']->getItemsByTag('fail2ban');
    foreach($ips as $ip) {
      if($ip->get() >= MAX_AUTH_FAILURES) { 
        $count++;
        if($eo == 0) { $evenOddClass = "banRowOdd";  }
        else         { $evenOddClass = "banRowEven"; }
        $dto = $ip->getExpirationDate();
        $out .= '
          <tr>
            <td class="'.$evenOddClass.' fontEighty">'.$ip->getKey().'</td>
            <td class="'.$evenOddClass.' fontEighty noLeftPadding">'.$dto->format('m/d H:i').'</td>
          </tr>
        ';
        $eo++;
        if($eo == 2) { $eo = 0; }
      }
    }
    if($count < 1) { $out = '<tr><td colspan=2 class="center banRowEven">empty</td></tr>'; }
    else { $out .= '<tr><td colspan=2><a onclick="admin_clear_ban_list();"><paper-button class="" role="button">Clear Ban List</paper-button></a></td></tr>'; }
  } else { $out = '<span style="color: #ddd !important;">DISABLED</span>'; }
  $global['xajax_res']->addAssign('ipDiv', 'innerHTML', $out);
  // admin_show_stats();
  $out = "";
  $tr = '';
  // missing
  $q = "
    SELECT COUNT(*)
    FROM `person_status`
    WHERE `opt_status` = 'mis';
  ";
  $result = $global['db']->Execute($q);
  if($result === false) { daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "arrivals simple >> ".$q); }
  $mis = $result->fields['COUNT(*)'];
  $tr .= '
    <tr>
      <td class="center banRowEven">Missing</td>
      <td class="center banRowEven">'.$mis.'</td>
    </tr>
  ';
  // found
  $q = "
    SELECT COUNT(*)
    FROM `person_status`
    WHERE `opt_status` = 'fnd';
  ";
  $result = $global['db']->Execute($q);
  if($result === false) { daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "arrivals simple >> ".$q); }
  $fnd = $result->fields['COUNT(*)'];
  $tr .= '
    <tr>
      <td class="center banRowOdd">Found</td>
      <td class="center banRowOdd">'.$fnd.'</td>
    </tr>
  ';
  // alive
  $q = "
    SELECT COUNT(*)
    FROM `person_status`
    WHERE `opt_status` = 'ali';
  ";
  $result = $global['db']->Execute($q);
  if($result === false) { daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "arrivals simple >> ".$q); }
  $ali = $result->fields['COUNT(*)'];
  $tr .= '
    <tr>
      <td class="center banRowEven">Alive & Well</td>
      <td class="center banRowEven">'.$ali.'</td>
    </tr>
  ';
  // injured
  $q = "
    SELECT COUNT(*)
    FROM `person_status`
    WHERE `opt_status` = 'inj';
  ";
  $result = $global['db']->Execute($q);
  if($result === false) { daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "arrivals simple >> ".$q); }
  $inj = $result->fields['COUNT(*)'];
  $tr .= '
    <tr>
      <td class="center banRowOdd">Injured</td>
      <td class="center banRowOdd">'.$inj.'</td>
    </tr>
  ';
  // deceased
  $q = "
    SELECT COUNT(*)
    FROM `person_status`
    WHERE `opt_status` = 'dec';
  ";
  $result = $global['db']->Execute($q);
  if($result === false) { daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "arrivals simple >> ".$q); }
  $dec = $result->fields['COUNT(*)'];
  $tr .= '
    <tr>
      <td class="center banRowEven">Deceased</td>
      <td class="center banRowEven">'.$dec.'</td>
    </tr>
  ';
  // unknown
  $q = "
    SELECT COUNT(*)
    FROM `person_status`
    WHERE `opt_status` = 'unk';
  ";
  $result = $global['db']->Execute($q);
  if($result === false) { daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "arrivals simple >> ".$q); }
  $unk = $result->fields['COUNT(*)'];
  $tr .= '
    <tr>
      <td class="center banRowOdd">Unknown</td>
      <td class="center banRowOdd">'.$unk.'</td>
    </tr>
  ';
  // sources
  $source_website = 0;
  $source_reunite = 0;
  $source_pfif = 0;
  $q = "
    SELECT DISTINCT incident_id
    FROM `arrival_rate` ;
  ";
  $result = $global['db']->Execute($q);
  if($result === false) { daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "arrivals simple22 >> ".$q); }
  while(!$result == NULL && !$result->EOF) {
    $qq = "
      SELECT *
      FROM `arrival_rate`
      WHERE `incident_id` = '".$result->fields['incident_id']."'
      ORDER BY DATE(arrival_time) DESC
      LIMIT 1;
    ";
    $rr = $global['db']->Execute($qq);
    if($rr === false) { daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "arrivals simple22 >> ".$qq); }
    $source_website   += $rr->fields['source_website'];
    $source_reunite   += $rr->fields['source_reunite'];
    $source_pfif      += $rr->fields['source_pfif'];
    $result->MoveNext();
  }
  $tp = '';
  $tr .= '
    <tr><td colspan=2>&nbsp;</td></tr>
    <tr><td class="center banRowEven">from website</td  ><td class="center banRowEven">'.$source_website.'</td></tr>
    <tr><td class="center banRowOdd">from ReUnite</td   ><td class="center banRowOdd" >'.$source_reunite.'</td></tr>
    <tr><td class="center banRowEven">from PFIF</td      ><td class="center banRowEven" >'.$source_pfif.'</td></tr>
    '.$tp.'
  ';
  $out = '<table>'.$tr.'</table>';
  $global['xajax_res']->addAssign('statsDiv', 'innerHTML', $out);
  $global['xajax_res']->addScript("setTimeout(function() { $('.fadeFromZero3').animate({ opacity: 1 }, 500); }, 100);");
  return $global['xajax_res']->getXML();
}

// clear the ip ban list
function admin_clear_ban_list() {
  global $global;
  // clear list
  clear_ban_list();
  // update list
  $global['xajax_res']->addScript("admin_show_dashboard();");
  return $global['xajax_res']->getXML();
}
