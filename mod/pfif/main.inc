<?
/**
 * @name     Person Finder Interchange Format
 * @author   pl@miernicki.com
 * @about    Developed by the U.S. National Library of Medicine
 * @link     https://gitlab.com/tehk/people-locator
 * @license  https://gitlab.com/tehk/people-locator/blob/master/LICENSE
 */

global $global, $conf;
$global['use_stream'] = true; // use lib_stream
require_once('pfif.inc');

// default controller function
function taupo_pfif_default() {
  global $global, $conf;
  $control = "";
  ?>
  <div id="menu2">
    <a href="pfif?status">PFIF Status</a> // 
    <a href="pfif?archive">PFIF Archive</a> //
    <a href="pfif?import">Import</a> //
    <a href="pfif?export">Export</a>
  </div>
  <?
  if(isset($_GET['status']))      { $control = "taupo_pfif_status";  }
  elseif(isset($_GET['archive'])) { $control = "taupo_pfif_archive"; }
  elseif(isset($_GET['import']))  { $control = "taupo_pfif_import";  }
  elseif(isset($_GET['export']))  { $control = "taupo_pfif_export";  }
  if($control == "")              { $control = "taupo_pfif_status";  }
  echo '<div id="adminHolder">';
  echo '<style>'.file_get_contents($conf['approot']."mod/pfif/pfif.css").'</style>';
  $control();
  echo '</div>';
}

// manage interactive PFIF import
function taupo_pfif_import() {
  global $global;
  // If there is no post, render the form to upload.
  if(isset( $_POST['upload'])) {
    if( $_FILES['xml']['error'] != 0){
      // Validate
      echo "<script>$(document).ready(function(){ toast('Please enter a filename to upload.'); });</script>";
      //unset($_FILES); unset($_POST);
      //taupo_pfif_import();
    } else {
      // If there is a post, upload the file and import the file to the database.
      if (is_uploaded_file($_FILES['xml']['tmp_name'])) {
        $pfif = new Pfif();
        $loaded = $pfif->loadFromXML($_FILES['xml']['tmp_name']);
        if ($loaded > 0) {
          //$xml = $pfif->storeInXML();
          //print "<pre>$xml</pre>";
          $stored = $pfif->storeInDatabase($_POST['pfif_incident_id']);
          // Get incident name.
          $sql = "SELECT translations from incident where incident_id=".$_POST['pfif_incident_id'];
          $res = $global['db']->GetRow($sql);
          $name = json_decode($res['translations'])->en->name;
          if($stored['person']==0 && $stored['note']==0) {
            echo "<script>$(document).ready(function(){ toast('No records were imported into the ".$name." event.'); });</script>";
          } else {
            echo "<script>$(document).ready(function(){ toast3('Successfully imported ".$stored['person']." persons and ".$stored['note']." notes into the ".$name." event.'); });</script>";
          }
        } else {
          echo "<script>$(document).ready(function(){ toast('File not loaded. Please confirm that it contains valid XML.'); });</script>";
        }
      }
    }
  }
  echo '<div id="importPFIF">Import from PFIF File<br>';
  // Form returns here so use same url.
  echo '<form id="importForm" method="post" enctype="multipart/form-data" action="'.htmlspecialchars($_SERVER['REQUEST_URI']).'" >';
  echo '<input type="file" name="xml" id="xml" tabindex="0" max_file_size="128M">';
  echo '&nbsp;&nbsp;<label for="xml">Filename</label><br>';
  $incident_id = (isset($_POST['pfif_incident_id']))? $_POST['pfif_incident_id']:1;
  taupo_pfif_incident_select("Event", $incident_id);
  echo "<br>";
  echo '<input type="hidden" name="upload" value="upload">';
  echo '<div id="submitButton">';
  echo '<paper-button raised="" id="importSubmit" class="buttonEM bgGreen" role="button" onclick="document.getElementById(\'importForm\').submit();">Import</paper-button>';
  echo '</div></form><div>';
}

