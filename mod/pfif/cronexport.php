<?
/**
 * @name         Person Finder Interchange Format
 * @author       pl@miernicki.com
 * @about        Developed by the U.S. National Library of Medicine
 * @link         https://gitlab.com/tehk/people-locator
 * @license	     https://gitlab.com/tehk/people-locator/blob/master/LICENSE
 */


// cron job task for for pfif export
require_once("../../conf/taupo.conf");
require_once("../../3rd/adodb/adodb.inc.php");
require_once("../../inc/lib_database.inc");
require_once("pfif.inc");
require_once("repository.inc");
require_once("croninit.inc");

// Put after includes.
// print "Configuring error reporting  ...\n";
error_reporting(E_ALL & ~E_DEPRECATED & ~E_STRICT & ~E_NOTICE);
// print "Configuring error display  ...\n";
ini_set("display_errors", "stdout");

/**
 *  Log harvest end
 */
function update_harvest_log($r, $req_params, $status) {
   if (array_key_exists('pfif_info', $_SESSION)) $pfif_info = $_SESSION['pfif_info'];
   $pfif_info['end_time'] = time();
   //var_dump("ending harvest with pfif_info:", $pfif_info);
   $r->end_harvest($status, $req_params, $pfif_info);
}

session_start();
print "\nDatabase = " . $conf['db_name'];

// Get all PFIF repository sources.
$repositories = Pfif_Repository::find_sink();
if (!$repositories) {
   die("No repositories ready for harvest.\n");
}
//var_dump("Found repositories for export", $repositories);

$sched_time = time();
$export_repos = array();
foreach ($repositories as $r) {
   if ($r->is_ready_for_harvest($sched_time)) {
      add_pfif_service($r);             // initializes pfif_conf
      //var_dump("exporting to repository",$r);
   }
}
unset($r);
unset($repositories);
$export_queue = $pfif_conf['services'];

foreach ($export_queue as $service_name => $service) {
   $repos = $service['repository'];
   $req_params = $repos->get_request_params();
   $min_entry_date = $req_params['min_entry_date'];
   $skip = $req_params['skip'];
   $subdomain = empty($service['subdomain'])? '' : '?subdomain='.$service['subdomain'];
   $auth_key = empty($service['auth_key'])? '' : '?key='.$service['auth_key'];
   $pfif_uri = $service['post_url'].$auth_key;
   $at_subdomain = " at subdomain $subdomain ";
   $p = new Pfif();
   $p->setService($service_name,$service);

   $repos->start_harvest('scheduled', 'out');
   print "\n\nExport started to ".$service['post_url']." at ".date("Y-m-d H:i:s")."\n";
   $local_date = local_date($min_entry_date);
   $loaded = $p->loadFromDatabase($local_date, null, $skip);
   print "Exporting original records after $local_date.\n";

   if ($loaded > 0) {
      // Export records
      $xml = $p->storeInXML(false, true);
      if ($xml != null) {
         //$fh = fopen('cronpfif.xml', 'w');
         //$charstowrite = strlen($xml);
         //$written = fwrite($fh, $xml, $charstowrite);
         //fclose($fh);
      	// Save raw output for auditing purposes.
			@file_put_contents("../../www/tmp/pfif_logs/raw_exports.xml", $xml, FILE_APPEND);
         $post_status = $p->postToService($xml);
         // Person and note counts are in $_SESSION['pfif_info'].
         if ($post_status == -1) {
            update_harvest_log($repos, $req_params, 'error');
            pfif_error_log("Export failed.\n");
         } else {
            update_harvest_log($repos, $req_params, 'completed');
            // Parse XML response.
            $post_status_no_ns = str_replace("status:", "", $post_status);
            $response = @new SimpleXMLElement($post_status_no_ns);
            $written = $response->write[0]->written;
            $parsed = $response->write[0]->parsed;
            $sent = $_SESSION['pfif_info']['pfif_person_count'];
            print "Export status: Sent=$sent, Parsed=$parsed, Accepted=$written\n";
            if ($sent != $written) {
               pfif_error_log("Status: $post_status_no_ns\n");
            }
         }
      } else {
         update_harvest_log($repos, $req_params, 'completed');
         print "Export complete: no records to upload\n";
      }
   } else {
      if ($loaded == -1) {
         update_harvest_log($repos, $req_params, 'error');
         pfif_error_log("Export failed: no records to upload\n");
      } else {
         update_harvest_log($repos, $req_params, 'completed');
         print "Export completed: no records to upload\n";
      }
   }
   if (array_key_exists('pfif_info', $_SESSION)) unset($_SESSION['pfif_info']);
}
