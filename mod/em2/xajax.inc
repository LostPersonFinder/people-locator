<?
/**
 * @name     Event Manager 2
 * @author   pl@miernicki.com
 * @about    Developed by the U.S. National Library of Medicine
 * @link     https://gitlab.com/tehk/people-locator
 * @license  https://gitlab.com/tehk/people-locator/blob/master/LICENSE
 */

error_reporting(0);

global $global, $conf;
$global['xajax_functions'] = array();
require_once($conf['approot'].'mod/pfif/main.inc');
require_once($conf['approot'].'mod/em2/main.inc');

// publicly register accessible xajax funtions
array_push($global['xajax_functions'],'em2_show_events');
array_push($global['xajax_functions'],'em2_perform_edit');
array_push($global['xajax_functions'],'em2_perform_delete');
array_push($global['xajax_functions'],'em2_perform_purge');
array_push($global['xajax_functions'],'em2_perform_insert');
array_push($global['xajax_functions'],'em2_perform_save');
array_push($global['xajax_functions'],'em2_image_send');

// Reloads the mainArea div with event information.
function em2_show_events($col='date', $direction='up', $showArchived=0, $internal=FALSE) {
  global $conf, $global;
  $htmlLog   = "";
  $htmlMain  = "";
  $num       = 0;
  $order     = "date desc";  // default sort order
  $arrow0    = $arrow1 = $arrow2 = $arrow3 = $arrow4 = '';
  $downArrow = "&#x25BC;";
  $upArrow   = "&#x25B2;";
  $archivedClause = "";
  if(!$showArchived) { $archivedClause = " WHERE archived IS FALSE "; }
   // Sort on specified column.
  if($col == 'date') {
    if($direction == 'down') {
      $arrow0 = $downArrow; 
      $order = "date asc";
    } else {
      $arrow0 = $upArrow; 
      $order = "date desc";
    }
  } elseif($col == 'name') {
    if($direction == 'down') {
      $arrow1 = $downArrow; 
      $order = "name asc";
    } else {
      $arrow1 = $upArrow; 
      $order = "name desc";
    }
  } elseif($col == 'shortname') {
    if($direction == 'down') {
      $arrow2 = $downArrow; 
      $order = "shortname asc";
    } else {
      $arrow2 = $upArrow; 
      $order = "shortname desc";
    }
  } elseif($col == 'group') {
    if($direction == 'down') {
      $arrow3 = $downArrow; 
      $order = "private_group asc";
    } else {
      $arrow3 = $upArrow; 
      $order = "private_group desc";
    }
  } elseif($col == 'closed') {
    // Sort by reporting status.
    if($direction == 'down') {
      $arrow4 = $downArrow; 
      $order = "closed asc";
    } else {
      $arrow4 = $upArrow;
      $order = "closed desc";
    }
  }
  $htmlMain .= "
    <table class=\"emTable lessPadded\" style=\"border-spacing: 0 0px; border-collapse: separate;\">
      <tr>
        <td class=\"emColumn emHead\"><b>#</b></td>
        <td class=\"emColumn emHead\"><a href=\"javascript:void(0);\" onclick=\"em2_sort_events('date',".$showArchived.");\">Date Time (UTC) <span id=\"eventsColdate\">$arrow0</span></a></td>
        <td class=\"emColumn emHead\"><a href=\"javascript:void(0);\" onclick=\"em2_sort_events('name',".$showArchived.");\">Name <span id=\"eventsColname\">$arrow1</span></a></td>
        <td class=\"emColumn emHead\"><a href=\"javascript:void(0);\" onclick=\"em2_sort_events('shortname',".$showArchived.");\">Shortname <span id=\"eventsColshortname\">$arrow2</span></a></td>
        <td class=\"emColumn emHead center\"><a href=\"javascript:void(0);\" onclick=\"em2_sort_events('group',".$showArchived.");\">Event Class<span id=\"eventsColgroup\">$arrow3</span></a></td>
        <td class=\"emColumn emHead center\"><a href=\"javascript:void(0);\" onclick=\"em2_sort_events('closed',".$showArchived.");\">Reporting<span id=\"eventsColclosed\">$arrow4</span></a></td>
        <td class=\"emColumn emHead center\">&nbsp;</td>
      </tr>
  ";
  // get list of events
  $q = "
    SELECT *
    FROM   `incident`
    $archivedClause
    ORDER  BY $order;
  ";
  $result = $global['db']->Execute($q);
  if($result === false) { daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "show events 1"); }
  // find out how many disasters we have
  $disasters = array();
  $dcount = 0;
  while($row = $result->FetchRow()) {
    $disasters[$dcount] = $row;
    $dcount++;
  }
  $i = 1;
  while($i <= $dcount) {
    $num++;
    $row           = $disasters[$i-1];
    $disaster_id   = $row['incident_id'];
    $date          = $row['date'];
    $decoded       = json_decode($row['translations']);
    $longname      = $decoded->en->name;
    $shortname     = $row['shortname'];
    $private_group = $row['private_group'];
    $closed        = $row['closed'];
    $eArchived     = $row['archived'] ? 'mainRowArchived' : '';
    $eUnlisted     = $row['unlisted'] ? 'mainRowUnlisted' : '';
    // show group name or public if null group
    if($private_group == NULL) { $groupText = "PUBLIC"; }
    else                       { $groupText = 'Admin'; }
    // generate open close text
    if((int)$closed == 0)      { $closedText = "OPEN"; }
    else if((int)$closed == 1) { $closedText = "CLOSED"; }
    else if((int)$closed == 2) { $closedText = "CLOSED(PF)"; }
    else                       { $closedText = "OPEN"; } 
    // find class to color the row
    if($eUnlisted != '')     { $evenOddClass = "mainRowUnlisted"; }
    elseif($eArchived != '') { $evenOddClass = "mainRowArchived"; }
    else                     { $evenOddClass = ""; }
    $htmlMain .= "
      <tr>
        <td class=\"emColumn ".$evenOddClass."\">".$num."</td>
        <td class=\"emColumn ".$evenOddClass."\">".$date."</td>
        <td class=\"emColumn ".$evenOddClass."\">".$longname."</td>
        <td class=\"emColumn ".$evenOddClass."\">".$shortname."</td>
        <td class=\"emColumn center ".$evenOddClass."\">".$groupText."</td>
        <td class=\"emColumn center ".$evenOddClass."\">".$closedText."</td>
        <td class=\"emColumn center emright ".$evenOddClass."\">
          <paper-button class=\"emEditButton bgBlue\" onclick=\" em2_perform_edit(".$disaster_id.");\">Edit</paper-button>
        </td>
      </tr>
    ";
    $i++;
  }
  if($dcount == 0) { $htmlMain .= "<tr><td colspan=8 class=\"mainRowEven\" style=\"text-align: center;\">No Events have been created thus far.</td></tr>"; }
  $htmlMain .= "</table>";
  $pageControlHtml = "
    <paper-button class=\"buttonEM bgYellow emButtonNew\" role=\"button\" tabindex=\"0\" onclick=\"em2_perform_edit(0);\">Create a new Event</paper-button>
    &nbsp; &nbsp; &nbsp; &nbsp;
    <paper-checkbox id=\"showArchived\" class=\"mainRowArchived emButtonNew\" ".(($showArchived)? 'checked' : '')." onchange=\"em2_filter_events('".$col."','".$direction."');\">Show Archived</paper-checkbox>
  ";
  // internal // no ajax, just pass back
  if($internal) { return $htmlMain; }
  else {
    $global['xajax_res']->addAssign('rezControl','style.opacity','1.0');
    $global['xajax_res']->addAssign('rezControl','innerHTML',$pageControlHtml);
    $global['xajax_res']->addAssign('rezMain','style.opacity','1.0');
    $global['xajax_res']->addAssign('rezMain','innerHTML',$htmlMain);
    return $global['xajax_res']->getXML();
  }
}

