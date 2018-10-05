<?
$incident_id = $argv[1];
error_reporting(E_ALL ^ E_NOTICE);
ini_set("display_errors", "stdout");
require_once("../conf/taupo.conf");
require_once("../inc/lib_includes.inc");
echo "\n\nBeginning event purge at " . strftime("%c") . "\n";
echo "Using db ". $global['db']->database."\n";
$count = 0;
$q1 = "
  SELECT *,
  (SELECT count(*) FROM person_uuid p WHERE 
    p.incident_id = i.incident_id) AS total 
  FROM incident i
  WHERE incident_id = ".(int)$incident_id."
  LIMIT 1;
";
$result1 = $global['db']->Execute($q1);
if($result1 === false) { daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "event_delete_1 ((".$q1."))"); }
$do = 0;
while($row1 = $result1->FetchRow() ) {
  $do++;
  $total = (int)$row1['total'];
  echo "There are ".$total." total records in this event.\n";
  $webroot = $conf['approot']."www/";
  $delPersons = 0;
  // Reset last_count for last export log  record. (Otherwise, the first 'last_count' new records won't get exported.)
  $q2 = "
    UPDATE pfif_export_log pe, pfif_repository pr SET pe.last_count = 0
    WHERE pr.incident_id = ".(int)$incident_id."
    AND pe.repository_id = pr.id
    AND pe.status = 'paused';
  ";
  $st = $global['db']->Execute($q2);
  if($st === false) {
    daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "purge 0 ((".$q2."))");
    $errchk = $global['db']->ErrorMsg();
    echo "Error updating last_count for export log for this incident: (".$errchk.")\n";
  }
  echo "Loading person data.\n";
  // Get all persons for this incident
  $q3 = "
    SELECT *
    FROM person_uuid
    WHERE incident_id = ".(int)$incident_id.";
  ";
  $result3 = $global['db']->Execute($q3);
  if($result3 === false) { daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "purge 1");  echo "load error! dying!\n"; die(); }
  $people = array();
  $count = 0;
  while($row3 = $result3->FetchRow()) {
    $people[$count] = $row3['p_uuid'];
    $count++;
  }
  echo "Counted ".$count." persons to delete.\n";
  echo 'Deleting persons ';
  foreach($people as $p_uuid) {
    $p = new person();
    $p->p_uuid = $p_uuid;
    $p->load();
    $p->solrDelete = false; // we do a full-import afterwards, so this is not necessary
    $p->delete();
    $delPersons++;
    if($delPersons % 1000 == 0) { echo $delPersons."/".$count; }
    elseif($delPersons % 100 == 0) { echo $delPersons; }
    elseif($delPersons % 10 == 0) { echo "o"; }
    else { echo "."; }
  }
  echo "\nFinished deleting a total of ".$delPersons." persons.\n";  
  // Have SOLR do a full update of indexes.
  if($conf['SOLR_on'] == true) {
    echo "Performing a full index on SOLR.\n";
    $solr_url = str_replace("/solr/", ":".$conf["SOLR_port"]."/solr/", $conf["SOLR_root"]);
    $handle = fopen($solr_url."dataimport?command=full-import", "r");
    if(!$handle) { echo "Error doing full reload of indexes after purge.\n"; }
  }
  echo "Deleting incident: ".$incident_id." from the database.\n";
  // delete the incident from the db
  $q4 = "DELETE from incident WHERE incident_id = '".(int)$incident_id."';  ";
  $result4 = $global['db']->Execute($q4);
  if($result4 === false) { daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "delete 1"); }
}
if($do == 0) { echo "Invalid incident_id: ".$incident_id."\n"; }
echo "Finished event purge at " . strftime("%c") . "\n\n\n";
