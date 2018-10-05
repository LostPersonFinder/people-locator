<?
/**
 * @name     FaceMatch Monitor
 * @author   pl@miernicki.com
 * @about    Developed by the U.S. National Library of Medicine
 * @link     https://gitlab.com/tehk/people-locator
 * @license  https://gitlab.com/tehk/people-locator/blob/master/LICENSE
 */

$global['use_stream'] = true; // use lib_stream

global $tabid;
$tabid = isset($_GET['tabid']) ? $_GET['tabid'] : 0;
$tabid = "&tabid=".$tabid;

function taupo_fm_default() {
  global $global, $conf;
  $htmlMain = '
    <div class="four44 oneEM">
    <h2><center>FaceMatch Use by Event and Source</center></h2>
    <table id="regLog">
      <tr style="font-weight:bold">
        <td class="center">Event</td>
        <td class="center">Local Ingests</td>
        <td class="center greenTD">Web Ingests</td>
        <td class="center greenTD">Web Queries</td>
        <td class="center redTD">RU iOS Ingests</td>
        <td class="center redTD">RU iOS Queries</td>
        <td class="center blueTD">RU Android Ingests</td>
        <td class="center blueTD">RU Android Queries</td>
        <td class="center">Removes</td>
      </tr>
  ';
  $incidents = array();
  $names = array();
  $q = "
    select *
    from incident 
    WHERE incident_id IN (SELECT incident_id from facematch_log);
  ";
  $result = $global['db']->Execute($q);
  if($result === false) { daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "fm log 00 ((".$q."))"); }
  while(!$result == NULL && !$result->EOF) {
    $incidents[] = $result->fields["incident_id"];
    $names[]     = json_decode($result->fields["translations"])->en->name;
    $result->MoveNext();
  }
  $count = 1;
  $ilocalTotal = 0;
  $iwebTotal = 0;
  $iruiTotal = 0;
  $iruaTotal = 0;
  $qwebTotal = 0;
  $qruiTotal = 0;
  $qruaTotal = 0;
  $removesTotal = 0;
  foreach($incidents as $incident=>$id) {
    if(($count%2)==0) {
      $eoT = 'banRowEven';
    } else {
      $eoT = 'banRowOdd';
    }
    // count ingests local
    $q = "
      SELECT count(*)
      FROM `facematch_log`
      WHERE `service` = '0'
      AND `incident_id` = '".$id."'
      AND `source` = '0';
    ";
    $result = $global['db']->Execute($q);
    if($result === false) { daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "fm log 0 ((".$q."))"); }
    $row = $result->FetchRow();
    $ilocal = $row['count(*)'];
    $ilocalTotal += $ilocal;
    // count ingests website
    $q = "
      SELECT count(*)
      FROM `facematch_log`
      WHERE `service` = '0'
      AND `incident_id` = '".$id."'
      AND `source` = '1';
    ";
    $result = $global['db']->Execute($q);
    if($result === false) { daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "fm log 0 ((".$q."))"); }
    $row = $result->FetchRow();
    $iweb = $row['count(*)'];
    $iwebTotal += $iweb;
    // count ingests rui
    $q = "
      SELECT count(*)
      FROM `facematch_log`
      WHERE `service` = '0'
      AND `incident_id` = '".$id."'
      AND `source` = '2';
    ";
    $result = $global['db']->Execute($q);
    if($result === false) { daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "fm log 0 ((".$q."))"); }
    $row = $result->FetchRow();
    $irui = $row['count(*)'];
    $iruiTotal += $irui;
    // count ingests rua
    $q = "
      SELECT count(*)
      FROM `facematch_log`
      WHERE `service` = '0'
      AND `incident_id` = '".$id."'
      AND `source` = '3';
    ";
    $result = $global['db']->Execute($q);
    if($result === false) { daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "fm log 0 ((".$q."))"); }
    $row = $result->FetchRow();
    $irua = $row['count(*)'];
    $iruaTotal += $irua;
    // count queries web
    $q = "
      SELECT count(*)
      FROM `facematch_log`
      WHERE `service` = '1'
      AND `incident_id` = '".$id."'
      AND `source` = '1';
    ";
    $result = $global['db']->Execute($q);
    if($result === false) { daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "fm log 0 ((".$q."))"); }
    $row = $result->FetchRow();
    $qweb = $row['count(*)'];
    $qwebTotal += $qweb;
    // count queries ru ios
    $q = "
      SELECT count(*)
      FROM `facematch_log`
      WHERE `service` = '1'
      AND `incident_id` = '".$id."'
      AND `source` = '2';
    ";
    $result = $global['db']->Execute($q);
    if($result === false) { daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "fm log 0 ((".$q."))"); }
    $row = $result->FetchRow();
    $qrui = $row['count(*)'];
    $qruiTotal += $qrui;
    // count queries ru ios
    $q = "
      SELECT count(*)
      FROM `facematch_log`
      WHERE `service` = '1'
      AND `incident_id` = '".$id."'
      AND `source` = '3';
    ";
    $result = $global['db']->Execute($q);
    if($result === false) { daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "fm log 0 ((".$q."))"); }
    $row = $result->FetchRow();
    $qrua = $row['count(*)'];
    $qruaTotal += $qrua;
    // count removes
    $q = "
      SELECT count(*)
      FROM `facematch_log`
      WHERE `service` = '2'
      AND `incident_id` = '".$id."';
    ";
    $result = $global['db']->Execute($q);
    if($result === false) { daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "fm log 0 ((".$q."))"); }
    $row = $result->FetchRow();
    $removes = $row['count(*)'];
    $removesTotal += $removes;
    $htmlMain .= '
      <tbody>
      <tr>
        <td class="center '.$eoT.'">'.$names[$incident].'</td>
        <td class="center '.$eoT.'">'.$ilocal.'</td>
        <td class="center '.$eoT.'">'.$iweb.'</td>
        <td class="center '.$eoT.'">'.$qweb.'</td>
        <td class="center '.$eoT.'">'.$irui.'</td>
        <td class="center '.$eoT.'">'.$qrui.'</td>
        <td class="center '.$eoT.'">'.$irua.'</td>
        <td class="center '.$eoT.'">'.$qrua.'</td>
        <td class="center '.$eoT.'">'.$removes.'</td>
      </tr>
      </tbody>
    ';
    $count++;
  }
  // Print out totals row.
  if(($count%2)==0) {
    $eoT = 'banRowEven';
  } else {
    $eoT = 'banRowOdd';
  }
  $htmlMain .= '
    <tfooter>
    <tr style="font-weight:bold">
      <td class="center '.$eoT.'">Totals</td>
      <td class="center '.$eoT.'">'.$ilocalTotal.'</td>
      <td class="center '.$eoT.'">'.$iwebTotal.'</td>
      <td class="center '.$eoT.'">'.$qwebTotal.'</td>
      <td class="center '.$eoT.'">'.$iruiTotal.'</td>
      <td class="center '.$eoT.'">'.$qruiTotal.'</td>
      <td class="center '.$eoT.'">'.$iruaTotal.'</td>
      <td class="center '.$eoT.'">'.$qruaTotal.'</td>
      <td class="center '.$eoT.'">'.$removesTotal.'</td>
    </tr>
    </tfooter>
    ';
  $htmlMain .= '</table></div>';

  $errors = '';
  $fm_error_log = "/pl/facematch/logs/".$conf['db_name']."_fm_error.log";
  if (file_exists ($fm_error_log)) {
    $errors = file_get_contents($fm_error_log);
  }
  $htmlMain .= '<div class="modHolder"><div class="oldDiv fmL2"><h5>Log errors:</h5><div class="solrlog">'.htmlspecialchars($errors).'</div></div>';
  echo $htmlMain;
}
