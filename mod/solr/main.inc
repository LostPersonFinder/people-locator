<?
/**
 * @name     SOLR Monitor
 * @author   pl@miernicki.com
 * @about    Developed by the U.S. National Library of Medicine.
 * @link     https://gitlab.com/tehk/people-locator
 * @license  https://gitlab.com/tehk/people-locator/blob/master/LICENSE
 */

$global['use_stream'] = true; // use lib_stream

function taupo_solr_default() {
  global $global;
  display_solr_status();
}
function display_solr_status() {
  global $conf;
  // Cover multiple instances of SOLR.
  $hosts = $conf["SOLR_hosts"];
  echo "<div class=\" four44 oneEM\"><h2><center>SOLR Status</center></h2>";
  foreach ($hosts as $host) {
    // For each host, first check whether SOLR is up.
    $solrQuery = str_replace("localhost", $host, $conf["SOLR_root"])."admin/ping";
    $ch = curl_init();
    curl_setopt($ch, CURLOPT_URL, $solrQuery);
    curl_setopt($ch, CURLOPT_PORT, $conf['SOLR_port']);
    curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
    $data = curl_exec($ch);
    curl_close($ch);
    $msg = '';
    if (!$data) {
      $msg .= '<span style="color: red;">SOLR is DOWN</span>';
      continue;
    } else {
      $msg .= '<span style="color: green;">SOLR is UP</span>';
    }
    // Then report any errors.
    $solrQuery = str_replace("localhost", $host, $conf["SOLR_root"])."admin/logging?wt=json&since=0";
    $ch = curl_init();
    curl_setopt($ch, CURLOPT_URL, $solrQuery);
    curl_setopt($ch, CURLOPT_PORT, $conf['SOLR_port']);
    curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
    $data = curl_exec($ch);
    curl_close($ch);
    if ($data) {
      $temp = json_decode($data);
      $num_found = $temp->history->numFound;
      $errorLog = '';
    for ($i = 0; $i < $num_found; ++$i) {
      if ($temp->history->docs[$i]->level == "ERROR") {
        // Show time, level, and the first line of the error.
        $local_time = date('Y-m-d H:i:s A', strtotime($temp->history->docs[$i]->time));
        $level = $temp->history->docs[$i]->level;
        $errorLog .= "$local_time " . "$level " . strtok($temp->history->docs[$i]->message,"\n") . "<br>";
      }
    }
      echo "<h4><center>$host: $msg</center></h4><h5>Log errors:</h5><div><pre>".htmlspecialchars($errorLog)."</pre></div>";
    } else {
      // SOLR down, so can't report on errors.
       echo "<h4><center>$host: $msg</center></h4>";
    }
  }
  echo "</div>";
}
