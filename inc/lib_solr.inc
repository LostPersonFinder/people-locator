<?
/**
 * @name     SOLR Library
 * @author   pl@miernicki.com
 * @about    Developed by the U.S. National Library of Medicine
 * @link     https://gitlab.com/tehk/people-locator
 * @license  https://gitlab.com/tehk/people-locator/blob/master/LICENSE
 */

// Check whether SOLR is up.
function taupo_solr_status() {
  global $conf;
  // Cover multiple instances of SOLR.
  $hosts = $conf["SOLR_hosts"];
  $errorFound = false;
  foreach ($hosts as $host) {
    // For each host, first check whether SOLR is up.
    $solrQuery = str_replace("localhost", $host, $conf["SOLR_root"])."admin/ping";
    $ch = curl_init();
    curl_setopt($ch, CURLOPT_URL, $solrQuery);
    curl_setopt($ch, CURLOPT_PORT, $conf['SOLR_port']);
    curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
    $data = curl_exec($ch);
    curl_close($ch);
    if (!$data) {
      $errorFound = true;
      continue;
    }
  }
  return !$errorFound;
}

// Check whether SOLR logs are clean during last 24 hours.
function taupo_solr_log_status() {
  global $conf;
  // Cover all instances of SOLR.
  $hosts = $conf["SOLR_hosts"];
  // 24 hours ago converted to milliseconds since epoch.
  $since = strtotime("-1 day") * 1000;
  $errorFound = false;
  foreach ($hosts as $host) {
    $solrQuery = str_replace("localhost", $host, $conf["SOLR_root"])."admin/logging?wt=json&since=$since";
    $ch = curl_init();
    curl_setopt($ch, CURLOPT_URL, $solrQuery);
    curl_setopt($ch, CURLOPT_PORT, $conf['SOLR_port']);
    curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
    $data = curl_exec($ch);
    curl_close($ch);
    if($data) {
      $temp = json_decode($data);
      $num_found = $temp->history->numFound;
      for($i = 0; $i < $num_found; ++$i) {
        if ($temp->history->docs[$i]->level == "ERROR") {
          $errorFound = true;
        }
      }
    }
  }
  return !$errorFound;
}

/**
 * Have SOLR do a full update of indexes for all load-balanced hosts.
 * NOTE: NOT used currently.
 */
function taupo_solr_full_import() {
  global $conf;
  if($conf['SOLR_on'] == true) {
    $hosts = $conf["SOLR_hosts"]; 
    foreach($hosts as $host) {
      $solr_url = str_replace("localhost", $host.":".$conf["SOLR_port"], $conf["SOLR_root"]);
      $handle = fopen($solr_url."dataimport?command=full-import&optimize=true", "r");
      if(!$handle) {
        error_log("taupo_solr_full_import() : ERROR!");
      }
    }
  }
}

/**
 * Have SOLR add/update its indexes with information from the database for a single person.
 *
 * @param string $p_uuid
 */
function taupo_solr_add_person($p_uuid) {
  global $conf;
  $retries = 10;
  if($conf['SOLR_on'] == true) {
    $hosts = $conf["SOLR_hosts"]; 
    foreach($hosts as $host) {
      $solrQuery = str_replace("localhost", $host.":".$conf["SOLR_port"], $conf["SOLR_root"])."dataimport?command=delta-import&p_uuid=$p_uuid&wt=json";
      $ch = curl_init();
      curl_setopt($ch, CURLOPT_URL, $solrQuery);
      curl_setopt($ch, CURLOPT_PORT, $conf['SOLR_port']);
      curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
      // Since SOLR dataimporter doesn't requeue "busy" requests, do that here.
      $busy = true;
      for ($i=0; $i<$retries; ++$i) {
        $data = curl_exec($ch);
        if ($data && json_decode($data)->status == "idle") {
          $busy = false;
          break;
        } else {
          // Requeue after .5 seconds.
          usleep(500000);
        }
      }
      curl_close($ch);
      // Log "busy" status. 
      if ($data && $busy) {
        $message = "taupo_solr_add_person: Unable to add $p_uuid to SOLR\n";
        // Use FM error log for now so I can monitor.
        $fm_error_log = "/pl/facematch/logs/".$conf['db_name']."_fm_error.log";
        error_log($message, 3, $fm_error_log);
      }
    }
  }
}

/**
 * Have SOLR delete a single person from its indexes.
 *
 * @param string $p_uuid
 */
function taupo_solr_delete_person($p_uuid) {
  global $conf;
  if ($conf['SOLR_on'] == true) {
    $hosts = $conf["SOLR_hosts"]; 
    foreach($hosts as $host) {
      $solrQuery = str_replace("localhost", $host, $conf["SOLR_root"])."update?stream.body=<delete><id>".$p_uuid."</id></delete>&commit=true";
      $ch = curl_init();
      curl_setopt($ch, CURLOPT_URL, $solrQuery);
      curl_setopt($ch, CURLOPT_RETURNTRANSFER, 1);
      curl_setopt($ch, CURLOPT_VERBOSE, 1);
      curl_setopt($ch, CURLOPT_PORT, $conf['SOLR_port']);
      $data = curl_exec($ch);
      curl_close($ch);
    }
  }
}

/**
 * Have SOLR update its indexes from the database for all persons in an event.
 *
 * @param string $event_id
 */
function taupo_solr_add_persons($event_id) {
  global $global;
  // For each person in the event, re-add them to SOLR.
  $sql = "
      SELECT p_uuid FROM person_uuid
      WHERE incident_id = $event_id
    ";
  $res = $global['db']->Execute($sql);
  while ($row = $res->FetchRow()) {
    taupo_solr_add_person($row['p_uuid']);
  }
}