function em2_perform_save($page_id, $rj) {
  global $global, $conf;
  $r       = json_decode($rj, true);
  $log     = "";
  $suggest = "";
  if($r['eventId'] == 'stub') {
    $q = "
      SELECT AUTO_INCREMENT
      FROM   information_schema.`TABLES`
      WHERE  `table_schema` = '".$conf['db_name']."'
      AND    `table_name` = 'incident';
    ";
    $result = $global['db']->Execute($q);
    if($result === false) { daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "em2 _ get index ".$q); }
    $row = $result->FetchRow();
    $eventId = $row['AUTO_INCREMENT'];
    // insert stub
    $q = "INSERT INTO `incident` (incident_id) VALUES(".$eventId.");";
    $result = $global['db']->Execute($q);
    if($result === false) { daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "class person image _ stub insert ".$q); }
    // insert empty row in arrival_rate table
    $q = "
      INSERT INTO arrival_rate (person_uuid, incident_id, arrival_time, source_all, source_triagepic, source_reunite, source_website, source_pfif, source_vanilla_email)
      VALUES (1, ".$eventId.", '2000-01-01 01:01:01', 0,0,0,0,0,0);
    ";
    $result = $global['db']->Execute($q);
    if($result === false) { daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "save 3a2b1"); }
    $isNew = true;
  } else {
    $eventId = (int)$r['eventId'];
    $isNew = false;
  }
  $name = $r['longName'];
  $name = strip_tags($name);
  $name = preg_replace('/\s+/', ' ', $name);
  $name = preg_replace('/[0-9]+/', '', $name);
  $name = rtrim($name);
  $name = ltrim($name);
  $shortName = $r['shortName'];
  $shortName = strip_tags($shortName);
  $shortName = trim($shortName);
  $shortName = preg_replace("/[^a-zA-Z0-9\s]/", "", $shortName);
  $shortName = strtolower($shortName);
  $article = $r['article'];
  $article = strip_tags($article);
  $article = preg_replace('/\s+/', ' ', $article);
  $images  = $r['images'];
  $caption = $r['caption'];
  $caption = strip_tags($caption);
  $caption = preg_replace('/\s+/', ' ', $caption);
  $tags = preg_replace('/\s+/', '', $r['tags']);
  $tags = strip_tags($tags);
  $lang = $r['lang'];
  $translation = '{"'.$lang.'":{"name":"'.$name.'","article":"'.$article.'","caption":"'.$caption.'","tags":"'.$tags.'"}}';
  if($r['shortName'] != $shortName) { $suggest .= "Event shortname revised to match rules: 1) lowercase letters and numbers only 2) 1-16 characters in length. "; }
  if(strlen($shortName) < 1 || strlen($shortName) > 16) {
    $suggest .= "Event shortname field was an improper length and has been revised. ";
    $shortName = "event".date("mdGis");
  }
  if($suggest != "") { $suggest .= "The new Event shortname is : ".$shortName." "; }
  $date       = $r['eventDate'];
  $pfKey      = isset($r['pfKey']) ? $r['pfKey'] : null;
  $pfSync     = isset($r['pfSync']) ? (int)$r['pfSync'] : 0;
  $archived   = isset($r['eventArchived']) ? (int)$r['eventArchived'] : 0;
  $unlisted   = isset($r['eventUnlisted']) ? (int)$r['eventUnlisted'] : 0;
  $street     = $r['street'];
  $streetE    = $r['street'];
  $latitude   = (double)$r['latitude'];
  $longitude  = (double)$r['longitude'];
  $adminotes  = $r['adminotes'];
  $adminotesE = $r['adminotes'];
  // reject if shortname already exists with another event
  $q = "SELECT * FROM incident WHERE shortname = '".$shortName."' AND incident_id != '".$eventId."'";
  $result = $global['db']->Execute($q);
  if($result === false) { daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "save event 0"); }
  if($row = $result->FetchRow()) {
    $global['xajax_res']->addScript("toast('An event with shortname \'".$shortName."\' already exists. Please choose another.')");
    return $global['xajax_res']->getXML();
  }
  // formulate type
  $type = $r['eventType'];
  if(($type != "REAL") && ($type != "TEST")) { $type = "TEST"; }
  // formulate default
  $default = (int)$r['eventDefault'];
  if($default == 1) {
    // return to non-default all other events if we have a new default
    $q = "
      UPDATE incident
      SET `default` = '0'
      WHERE incident_id != '".$eventId."';
    ";
    $result = $global['db']->Execute($q);
    if($result === false) { daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "save event 2"); }
    $row = $result->FetchRow();
    $default = "'1'";
    $defaultC = 1;
  } else {
    $default = "'0'";
    $defaultC = 0;
  }
  // check for bad values
  $closed = (int)$r['eventClosed'];
  if(($closed < 0) || ($closed > 2)) { $closed = 0; }
  // formulate group
  $group = (int)$r['eventVisibility'];
  if($group == -1) {
    $group = "NULL";
    $groupC = null;
  } else { $groupC = $group; }
  if(!$isNew) {
    $q = "
      SELECT *
      FROM incident
      WHERE incident_id = '".$eventId."'
      LIMIT 1;
    ";
    $result = $global['db']->Execute($q);
    if($result === false) { daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "audit email"); }
    $row = $result->FetchRow();
    $change = ''; // push body
    $changp = ''; // email body
    if($shortName != $row['shortname']) {
      $changp .= 'shortname ';
      $change .= "Shortname changed from: ".$row['shortname']." to: ".$shortName."<br>";
      taupo_solr_add_persons($eventId); // requires updating SOLR indexes
    }
    if($date != $row['date']) {
      $changp .= 'date '; 
      $change .= "Date changed from: ".$row['date']." to: ".$date."<br>";
    }
    if($type != $row['type']) {
      $changp .= 'type ';
      $change .= "Type changed from: ".$row['type']." to: ".$type."<br>";
    }
    if($latitude != $row['latitude'] || $longitude != $row['longitude']) {
      $changp .= 'location ';
      $change .= "Lat/Lon changed from: ".$row['latitude']."/".$row['longitude']." to: ".$latitude."/".$longitude."<br>";
    }
    if((int)$defaultC != (int)$row['default']) { 
      if($defaultC == 1) {
        $changp .= 'default ';
        $change .= "Changed to the default event for ReUnite.<br>";
      } else {
        $changp .= 'non-default ';
        $change .= "Changed to non-default event for ReUnite. Test Exercise is now set as the default event for ReUnite.<br>";
        setTestDefault(); // must be a default event
      }
    }
    if($groupC != $row['private_group']) {
      if($groupC == 1) {
        $changp .= 'admin-only ';
        $change .= "Event changed to ADMIN.<br>";
      } else {
        $changp .= 'public ';
        $change .= "Event changed to PUBLIC.<br>";
      }
    }
    if($closed != $row['closed']) {
      if($closed == 0) {
        $changp .= 'reporting-open ';
        $change .= "Event opened for reporting.<br>";
      } elseif($closed == 1) {
        $changp .= 'reporting-closed ';
        $change .= "Event closed for reporting.<br>";
      }
    }
    if($archived != $row['archived']) {
      if($archived == 1) {
        $changp .= 'archived ';
        $change .= "Event changed to archived.<br>";
        renamePFIFImages($eventId);
      } else {
        $changp .= 'unarchived ';
        $change .= "Event change to unarchived.<br>";
      }
    }
    if($unlisted != $row['unlisted']) {
      if($unlisted == 1) {
        $changp .= 'unlisted ';
        $change .= "Event changed to unlisted.<br>";
        renamePFIFImages($eventId);
      } else {
        $changp .= 'listed ';
        $change .= "Event changed to listed.<br>";
      }
    }
    if($images != $row['images']) {
      $changp .= 'image ';
      $change .= "Image changed.<br>";
    }
    $translations = $row['translations'];
    $decoded = json_decode($translations);
    if(property_exists($decoded, $lang)) {
      if($name != $decoded->$lang->name) {
        $changp .= 'name ';
        $change .= "Event name (".$lang.") changed from: '".$decoded->$lang->name."' to: '".$name."'<br>";
      }
      if($article!=$decoded->$lang->article) {
        $changp .= 'article ';
        $change .= "Article (".$lang.") changed from:<br>'".$decoded->$lang->article."'<br>to:<br>'".$article."'<br>";
      }
      if($tags != $decoded->$lang->tags) {
        $changp .= 'tags ';
        $change .= "Tags (".$lang.") changed from: '".$decoded->$lang->tags."' to: '".$tags."'<br>";
      }
    } else {
      $changp .= 'translation('.$lang.') ';
      $change .= "Added translation of multilingual fields for lang: '".$lang."'.<br>";
    }
    // get Google repository info
    $q = "
      SELECT SUBSTRING_INDEX(base_url, '/', -1) AS pfKey,sched_interval_minutes
      FROM pfif_repository
      WHERE incident_id = ".$eventId." AND name LIKE 'google%'
      LIMIT 1;
    ";
    $result = $global['db']->Execute($q);
    if($result === false) { daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "audit email 2"); }
    while($row = $result->FetchRow()) {
      $pfSyncDB = $row['sched_interval_minutes'] == "0" ? 0 : 1;
      if($pfSyncDB != $pfSync) {
        $changp .= 'PF-sync-'.(($pfSync == 0)?'off':'on').' ';
        $change .= "Google Person Finder imports/exports ".(($pfSync == 0)? "disabled":"enabled").".<br>";
      }
      if($pfKey != $row['pfKey']) {
        $changp .= 'PF-key ';
        $change .= "Google Person Finder key changed from: '".$row['pfkey']."' to: '".$pfKey."'<br>";
      }
    }
  }
  // save the revision
  $q = "
    UPDATE incident
    SET
      shortname       = ".$global['db']->qstr($shortName).",
      `date`          = ".$global['db']->qstr($date).",
      type            = '".$type."',
      latitude        = '".$latitude."',
      longitude       = '".$longitude."',
      `default`       = ".$default.",
      private_group   = ".$group.",
      closed          = ".$closed.",
      street          = ".$global['db']->qstr($street).",
      archived        = ".$archived.",
      unlisted        = ".$unlisted.",
      images          = ".$global['db']->qstr($images).",
      admin_notes     = ".$global['db']->qstr($adminotes)."
      WHERE 
      incident_id     = '".$eventId."';
  ";
  $result = $global['db']->Execute($q);
  if($result === false) { daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "save event 3b"); }
  // save language-specific fields in JSON format
  if($isNew) { $translations = $translation; }
  else {
    $decoded = json_decode($translations);
    if(property_exists($decoded, $lang)) {
      // Modify current translation.
      $decoded->$lang->name = $name;
      $decoded->$lang->article = $article;
      $decoded->$lang->caption = $caption;
      $decoded->$lang->tags = $tags;
    } else {
      // Add new translation.
      $decoded->$lang = new stdClass();
      $decoded->$lang->name = $name;
      $decoded->$lang->article = $article;
      $decoded->$lang->caption = $caption;
      $decoded->$lang->tags = $tags;
    }
    $translations = json_encode($decoded);
  }
  // use adodb prepared statements to avoid problems with quotes
  $sql_tran = "UPDATE incident SET translations = ? WHERE incident_id = ?";
  $result = $global['db']->Execute($sql_tran, array($translations, $eventId));
  if($result === false) { daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "save event 3c"); }
  // create default English translation if necessary
  if(!property_exists($decoded, "en")) {
      $decoded->en->name = $name;
      $decoded->en->article = $article;
      $decoded->en->caption = $caption;
      $decoded->en->tags = $tags;
      $translations = json_encode($decoded);
      $sql_tran = "UPDATE incident SET translations = ? WHERE incident_id = ?";
      $result = $global['db']->Execute($sql_tran,  array($translations, $eventId));
      if($result === false) { daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "save event 3d"); }
  }
  // PFIF
  $repoName = 'google'.$shortName;  // repository name = "google"+shortname
  $base_url = ($pfKey == null) ? '' : GOOGLE_PF.$pfKey;
  if($pfKey) {
    // create base entries in pfif_repository table if they don't exist
    taupo_pfif_add_repository($repoName, $eventId);
    // update the entries with edited information
    taupo_pfif_update_repository($repoName, $eventId, $base_url, $pfSync);
  }
  if($isNew) {
    // FaceMatch needs to know about new events
    if($conf['image_search'] == true) { 
      try { taupo_facematch_add_event($eventId); }
      catch (RuntimeException $e) { /* fm down */ }
    }
    // message admins
    taupo_message_admins('New event "'.$name.'" started by '.$_SESSION['user'], null, $conf['base_url'].'/en/events/'.$shortName);
    // audit
    taupo_acl_log('EVENT_NEW', 'user '.$_SESSION['user'].' created a new event: '.$name);
  } elseif($changp != '') {
    // message admins
    $msg  = 'Event "'.$name.'" modified by '.$_SESSION['user'].' changes: '.$changp; // push
    $msge = 'Event "'.$name.'" modified by '.$_SESSION['user'].'<br><br>'.$change; // email
    taupo_message_admins($msg, $msge, $conf['base_url'].'/en/events/'.$shortName);
    // audit
    taupo_acl_log('EVENT_MOD', 'user '.$_SESSION['user'].' modified event: '.$name);
  }
  $log = "Event #".$page_id." Saved.";
  if((int)$page_id == 0) { $log = "New event saved"; }
  // update log/ui
  $pageControlHtml = '
    <paper-button class="buttonEM bgYellow" role="button" tabindex="0" onclick="em2_perform_edit(0);">Create a new Event</paper-button>
    &nbsp; &nbsp; &nbsp; &nbsp;
    '."<paper-checkbox id=\"showArchived\" class=\"mainRowArchived emButtonNew\" onchange=\"em2_filter_events('date','up');\">Show Archived</paper-checkbox>
  ";
  $global['xajax_res']->addAssign('rezControl','innerHTML',$pageControlHtml);
  $global['xajax_res']->addAssign('rezMain', 'innerHTML', em2_show_events('date','up',0,true));
  $global['xajax_res']->addScript("toast3('".$log."')");
  $global['xajax_res']->addScript("em2_reset_lS()");
  return $global['xajax_res']->getXML();
}

