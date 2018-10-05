<?
/**
 * @name     emailer
 * @author   pl@miernicki.com
 * @about    Developed by the U.S. National Library of Medicine
 * @link     https://gitlab.com/tehk/people-locator
 * @license   https://gitlab.com/tehk/people-locator/blob/master/LICENSE
 */

$global['use_stream'] = true; // use lib_stream

function taupo_email_default() {
  global $global;
  if(isset($_GET['pagenum'])) {
    $startPoint = (int)$_GET['pagenum']*50;
    $endPoint = $startPoint + 50;
    $msg = "showing messages ".($startPoint+1)." to ".$endPoint;
  } else {
    $startPoint = 0;
    $endPoint = $startPoint + 50;
    $msg = "showing last 50 messages";
  }
  if(isset($_GET['pagenum']) && (int)$_GET['pagenum'] > 0) {
    $previous = "<a href=\"email?pagenum=".((int)$_GET['pagenum']-1)."\">◄</a>";
  } else {
    $previous = "◄";
    $_GET['pagenum'] = 0;
  }
  $q = "
    SELECT count(*)
    FROM `emails`;
  ";
  $result = $global['db']->Execute($q);
  if($result === false) { daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "email 1 ((".$q."))"); }
  $row = $result->FetchRow();
  $totalCount = $row['count(*)'];

  if($endPoint < $totalCount) {
    $next = "<a href=\"email?pagenum=".($_GET['pagenum']+1)."\">►</a>";
  } else {
    $next = "►";
    $msg = "showing messages ".($startPoint+1)." to ".$totalCount;
  }
  $query  = "
    SELECT *
    FROM emails
    ORDER BY time_sent DESC
    LIMIT ".$startPoint.", 50;
  ";
  $result = $global['db']->Execute($query);
  if($result === false) { daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "email 2 ((".$q."))"); }
  $count = 0;
  echo "<div class=\"adminHolder four44 oneEM\"><h2><centeR>Outbound Email Log (".$msg." of ".$totalCount." total) ".$previous." ".$next." &nbsp; &nbsp; </h2>";
  echo "<table id=\"regLog\"><tr>
    <td class=\"evener\"><b>Module</b></td>
    <td class=\"evener\"><b>Date/Time</b></td>
    <td class=\"evener\"><b>Status</b></td>
    <td class=\"evener\"><b>Subject</b></td>
    <td class=\"evener\"><b>Recipients</b></td>
  </tr>";
  while($row = $result->FetchRow()) {
    if(($count%2) == 0) {
      $odd = "class=\"odder\"";
    } else {
      $odd = "class=\"evener\"";
    }
    if((string)trim($row['send_status']) != "SUCCESS") {
      $ss = "<span style=\"color: red; font-weight: bold;\">".$row['send_status']."</span>";
    } else {
      $ss = "<span style=\"color: rgb(0,255,0); font-weight: bold;\">".$row['send_status']."</span>";
    }
    echo "  <tr>
      <td ".$odd.">".$row['mod_accessed']."</td>
      <td ".$odd.">".$row['time_sent']."</td>
      <td ".$odd.">".$ss." ".$row['error_message']."</td>
      <td ".$odd.">".$row['email_subject']."</td>
      <td ".$odd.">".$row['email_recipients']."</td>
      </tr>";
    $count++;
  }
  if($count==0) {
    echo "<tr><td colspan=6 class=\"odder\">No emails sent yet..</td></tr>";
  }
  echo "</table>";
  echo "</fieldset></form></div></div>";
}

// report status
function taupo_email_statusGood() {
  global $global;
    $q = "
    SELECT count(*)
    FROM `emails`
    ORDER BY `emails`.`time_sent` DESC LIMIT 1;
  ";
  $result = $global['db']->Execute($q);
  if($result === false) { daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "email StatusGood 1 ((".$q."))"); }
  $row = $result->FetchRow();
  if($row['count(*)'] == 0) {
    return true;
  }
  $q = "
    SELECT *
    FROM `emails`
    ORDER BY `emails`.`time_sent` DESC LIMIT 1;
  ";
  $result = $global['db']->Execute($q);
  if($result === false) { daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "email StatusGood 1 ((".$q."))"); }
  $row = $result->FetchRow();
  $pass = $row['send_status'];
  if((string)trim($pass) == "SUCCESS" || $row['count(*)'] == 0) {
    return true;
  } else {
    return false;
  }
}