// note: this could be made a more generic "taupo_incident_select" if others could benefit from it
function taupo_pfif_incident_select($label, $incident_id=1) {
  global $global;
  ?><select name="pfif_incident_id" id="pfif_incident_id" tabindex="0"><?php
  $query1 = "SELECT incident_id, translations FROM incident";
  $result1 = $global['db']->Execute($query1);
  while($row1 = $result1->FetchRow()){
    ?><option value="<?php echo $row1['incident_id'];?>"  <?php echo ($incident_id==$row1['incident_id']?'selected="selected"':'');?> >&nbsp;&nbsp;&nbsp;<?php echo json_decode($row1['translations'])->en->name;?></option><?php
  }
  ?></select>
  &nbsp; <label for="pfif_incident_id"><?php echo $label;?></label>
  <?php
}

// manage interactive PFIF Export
function taupo_pfif_export() {
  global $global;
  echo '<div id="exportPFIF">Export to PFIF<br>';
  // Form returns here so use same url.
  echo '<form id="exportForm" method="post" action="'.htmlspecialchars($_SERVER['REQUEST_URI']).'" >';
  $incident_id = (isset($_POST['pfif_incident_id']))? $_POST['pfif_incident_id']:1;
  taupo_pfif_incident_select("Event", $incident_id);
  echo "<br>";
  if (isset($_POST['since'])) {
    $value = $_POST['since'];
  } else {
    $value = date("Y")."-01-01 00:00:00";
  }
  echo '<input type="text" class="exporttext" name="since" id="since" tabindex="0" value="'.htmlspecialchars($value).'">';
  echo ' &nbsp; <label for="since">Start</label><br>';
  if (isset($_POST['before'])) {
    $value = $_POST['before'];
  } else {
    $value = date("Y-m-d h:i:s");
  }
  echo '<input type="text" class="exporttext" name="before" id="before" tabindex="0" value="'.htmlspecialchars($value).'">';
  echo ' &nbsp; <label for="before">End</label><br>';
  $checked = (isset($_POST['original']))? "checked":"";
  echo '<input type="checkbox" id="original" name="original" tabindex="0" '.$checked.'>';
  echo '&nbsp; Records originating in PL only<br>';
  echo '<input type="hidden" name="download" value="download">';
  echo '<div style="clear:both"></div>';
  echo '<div id="submitButton">';
  echo '<paper-button raised="" id="exportSubmit" class="buttonEM bgGreen" role="button" onclick="document.getElementById(\'exportForm\').submit();">Export</paper-button>';
  echo '</div></form><div><br>';
  // If this is due to a form submission, perform export.
  if(isset($_POST['download'])) {
    $since = $_POST['since'];
    $before = $_POST['before'];
    // Get incident name.
    $sql = "SELECT translations from incident where incident_id=".$_POST['pfif_incident_id'];
    $res = $global['db']->GetRow($sql);
    $name = json_decode($res['translations'])->en->name;
    $pfif = new Pfif();
    $pfif->setIncidentId($_POST['pfif_incident_id']);
    $original = isset($_POST['original']) ? true : false;
    $pfif->loadFromDatabase($since, $before);
    if ($original) {
      // Export only original records.
      $xml = $pfif->storeInXML(true, $original);
    } else {
      $xml = $pfif->storeInXML();
    }
    if ($xml == null) {
      $xml = "No records matching export constraints.";
    }
      echo "<div style=\"font-size: 110%;\">New ".($original? '<b>original</b>':'')." records in <b>$name</b> between <b>".htmlspecialchars($since)."</b> and <b>".htmlspecialchars($before)."</b> (use Ctrl-A/Ctrl-C to copy to clipboard):</div>";
    echo "<br><div contentEditable=\"true\" spellcheck=\"false\" id=\"exportArea\"><pre>".htmlspecialchars($xml)."</pre></div>";
  }
}

// return overall import/export system status
function taupo_pfif_statusGood() {
  // Check DB end times.
  return check_db_end_times();
}