// delete an event
function em2_perform_delete($incident_id) {
  global $global, $conf;
  // catch delete on newly created event
  if((int)$incident_id != 0) {
    // email audit
    $q = "
      SELECT *
      FROM `incident`
      WHERE incident_id = '".$incident_id."';
    ";
    $result = $global['db']->Execute($q);
    if($result === false) { daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "delete event 2"); }
    $row = $result->FetchRow();
    $name = json_decode($row['translations'])->en->name;
    $shortname = $row['shortname'];
    // message admins
    taupo_message_admins('Event "'.$name.'" deleted by '.$_SESSION['user'], null, $conf['base_url'].'/events');
    // purge the incident of persons and related image data
    $response = em2_perform_purge_do($incident_id);
    // Let FaceMatch know event is gone.
    if($conf['image_search'] == true) { try { taupo_facematch_delete_event($incident_id); } catch (RuntimeException $e) { /* fm down */ }}
    // master audit log the action
    taupo_acl_log('EVENT_DELETE', 'user '.$_SESSION['user'].' deleted event: '.$name);
    // delete the incident from the db
    $q = "
      DELETE from incident
      WHERE incident_id = '".$incident_id."';
    ";
    $result = $global['db']->Execute($q);
    if($result === false) { daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "delete 1"); }
    $global['xajax_res']->addScript("toast3('Event #".$incident_id." deleted')");
  } else { $global['xajax_res']->addScript("toast3('New empty event discarded')"); }
  // update UI
  $pageControlHtml = '
    <paper-button class="buttonEM bgYellow" role="button" tabindex="0" onclick="em2_perform_edit(0);">Create a new Event</paper-button>
    &nbsp; &nbsp; &nbsp; &nbsp;
    '."<paper-checkbox id=\"showArchived\" class=\"mainRowArchived emButtonNew\" onchange=\"em2_filter_events('date','up');\">Show Archived</paper-checkbox>
  ";
  $global['xajax_res']->addAssign('rezControl','style.opacity','1.0');
  $global['xajax_res']->addAssign('rezControl','innerHTML',$pageControlHtml);
  $global['xajax_res']->addAssign('rezMain', 'innerHTML', em2_show_events('date','up',0,true));
  //$global['xajax_res']->addScript("");
  return $global['xajax_res']->getXML();
}

