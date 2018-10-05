<?
/**
 * @name     Arrivals
 * @author   pl@miernicki.com
 * @about    Developed by the U.S. National Library of Medicine
 * @link     https://gitlab.com/tehk/people-locator
 * @license  https://gitlab.com/tehk/people-locator/blob/master/LICENSE
 */

$global['use_stream'] = true; // use lib_stream

function taupo_arrive_default() {
  global $global;
  $htmlMain = "";
  $count = 0;
  $allEvents = true;
  $htmlMain .= "
    <table class=\"emTable\">
      <tr>
        <td><b>Arrival Time</b></td>
        <td><b>Event</b></td>
        <td><b>Person UUID</b></td>
        <td><b>Person Name</b></td>
      </tr>
  ";
  // get list of last 100 arrivals...
  $q = "
    SELECT *
    FROM arrival_rate a, incident i, person_uuid p
    WHERE i.incident_id = a.incident_id
    AND p.p_uuid = a.person_uuid
    ORDER BY arrival_time DESC
    LIMIT 100;
  ";
  $result = $global['db']->Execute($q);
  if($result === false) { daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "arrive show list 1 ((".$q."))"); }
  while($row = $result->FetchRow()) {
    $count++;
    $htmlMain .= "
      <tr>
        <td>".$row['arrival_time']."</td>
        <td>".json_decode($row['translations'])->en->name."</td>
        <td>
          <a href=\"".makeRecordUrl($row['person_uuid'])."\" target=\"_blank\">
            <paper-button class=\"colored statusPass\" role=\"button\">".$row['person_uuid']."</paper-button>
          </a>
        </td>
        <td>".htmlspecialchars($row['full_name'])."</td>
      </tr>
    ";
  }
  if($count == 0) {
    $htmlMain .= "<tr><td colspan=4 class=\"mainRowEven\" style=\"text-align: center;\">No persons have arrived thus far.</td></tr>";
  }
  $htmlMain .= "</table>";
  ?>
  <div id="adminHolder" class="oneEM">
  <center><h1 style="font-size: 150%; margin: 0;">Last 100 arrivals from all events</h1></center>
  <div id="aMain" style="margin-top: 1em;" class="mainArea"></div>
  <? echo $htmlMain; ?>
  </div>
  <?
}