// check that end_times in all active logs are current
function check_db_end_times() {
   global $global;
   // Check end times of all active person imports.
   $q = "
    SELECT (
      SELECT end_time
      FROM pfif_harvest_person_log ph
      WHERE ph.repository_id = pr.id
      ORDER BY log_index
      DESC LIMIT 1
    ) AS end_time
    FROM pfif_repository pr 
    WHERE role = 'source' 
    AND resource_type = 'person'
    AND pr.sched_interval_minutes != 0 
  ";
  $result = $global['db']->Execute($q);
  while ($row = $result->FetchRow()) {
    if (!check_end_time($row['end_time'])) {
      return 0;
    }
  }
  // Check end times of all active note imports.
  $q = "
    SELECT (
      SELECT end_time
      FROM pfif_harvest_note_log ph
      WHERE ph.repository_id = pr.id
      ORDER BY log_index
      DESC LIMIT 1
    ) AS end_time 
    FROM pfif_repository pr 
    WHERE role = 'source' 
    AND resource_type = 'note'
    AND pr.sched_interval_minutes != 0 
  ";
  $result = $global['db']->Execute($q);
  while ($row = $result->FetchRow()) {
    if (!check_end_time($row['end_time'])) {
      return 0;
    }
  }
  // Check end times of all active exports.
  $q = "
    SELECT (
      SELECT end_time
      FROM pfif_export_log pe
      WHERE pe.repository_id = pr.id
      ORDER BY log_index
      DESC LIMIT 1
    ) AS end_time
    FROM pfif_repository pr 
    WHERE role = 'sink' 
    AND pr.sched_interval_minutes != 0 
  ";
  $result = $global['db']->Execute($q);
  while($row = $result->FetchRow()) {
    if (!check_end_time($row['end_time'])) {
      return 0;
    }
  }
  return 1;
}

// add Google PFIF repository for event if it doesn't already exist
function taupo_pfif_add_repository($repoName, $eventId) {
  global $global;
  // if the repository already exists, we are done.
  $q = "
    SELECT *
    FROM pfif_repository
    WHERE incident_id = ".$eventId."
    AND name LIKE 'google%'
    LIMIT 1
  ";
  $result = $global['db']->Execute($q);
  if($result === false) { daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "repo 1"); }
  if ($row = $result->FetchRow()) return;
  $q = "
    INSERT INTO pfif_repository (name,role,resource_type,incident_id)
    VALUES ('".$repoName."','source','person',".$eventId.");
  ";
  $result = $global['db']->Execute($q);
  if($result === false) { daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "repo 2"); }
  $q = "
    INSERT INTO pfif_repository (name,role,resource_type,incident_id)
    VALUES ('".$repoName."','source','note',".$eventId.");
  ";
  $result = $global['db']->Execute($q);
  if($result === false) { daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "repo 3"); }
  $q = "
    INSERT INTO pfif_repository (name,role,resource_type,incident_id)
    VALUES ('".$repoName."','sink','both',".$eventId.");
  ";
  $result = $global['db']->Execute($q);
  if($result === false) { daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "repo 4"); }
}

// update Google PFIF repository for an event
function taupo_pfif_update_repository($repoName, $eventId, $base_url, $pfSync) {
  global $global;
  $q = "
    UPDATE pfif_repository SET
      name            = '".$repoName."',
      base_url        = '".$base_url."',
      sched_interval_minutes = '".$pfSync."'
    WHERE incident_id = '".$eventId."'
    AND name LIKE 'google%'
  ";
  $result = $global['db']->Execute($q);
  if($result === false) { daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "repo 5"); }
}

// Purge any PFIF export/import logs for an incident.
function taupo_pfif_purge_logs($incident_id) {
  global $global;
  // purge export logs for this event.
  $q = "
    DELETE pe FROM pfif_export_log pe, pfif_repository pr 
    WHERE pr.incident_id = $incident_id
    AND pe.repository_id = pr.id;
  ";
  $st = $global['db']->Execute($q);
  if($st === false) { daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "purge 1"); }
  // purge import logs for this event.
  $q = "
    DELETE php FROM pfif_harvest_person_log php, pfif_repository pr 
    WHERE pr.incident_id = $incident_id
    AND php.repository_id = pr.id;
  ";
  $st = $global['db']->Execute($q);
  if($st === false) { daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "purge 2"); }
  $q = "
    DELETE phn FROM pfif_harvest_note_log phn, pfif_repository pr 
    WHERE pr.incident_id = $incident_id
    AND phn.repository_id = pr.id;
  ";
  $st = $global['db']->Execute($q);
  if($st === false) { daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "purge 3"); }
  // reset repository records for this event.
  $q = "
    UPDATE pfif_repository SET first_entry=NOW(), last_entry=NOW(), total_persons=0, total_notes=0 
    WHERE incident_id = $incident_id
  ";
  $st = $global['db']->Execute($q);
  if($st === false) { daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "purge 4"); }
}