// purge event
function em2_perform_purge($incident_id) {
  global $global, $conf;
  // catch purge on newly created event
  if((int)$incident_id != 0) {
    $q = "
      SELECT *
      FROM `incident`
      WHERE incident_id = '".$incident_id."';
    ";
    $result = $global['db']->Execute($q);
    if($result === false) { daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "delete event 2"); }
    $row = $result->FetchRow();
    $name = json_decode($row['translations'])->en->name;
    $shortname = $row['shortname'];
    // message admins
    taupo_message_admins('Event "'.$name.'" purged by '.$_SESSION['user'], null, $conf['base_url'].'/events');
    // audit
    taupo_acl_log('EVENT_PURGE', 'user '.$_SESSION['user'].' purged event: '.$name);
    // purge the incident of persons and related image data
    $response = em2_perform_purge_do($incident_id);
    $global['xajax_res']->addScript("toast3('Event #".$incident_id." purged')");
  } else { $global['xajax_res']->addScript("toast2('New empty event not purged')"); }
  return $global['xajax_res']->getXML();
}


// purge all persons from an event
function em2_perform_purge_do($incident_id) {
  global $global, $conf;
  $log = "";
  $delPersons = 0;
  // Get all persons for this incident
  $q = "
    SELECT *
    FROM person_uuid
    WHERE incident_id = '".$incident_id."';
  ";
  $result = $global['db']->Execute($q);
  if($result === false) { daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "purge 1");  return($log."error!<br>"); }
  $people = array();
  $count = 0;
  while($row = $result->FetchRow()) { $people[$count++] = $row['p_uuid']; }
  foreach($people as $p_uuid) {
    $p = new person();
    $p->p_uuid = $p_uuid;
    $p->load();
    $p->delete();
    $delPersons++;
    $log .= "Deleted <b>".$p_uuid."</b><br>";
  }
  // Purge any import/export DB logs for this event.
  taupo_pfif_purge_logs($incident_id);
  $log .= "<b>".$delPersons." person(s)</b> purged.<br>";
  // clear arrival rate table if not new event
  if($incident_id > 0) {
    $q = "
      DELETE FROM arrival_rate
      WHERE `incident_id` = '".$incident_id."';
    ";
    $result = $global['db']->Execute($q);
    if($result === false) {daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "save 3a2b2 >> ".$q); }
    // insert empty row in arrival_rate table
    $q = "
      INSERT INTO arrival_rate (person_uuid, incident_id, arrival_time, source_all, source_triagepic, source_reunite, source_website, source_pfif, source_vanilla_email)
      VALUES (1, ".$incident_id.", '2000-01-01 01:01:01', 0,0,0,0,0,0);
    ";
    $result = $global['db']->Execute($q);
    if($result === false) {daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "save 3a2b3  >> ".$q); }
  }
  return($log);
}

