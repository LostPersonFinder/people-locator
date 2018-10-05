<?
/**
 * @name     abuse
 * @author   pl@miernicki.com
 * @about    Developed by the U.S. National Library of Medicine
 * @link     https://gitlab.com/tehk/people-locator
 * @license  https://gitlab.com/tehk/people-locator/blob/master/LICENSE
 */

global $global;
$global['xajax_functions'] = array();
array_push($global['xajax_functions'], 'abuse_show');
array_push($global['xajax_functions'], 'abuse_disregard');
array_push($global['xajax_functions'], 'abuse_delete');
require_once($conf['approot'].'mod/abuse/main.inc');

// reloads the mainArea div with information of what pages we have
function abuse_show() {
  global $global, $conf;
  $htmlMain = "";
  $eo = 0;
  $count = 0;
  // create table for abuse requests
  $htmlMain .= '
    <center><h1 style="font-size: 140%;">Abuse</h1>
    <table class="abuseT" style="table-layout: fixed; width: 100%">
      <tr>
        <td class="center" style="font-size: 120%;" ><b>UUID</td>
        <td class="center" style="font-size: 120%;" ><b>Type</td>
        <td class="center" style="font-size: 120%;" ><b>Actions</b></td>
      </tr>
  ';
  // get list of requests
  $q = "
    SELECT *
    FROM abuse
    ORDER BY p_uuid;
  ";
  $result = $global['db']->Execute($q);
  if($result === false) { daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "abuse show list 1 ((".$q."))"); }
  while($row = $result->FetchRow() ){
    $count++;
    switch($row['type']) {
      case 0:
        $ttype = 'Spam';
        break;
      case 1:
        $ttype = 'Sexually Explicit';
        break;
      case 2:
        $ttype = 'Hate Speech';
        break;
      case 3:
        $ttype = 'Violence';
        break;
      default:
        $ttype = 'Something Else';
    }
    $htmlMain .= '
    <tr>
      <td class="center">'.$row['p_uuid'].'</td>
      <td class="center">'.$ttype.'</td>
      <td class="center">
    ';
    $htmlMain .= "
        <paper-button class=\"bgGreen\" onclick=\"abuse_disregard('".$row['p_uuid']."');\">IGNORE</paper-button>
        <a href=\"".makeRecordUrl($row['p_uuid'])."?z=q\" target=\"_blank\">
          <paper-button class=\"bgYellow\">VIEW</paper-button>
        </a>
        <paper-button class=\"bgRed\" onclick=\"abuse_delete('".$row['p_uuid']."');\">DELETE</paper-button>
    </tr>
  ";
  }
  if($count == 0) { $htmlMain = "<center><h1 style=\"font-size: 140%;\">Abuse</h1>ZERO abuse reports pending."; }
  $htmlMain .= "</table>";
  $global['xajax_res']->addAssign('rezMain', 'innerHTML', $htmlMain);
  return $global['xajax_res']->getXML();
}

function abuse_disregard($p_uuid) {
  error_log($p_uuid);
  global $global;
  // get list of requests
  $q = "
    DELETE
    FROM abuse
    WHERE p_uuid = ".$global['db']->qstr($p_uuid)." ;
  ";
  $result = $global['db']->Execute($q);
  if($result === false) { daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "abuse disegard request 1 ((".$q."))"); }
  $global['xajax_res']->addScript("toast3('Abuse request ignored.'); abuse_show();");
  return $global['xajax_res']->getXML();
}

function abuse_delete($p_uuid) {
  global $global;
  // get list of requests
  $q = "
    DELETE
    FROM abuse
    WHERE p_uuid = ".$global['db']->qstr($p_uuid)." ;
  ";
  $result = $global['db']->Execute($q);
  if($result === false) {  daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "abuse delete request 1 ((".$q."))"); }
  // delete person record
  $p = new person();
  $p->p_uuid = $p_uuid;
  $p->load();
  // audit
  taupo_acl_log('RECORD_DELETE', 'user '.$_SESSION['user'].' deleted a record via abuse queue');
  $p->delete();
  $global['xajax_res']->addScript("toast('Record deleted.'); abuse_show(); ");
  return $global['xajax_res']->getXML();
}