// show status of imports and exports
function taupo_pfif_status() {
   global $global;
   echo "<div id=\"home\" class=\"oneEM\">";
   $goodStatusMsg = "<span style=\"color: green;\"><b>Nominal</b></span>";
   $badStatusMsg = "<span style=\"color: red;\"><b>Failure</b></span>";
   // Show status of active person imports.
   $q = "
    SELECT pr.base_url, (
      SELECT end_time
      FROM pfif_harvest_person_log ph
      WHERE ph.repository_id = pr.id
      ORDER BY log_index
      DESC LIMIT 1
    ) AS end_time, pr.first_entry, pr.last_entry, pr.total_persons, i.shortname 
    FROM pfif_repository pr, incident i 
    WHERE role = 'source'
    AND resource_type = 'person'
    AND pr.incident_id = i.incident_id 
    AND pr.sched_interval_minutes != 0 
    ORDER BY pr.first_entry DESC
  ";
  $result = $global['db']->Execute($q);
  $count = 0;
  echo "
    <h2><center>Active Imports and Exports</center></h2>
    <h4><center>Person Imports</center></h4>
    <table id=\"regLog\">
      <tr>
        <td class=\"evener\"><b>PFIF Repository</b></td>
        <td class=\"evener\"><b>PL Event</b></td>
        <td class=\"evener\"><b>Status</b></td>
        <td class=\"evener\"><b>First Processed Record</b></td>
        <td class=\"evener\"><b>Last Processed Record</b></td>
        <td class=\"evener\"><b>Total Imported Records</b></td>
      </tr>
 ";
  while($row = $result->FetchRow()) {
    if(($count%2)==0) {
      $odd = "class=\"odder\"";
    } else {
      $odd = "class=\"evener\"";
    }
    $status = (check_end_time($row['end_time']))? $goodStatusMsg : $badStatusMsg;
    echo "
    <tr>
      <td ".$odd.">".substr_replace($row['base_url'], '', 0, 8)."</td>
      <td ".$odd.">".$row['shortname']."</td>
      <td ".$odd.">".$status."</td>
      <td ".$odd.">".(($row['first_entry']=='')? '-':$row['first_entry'])."</td>
      <td ".$odd.">".(($row['last_entry']=='')? '-':$row['last_entry'])."</td>
      <td ".$odd.">".$row['total_persons']."</td>
    </tr>";
    $count++;
  }
  if($count==0) {
    echo "<tr><td colspan=7 class=\"odder\">No repositories!</td></tr>";
  }
  echo '</table>';
  // show status of active note imports.
  $q = "
    SELECT pr.base_url, (
      SELECT end_time
      FROM pfif_harvest_note_log ph
      WHERE ph.repository_id = pr.id
      ORDER BY log_index DESC LIMIT 1
    ) AS end_time, pr.first_entry, pr.last_entry, pr.total_notes, i.shortname 
    FROM pfif_repository pr, incident i 
    WHERE role = 'source' 
    AND resource_type = 'note'
    AND pr.incident_id = i.incident_id 
    AND pr.sched_interval_minutes != 0 
    ORDER BY pr.first_entry DESC
  ";
  $result = $global['db']->Execute($q);
  $count = 0;
  echo "
    <br><br>
    <h4><center>Note Imports</center></h4>
    <table id=\"regLog\">
      <tr>
        <td class=\"evener\"><b>PFIF Repository</b></td>
        <td class=\"evener\"><b>PL Event</b></td>
        <td class=\"evener\"><b>Status</b></td>
        <td class=\"evener\"><b>First Processed Record</b></td>
        <td class=\"evener\"><b>Last Processed Record</b></td>
        <td class=\"evener\"><b>Total Imported Records</b></td>
      </tr>
  ";
  while($row = $result->FetchRow()) {
  if(($count%2)==0) {
    $odd = "class=\"odder\"";
  } else {
    $odd = "class=\"evener\"";
  }
  $status = (check_end_time($row['end_time']))? $goodStatusMsg : $badStatusMsg;
  echo "
    <tr>
      <td ".$odd.">".substr_replace($row['base_url'], '', 0, 8)."</td>
      <td ".$odd.">".$row['shortname']."</td>
      <td ".$odd.">".$status."</td>
      <td ".$odd.">".(($row['first_entry']=='')? '-':$row['first_entry'])."</td>
      <td ".$odd.">".(($row['last_entry']=='')? '-':$row['last_entry'])."</td>
      <td ".$odd.">".$row['total_notes']."</td>
    </tr>";
    $count++;
  }
  if($count==0) {
    echo "<tr><td colspan=7 class=\"odder\">No repositories!</td></tr>";
  }
  echo '</table>';
  // show status of active exports
   $q = "
    SELECT pr.base_url, (
      SELECT end_time
      FROM pfif_export_log pe
      WHERE pe.repository_id = pr.id
      ORDER BY log_index DESC LIMIT 1
    ) AS end_time, pr.first_entry, pr.last_entry, pr.total_persons, pr.total_notes, pr.resource_type, i.shortname 
    FROM pfif_repository pr, incident i 
    WHERE role = 'sink' 
    AND pr.incident_id = i.incident_id 
    AND pr.sched_interval_minutes != 0 
    ORDER BY pr.first_entry DESC
  ";
  $result = $global['db']->Execute($q);
  $count = 0;
  echo "
    <br><br>
    <h4><center>Exports</center></h4>
    <table id=\"regLog\">
      <tr>
        <td class=\"evener\"><b>PFIF Repository</b></td>
        <td class=\"evener\"><b>PL Event</b></td>
        <td class=\"evener\"><b>Status</b></td>
        <td class=\"evener\"><b>First Processed Record</b></td>
        <td class=\"evener\"><b>Last Processed Record</b></td>
        <td class=\"evener\"><b>Total Exported Persons</b></td>
        <td class=\"evener\"><b>Total Exported Notes</b></td>
      </tr>
  ";
  while($row = $result->FetchRow()) {
  if(($count%2)==0) {
    $odd = "class=\"odder\"";
  } else {
    $odd = "class=\"evener\"";
  }
  $status = (check_end_time($row['end_time']))? $goodStatusMsg : $badStatusMsg;
  echo "
    <tr>
      <td ".$odd.">".substr_replace($row['base_url'], '', 0, 8)."</td>
      <td ".$odd.">".$row['shortname']."</td>
      <td ".$odd.">".$status."</td>
      <td ".$odd.">".(($row['first_entry']=='')? '-':$row['first_entry'])."</td>
      <td ".$odd.">".(($row['last_entry']=='')? '-':$row['last_entry'])."</td>
      <td ".$odd.">".$row['total_persons']."</td>
      <td ".$odd.">".$row['total_notes']."</td>
    </tr>";
    $count++;
  }
  if($count==0) {
    echo "<tr><td colspan=7 class=\"odder\">No repositories!</td></tr>";
  }
  echo "</table>";
  echo "</div>";
}