// presents a gui to edit a page and its corresponding fields
function em2_perform_edit($incident_id, $lang='en') {
  global $global, $conf;
  $new = false;
  // if incident_id = 0, must create a new incident then continue to edit it
  if($incident_id == 0) {
    $defaultArticleText = 'Two powerful bombs exploded near the epicenter of the event on Monday afternoon, killing three people, including a 99-year-old child, and injuring more than 100... Almost three-quarters of the 42,000 participants in the event had already finished when a bomb that had apparently been placed in a garbage dumpster exploded around 1:04 p.m. in a haze of smoke amid a crowd of spectators on Main Street, just off Town Square in the heart of the city.  Thirteen minutes later, another bomb exploded several hundred feet away. Pandemonium erupted as participants and spectators scattered, and rescue workers rushed in to care for the dozens of maimed and injured, some of whom lost items in the blast....';
    $random               = mt_rand();
    $new                  = true;
    $row['incident_id']   = 'stub'; 
    $name                 = "new event please change me pl".$random;
    $row['shortname']     = "pl".$random;
    $row['date']          = gmdate("Y-m-d H:i:s");
    $row['type']          = "TEST";
    $row['latitude']      = '38.897605758496255';
    $row['longitude']     = '-77.03658540942257';
    $row['default']       = null;
    $row['private_group'] = 1;
    $row['archived']      = 0;
    $row['unlisted']      = 0;
    $row['closed']        = 0;
    $row['street']        = null;
    $row['images']        = 'assets/images/events/earth7_566.jpg';
    $article              = $defaultArticleText;
    $caption              = null;
    $tags                 = null;
    $row['admin_notes']   = null;
    $rowpfif = null; 
  } else {
    $new = false;
    // get incident info
    $q = "
      SELECT *
      FROM   `incident`
      WHERE  `incident_id` = ".$incident_id."
      ORDER  BY date desc;
    ";
    $result = $global['db']->Execute($q);
    if($result === false) { daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "edit 1"); }
    $row = $result->FetchRow();
    // get long name, article, caption and tags for the current language 
    $translations = json_decode($row['translations'], true);
    $name = $translations[$lang]["name"];
    $article = $translations[$lang]["article"];
    $caption = $translations[$lang]["caption"];
    $tags = $translations[$lang]["tags"];
    // get Google repository info for this incident
    $q = "
      SELECT SUBSTRING_INDEX(base_url, '/', -1) AS pfKey,sched_interval_minutes
      FROM pfif_repository
      WHERE incident_id = ".$incident_id." AND name LIKE 'google%' LIMIT 1;
    ";
    $result = $global['db']->Execute($q);
    if($result === false) {  daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "edit 2"); }
    $rowpfif = $result->FetchRow();
  }
  // generate event type
  $real = $row['type'] == "REAL" ? "SELECTED" : "";
  $test = $row['type'] == "TEST" ? "SELECTED" : "";
  $type = "
    <select id=\"eventType\">
    <option value=\"REAL\" ".$real.">REAL</option>
    <option value=\"TEST\" ".$test.">TEST</option>
    </select>
  ";
  // generate default checkbox
  $checked = $row['default'] == "1" ? "CHECKED" : "";
  $default = "<input type=\"checkbox\" id=\"eventDefault\" name=\"eventDefault\" value=\"default\" ".$checked." >";
   // generate archived checkbox
  $checked = $row['archived'] == "1" ? "CHECKED" : "";
  $archived = "<input type=\"checkbox\" id=\"eventArchived\" name=\"eventArchived\" value=\"1\" ".$checked." >";
   // generate unlisted checkbox
  $checked = $row['unlisted'] == "1" ? "CHECKED" : "";
  $unlisted = "<input type=\"checkbox\" id=\"eventUnlisted\" name=\"eventUnlisted\" value=\"1\" ".$checked." >";
  // generate closed checkboxtextarea
  $checkedZero = $row['closed'] == "0" ? "CHECKED" : "";
  $checkedOne  = $row['closed'] == "1" ? "CHECKED" : "";
  if($row['closed'] != "0" && $row['closed'] != "1") { $checkedOne = "CHECKED"; }
  $closed = "
    <input type=\"radio\" id=\"eventClosed\" name=\"eventClosed\" value=\"0\" ".$checkedZero." > Reporting OPEN<br>
    <input type=\"radio\" id=\"eventClosed\" name=\"eventClosed\" value=\"1\" ".$checkedOne." > Reporting CLOSED
  ";
  // generate options for data sync with Google
  if($rowpfif != null) {
    $checkedSync = $rowpfif['sched_interval_minutes'] == "0" ? "" : "CHECKED";
    $sync = "
      PF Key: ".GOOGLE_PF."<input type=\"text\" id=\"pfKey\" name=\"pfKey\" size=\"28\" value=\"".$rowpfif['pfKey']."\">
      (e.g. 2013-boston-explosions)<br>Keep synchronized? <input type=\"checkbox\" id=\"pfSync\" name=\"pfSync\" value=\"1\" ".$checkedSync.">
      <span style=\"font-style:italic; color:red;\">Only check if Google Person Finder event activated.</span>
    ";
  } else {
    $sync = "
      PF Key: ".GOOGLE_PF."<input type=\"text\" id=\"pfKey\" name=\"pfKey\" size=\"28\" value=\"\">
      (e.g. 2013-boston-explosions)<br>Keep synchronized? <input type=\"checkbox\" id=\"pfSync\" name=\"pfSync\" value=\"1\">
      <span style=\"font-style: italic; color: red;\">Only check if Google Person Finder event activated.</span>
    ";
  }
  // generate visibility select box
  $options =  "<option value=\"-1\">PUBLIC</option>";
  if($row['private_group'] == 1) { $selected = "SELECTED"; }
  else                           { $selected = ""; }
  $options .= "<option value=\"1\" ".$selected." >Admin</option>";
  $visibility = "
    <select id=\"eventVisibility\">
    ".$options."
    </select>
  ";
  // end visibility select box
  if($row['images'] != null && $row['images'] != "") {
    $imgText = '<img src="'.$row['images'].'">';
    $delbooton = "";
  } else {
    $imgText = "";
    $delbooton = "opacity: 0.2;";
  }
  // gen edit page
  $editMainHtml = "
<div id=\"topBlock\" >
<div id=\"formBlock\" >
<table class=\"emTable2\">
  <tr>
    <td class=\"mainRowEven emFirst emLast\" style=\"\">Event ID#</td>
    <td class=\"mainRowEven emFirst emLast\" style=\"\"> &nbsp;".$row['incident_id']."<input id=\"eventId\" type=\"hidden\" value=\"".$row['incident_id']."\" readonly /></td>
  </tr>
  <tr>
    <td class=\"mainRowOdd emLastWhite\" style=\"\">Long Name</td>
    <td class=\"mainRowOdd emFirstWhite\" style=\"\"><input id=\"longName\" type=\"text\" size=30 maxlength=60 value=\"".$name."\" /> (do not use numbers)</td>
  </tr>
  <tr>
    <td class=\"mainRowEven emFirst emLast\" style=\"\">Short Name</td>
    <td class=\"mainRowEven emFirst emLast\" style=\"\"><input id=\"shortName\" type=\"text\" size=16 maxlength=16 value=\"".$row['shortname']."\" /> (lowercase letters and numbers only)</td>
  </tr>
  <tr>
    <td class=\"mainRowOdd emLastWhite\" style=\"\">Type</td>
    <td class=\"mainRowOdd emFirstWhite\" style=\"\">".$type."</td>
  </tr>
  <tr>
    <td class=\"mainRowEven emFirst emLast\" style=\"\">Event Class</td>
    <td class=\"mainRowEven emFirst emLast\" style=\"\">".$visibility."</td>
  </tr>
  <tr>
    <td class=\"mainRowOdd emLastWhite\" style=\"\">Date Time (UTC)</td>
    <td class=\"mainRowOdd emFirstWhite\" style=\"\"><input id=\"eventDate\" type=\"text\" value=\"".$row['date']."\" /></td>
  </tr>
  <tr>
    <td class=\"mainRowEven emFirst emLast\" style=\"\">Default Event</td>
    <td class=\"mainRowEven emFirst emLast\" style=\"\">".$default." (default event for ReUnite)</td>
  </tr>
  <tr>
    <td class=\"mainRowOdd emLastWhite\" style=\"\">Event Status</td>
    <td class=\"mainRowOdd emFirstWhite\" style=\"\">".$closed."</td>
  </tr>
  <tr>
    <td class=\"mainRowEven emFirst emLast\" style=\"\">Sync with<br>Google Person Finder</td>
    <td class=\"mainRowEven emFirst emLast\" style=\"\">".$sync."</td>
  </tr>
  <tr>
    <td id=\"ul1\" class=\"mainRowEven emLastWhite\" style=\"width: 250px;\">Unlisted Event</td>
    <td id=\"ul2\" class=\"mainRowEven emLastWhite\" style=\"\">".$unlisted." This event will not appear on the home page (except to admins)</td>
  </tr>
  <tr>
    <td id=\"ac1\" class=\"mainRowOdd emFirstWhite\" style=\"width: 250px;\">Archive Event</td>
    <td id=\"ac2\" class=\"mainRowOdd emFirstWhite\" style=\"\">".$archived." This event will not appear by default under the event manager</td>
  </tr>
  <tr>
    <td class=\"mainRowEven emLastWhite\" style=\"max-width: 200px;\">Card Content</td>
    <td class=\"mainRowEven emFirstWhite\"><textarea id=\"article\" class=\"five55 emta\" />".$article."</textarea></td>
  <tr>
  <tr>
    <td class=\"mainRowOdd emFirst emLast\" style=\"\">Image</td>
    <td class=\"mainRowOdd emFirst emLast\" style=\"\">
      <span class=\"emButton bgGreen spaceIt emButtonNew\" id=\"uploadChoosr\" style=\"cursor:pointer;\" onclick=\"$('#add_image').click();\">Choose an Image</span>
      <input style=\"height: 0px; width: 0px; overflow:hidden; opacity: 0; left: -5em; position: relative; z-index: -111;\" type=\"file\" name=\"add_image\" id=\"add_image\" onchange=\"em2_get_image(".$incident_id.");\" />
      <style>
        #UPLOAD:hover, #DELIMAGE:hover { text-decoration: none; }
      </style>
      <a id=\"DELIMAGE\" class=\"emButton bgRed spaceIt emButtonNew\" style=\"".$delbooton."\" onclick=\"javascript:em2_del_image();\">DELETE</a><br>
      <input id=\"images\" style=\"display: none;\" type=\"text\" size=60 maxlength=60 value=\"".$row['images']."\" readonly>
      <div id=\"uploadProgress\" style=\"color: red; font-weight: bold; margin-top: 10px; margin-bottom: 10px;\">".$imgText."</div>
    </td>
  <tr>
  <tr style=\"display: none;\">
    <td class=\"mainRowEven emLastWhite\" style=\"\">Image Caption</td>
    <td class=\"mainRowEven emFirstWhite\" style=\"\"><input id=\"caption\" type=\"text\" size=60 maxlength=256 value=\"".$caption."\" /></td>
  <tr>
    <tr>
    <td class=\"mainRowOdd emFirst emLastWhite\" style=\"\">Tags (comma separated)</td>
    <td class=\"mainRowOdd emFirst emLastWhite\" style=\"\"><input id=\"tags\" type=\"text\" size=60 maxlength=60 value=\"".$tags."\" /></td>
  <tr>
  <tr>
    <td class=\"mainRowEven\" style=\" border-bottom: none;\">Location</td>
    <td class=\"mainRowEven\" >
      <input id=\"address\" type=\"text\" size=\"60\" /> Street Address 
    </td>
  </tr>
  <tr>
    <td colspan=2 class=\"mainRowEven\" style=\"padding: 0;\">
      <div id=\"mapCanvas\" style=\"width: 100%; height: 500px;\"></div>
    </td>
  </tr>
  <tr>
    <td class=\"mainRowEven\" colspan=2 style=\"text-align: center;\">
      LAT <input id=\"latitude\" style=\"width: 15em;\" step=\"0.0000000001\" type=\"number\"/ min=\"-90\" max=\"90\" value=\"".$row['latitude']."\">
      LON <input id=\"longitude\" style=\"width: 15em;\" step=\"0.0000000001\" type=\"number\"/ min=\"-180\" max=\"180\" value=\"".$row['longitude']."\">
    </td>
  </tr>
  <tr>
    <td class=\"mainRowOdd\">Notes</td>
    <td class=\"mainRowOdd emFirst emLast emLastWhite\"><textarea class=\"emta five55 borderNone\" id=\"adminotes\">".$row['admin_notes']."</textarea></td>
  <tr>