// show inactive imports and exports
function taupo_pfif_archive() {
  global $global;
  echo "<div id=\"home\" class=\"oneEM\">";
  // show inactive imports from Google PF repositories.
  $q = "
    SELECT pr.base_url, pr.sched_interval_minutes, pr.first_entry, pr.last_entry, pr.total_persons, pr.total_notes, pr.resource_type, i.shortname
    FROM pfif_repository pr, incident i
    WHERE role = 'source'
    AND pr.name LIKE 'google%' AND pr.base_url != ''
    AND pr.incident_id = i.incident_id
    AND pr.sched_interval_minutes = 0
    ORDER BY pr.first_entry DESC
  ";
  $result = $global['db']->Execute($q);
  $count = 0;
  echo "
    <h2><center>Inactive Imports and Exports</center></h2>
    <h4><center>Imports</center></h4>
    <table id=\"regLog\">
      <tr>
        <td class=\"evener\"><b>PFIF Repository</b></td>
        <td class=\"evener\"><b>PL Event</b></td>
        <td class=\"evener\"><b>Record Type</b></td>
        <td class=\"evener\"><b>First Processed Record</b></td>
        <td class=\"evener\"><b>Last Processed Record</b></td>
        <td class=\"evener\"><b>Total Imported Records</b></td>
      </tr>
  ";
  while($row = $result->FetchRow()) {
    if(($count%2)==0) {
      $odd = "class=\"odder\"";
    } else {
      $odd = "class=\"evener\"";
    }
    if($row['resource_type']=='person' || $row['resource_type']=='both') {
      $total = $row['total_persons'];
    } else {
      $total = $row['total_notes'];
    }
    echo "
      <tr>
        <td ".$odd.">".substr_replace($row['base_url'], '', 0, 8)."</td>
        <td ".$odd.">".$row['shortname']."</td>
        <td ".$odd.">".$row['resource_type']."</td>
        <td ".$odd.">".(($row['first_entry']=='')? '-':$row['first_entry'])."</td>
        <td ".$odd.">".(($row['last_entry']=='')? '-':$row['last_entry'])."</td>
        <td ".$odd.">".$total."</td>
      </tr>
    ";
    $count++;
  }
  if($count==0) {
    echo "<tr><td colspan=7 class=\"odder\">No repositories!</td></tr>";
  }
  echo '</table>';
  // show inactive exports to Google PF repositories.
  $q = "
    SELECT pr.base_url, pr.sched_interval_minutes, pr.first_entry, pr.last_entry, pr.total_persons, pr.total_notes, i.shortname
    FROM pfif_repository pr, incident i
    WHERE role = 'sink'
    AND pr.name LIKE 'google%'
    AND pr.base_url != ''
    AND pr.incident_id = i.incident_id
    AND pr.sched_interval_minutes = 0
    ORDER BY pr.first_entry DESC
  ";
  $result = $global['db']->Execute($q);
  $count = 0;
  echo "
    <br><br>
    <h4><center>Exports</center></h4>
    <table id=\"regLog\">
      <tr>
        <td class=\"evener\"><b>PFIF Repository</b></td>
        <td class=\"evener\"><b>PL Event</b></td>
        <td class=\"evener\"><b>First Processed Record</b></td>
        <td class=\"evener\"><b>Last Processed Record</b></td>
        <td class=\"evener\"><b>Total Exported Persons</b></td>
        <td class=\"evener\"><b>Total Exported Notes</b></td>
      </tr>
  ";
  while($row = $result->FetchRow()) {
    if(($count%2)==0) {
      $odd = "class=\"odder\"";
    } else {
      $odd = "class=\"evener\"";
    }
    echo "
      <tr>
        <td ".$odd.">".substr_replace($row['base_url'], '', 0, 8)."</td>
        <td ".$odd.">".$row['shortname']."</td>
        <td ".$odd.">".(($row['first_entry']=='')? '-':$row['first_entry'])."</td>
        <td ".$odd.">".(($row['last_entry']=='')? '-':$row['last_entry'])."</td>
        <td ".$odd.">".$row['total_persons']."</td>
        <td ".$odd.">".$row['total_notes']."</td>
      </tr>
    ";
    $count++;
  }
  if($count==0) {
    echo "<tr><td colspan=7 class=\"odder\">No repositories!</td></tr>";
  }
  echo "</table>";
  echo "</div>";
}

// Is time of last activity within an acceptable delta
function check_end_time($time) {
  global $conf;
  $ts = strtotime($time);
  $delta = date("U") - $ts;
  $timeoutPeriod = 300;
  return (($delta < $timeoutPeriod) || $timeoutPeriod == 0);
}