</table>
";
  // control HTML
  if($new) { $newVar = "1"; }
  else     { $newVar = "0"; }
  $editControlHtml = '
  <select id="lang" style="margin-right:80px;" onchange="em2_change_lang('.$incident_id.', '.$newVar.');">
    <option value="en" '.(($lang=="en")?'SELECTED':"").' >English</option>';
  if((int)in_array('es',    $conf['i18n_on']) == 1) { $editControlHtml .= '<option value="es" '    .(($lang=="es")?'SELECTED':"").' >Spanish</option>'; }
  if((int)in_array('el',    $conf['i18n_on']) == 1) { $editControlHtml .= '<option value="el" '    .(($lang=="el")?'SELECTED':"").' >Greek</option>'; }
  if((int)in_array('fr',    $conf['i18n_on']) == 1) { $editControlHtml .= '<option value="fr" '    .(($lang=="fr")?'SELECTED':"").' >French</option>'; }
  if((int)in_array('hi',    $conf['i18n_on']) == 1) { $editControlHtml .= '<option value="hi" '    .(($lang=="hi")?'SELECTED':"").' >Hindi</option>'; }
  if((int)in_array('ja',    $conf['i18n_on']) == 1) { $editControlHtml .= '<option value="ja" '    .(($lang=="ja")?'SELECTED':"").' >Japanese</option>'; }
  if((int)in_array('mm',    $conf['i18n_on']) == 1) { $editControlHtml .= '<option value="mm" '    .(($lang=="mm")?'SELECTED':"").' >Burmese / Myanmar</option>'; }
  if((int)in_array('ru',    $conf['i18n_on']) == 1) { $editControlHtml .= '<option value="ru" '    .(($lang=="ru")?'SELECTED':"").' >Russian</option>'; }
  if((int)in_array('ta',    $conf['i18n_on']) == 1) { $editControlHtml .= '<option value="ta" '    .(($lang=="ta")?'SELECTED':"").' >Tamil</option>'; }
  if((int)in_array('ur',    $conf['i18n_on']) == 1) { $editControlHtml .= '<option value="ur" '    .(($lang=="ur")?'SELECTED':"").' >Urdu</option>'; }
  if((int)in_array('vi',    $conf['i18n_on']) == 1) { $editControlHtml .= '<option value="vi" '    .(($lang=="vi")?'SELECTED':"").' >Vietnamese</option>'; }
  if((int)in_array('zh_CN', $conf['i18n_on']) == 1) { $editControlHtml .= '<option value="zh_CN"  '.(($lang=="zh_CN")?'SELECTED':"").' >Chinese Simplified</option>'; }
  if((int)in_array('zh_TW', $conf['i18n_on']) == 1) { $editControlHtml .= '<option value="zh_TW"  '.(($lang=="zh_TW")?'SELECTED':"").' >Chinese Traditional</option>'; }
  $editControlHtml .= '</select>
  <input type="hidden" id="lang_prev" value="'.$lang.'" />
  <paper-button class="buttonEM bgGreen emButtonNew"  onclick="toast3(\'Saving Changes ... (this may take a long time if event being retired)\'); em2_perform_save('.$incident_id.', em2_get_data());">Save Changes</paper-button>
  <paper-button class="buttonEM bgYellow emButtonNew" onclick="saveCheck('.$newVar.');">Cancel Editing and Close</paper-button>';
  // show delete/purge buttons
  $deleteButtonTitle = 'Delete Event';
  $editControlHtml .= '&nbsp; &nbsp; &nbsp; &nbsp;<paper-button class="buttonEM bgRed emButtonNew" onclick="em2_ask_delete('.$incident_id.');">'.$deleteButtonTitle.'</paper-button>';
  $editControlHtml .= '&nbsp; &nbsp;<paper-button class="buttonEM bgRed emButtonNew" onclick="em2_ask_purge('.$incident_id.');">Purge Event</paper-button>';
  $global['xajax_res']->addAssign('rezMain', 'innerHTML', $editMainHtml);
  $global['xajax_res']->addAssign('rezControl', 'innerHTML', $editControlHtml);
  if($new) { $global['xajax_res']->addScript("toast3('Creating new event')"); }
  $streetClean = str_replace("'", "", $row['street']);
  $global['xajax_res']->addScript("load_map(".$row['latitude'].", ".$row['longitude'].", '".$streetClean."');");
  return $global['xajax_res']->getXML();
}

// helper function
function em2_image_send($incident_id, $data) {
  global $global, $conf;
  $image_id = $incident_id.'_'.date("mdGis");
  $filename = "eventArticleImage_".$image_id."_";
  $path = $conf['approot']."www/tmp/em_cache/".$filename;
  // save original like /opt/pl/www/tmp/em_cache/eventArticleImage_112233_original
  file_put_contents($path."original", base64_decode($data));
  // get information from original file
  $error = false;
  $info = getimagesize($path."original");
  if($info !== false) {
    $image_width  = $info[0];
    $image_height = $info[1];
    list(,$mime) = explode("/",$info['mime']);
    $mime = strtolower($mime);
    $image_type = $mime;
    if(stripos($mime,"png") !== FALSE) { $ext = ".png"; }
    elseif(stripos($mime,"gif") !== FALSE) { $ext = ".gif"; }
    elseif(stripos($mime,"jpeg") !== FALSE) { $ext = ".jpg"; }
    else { $error = true; }
  } else { $error = true;}
  $fullSizePath  = $path."full".$ext;
  $thumbnailPath = $path."thumb.jpg";
  if(!$error) {
    if($image_width == 400) {
      // rename original file like /opt/pl/www/tmp/em_cache/eventArticleImage_112233_full.ext
      rename($path."original", $fullSizePath);
      // update URLs
      $url       = "tmp/em_cache/".$filename."full".$ext;
      $url_thumb = "tmp/em_cache/".$filename."full".$ext;
      // make the files world writeable for other users/applications and to handle deletes
      chmod($fullSizePath,  0777);
      $passUrl = $url_thumb;
    } else {
      // rename original file like /opt/pl/www/tmp/em_cache/eventArticleImage_112233_full.ext
      rename($path."original", $fullSizePath);
      // save thumb resampled image / 600px width
      $thumb_height = ((566/$image_width)*$image_height);
      taupo_image_resize($fullSizePath, $path."thumb.jpg", 566, $thumb_height, true);
      // update URLs
      $url       = "tmp/em_cache/".$filename."full".$ext;
      $url_thumb = "tmp/em_cache/".$filename."thumb.jpg";
      // make the files world writeable for other users/applications and to handle deletes
      chmod($fullSizePath,  0777);
      chmod($thumbnailPath, 0777);
      $passUrl = $url_thumb;
    }
  } else { $passUrl = "INVALID_IMAGE"; }
  $global['xajax_res']->addScript("updateImage('".$passUrl."');");
  return $global['xajax_res']->getXML();  
}

// reset test exercise to default event
function setTestDefault() {
  global $global;
  $q = "UPDATE incident SET `default` = 1 WHERE incident_id = 1;";
  $result = $global['db']->Execute($q);
  if($result === false) { daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "save event 9988"); }
}

// Rename all Google Person Finder image files for a specified event (to prevent direct access to them using known name).
// IMPORTANT: Replicate any changes to this file in analogous file in tools directory !!
function renamePFIFImages($eventId) {
  global $global, $conf;
  // get all Google Person Finder images for this event
  $sql = "SELECT i.image_id, i.url, i.url_thumb FROM image i, person_uuid pu" .
  " where pu.incident_id=$eventId" .
  " AND i.url LIKE '%pfif_cache%'" .
  " and pu.p_uuid = i.p_uuid";
  $result = $global['db']->Execute($sql);
  if($result === false) { daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "renamePFIFImages"); }
  $pathprefix = $conf['approot'].'www/';
  $random_len = 6;
  while ($row = $result->FetchRow()) {
    $image_id = $row['image_id'];
    $full = $row['url'];
    $thumb = $row['url_thumb'];
    $samefilename = ($full == $thumb);
    // check if already renamed
    $pattern = '/_([^_]{'.$random_len.'})\./';
    if(preg_match($pattern, $full, $match)) {
      // already renamed. Replace previous random string
      $fullfile = $pathprefix.$full;
      $thumbfile = $pathprefix.$thumb;
      $fullnew = str_replace($match[1], random_str($random_len), $full);
      if($samefilename) { $thumbnew = $fullnew; } // full and thumb filenames the same so just copy the new name
      else { $thumbnew = str_replace($match[1], random_str($random_len), $thumb); }
      $fullfilenew = $pathprefix.$fullnew;
      $thumbfilenew = $pathprefix.$thumbnew;
    } else {
      // insert new random string
      $fullfile = $pathprefix.$full;
      $thumbfile = $pathprefix.$thumb;
      // append random string after p_uuid/image_id
      $pattern = '/(.*)(\..+)$/';
      $replace = '${1}_'.random_str($random_len).'${2}';
      $fullnew = preg_replace($pattern, $replace, $full); 
      if($samefilename) { $thumbnew = $fullnew;  } // full and thumb filenames the same so just copy the new name
      else { $thumbnew = preg_replace($pattern, $replace, $thumb);  }
      $fullfilenew = $pathprefix.$fullnew;
      $thumbfilenew = $pathprefix.$thumbnew;
    }
    // rename the full file
    if(rename($fullfile, $fullfilenew)) {
      $q = "
        UPDATE  image
        SET url = '".$fullnew."'
        WHERE image_id = '".$image_id."';
      ";
      $status = $global['db']->Execute($q);
      if($status === false) { daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "renamePFIFImages 2");  }
    } else { daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, "Rename of $fullfile to $fullfilenew failed", "renamePFIFImages 3"); }
    // Rename the thumb file if different file name. Otherwise, just update the DB.
    if(!$samefilename) { $status = rename($thumbfile, $thumbfilenew); }
    else { $status = 1; }
    if($status) {
      // Update DB.
      $q = "
        UPDATE  image
        SET url_thumb = '".$thumbnew."'
        WHERE image_id = '".$image_id."';
      ";
      $status = $global['db']->Execute($q);
      if($status === false) { daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "renamePFIFImages 4"); }
    } else { daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, "Rename of $thumbfile to $thumbfilenew failed", "renamePFIFImages 5"); }
  }
}
