<?
/**
 * @name     Batch Reporter (br) Controller
 * @author   pl@miernicki.com
 * @about    Developed in whole by the U.S.National Library of Medicine
 * @link     https://gitlab.com/tehk/people-locator
 * @license  https://gitlab.com/tehk/people-locator/blob/master/LICENSE
 */

error_reporting(0); // Turn off all error reporting

// Report formats.
$formats = array("sde" => array(  // San Diego EMS
  "header_len" => 3,  // # header lines 
  "name_format" => 1, // given name followed by surname 
  "name_pos" => 0, 
  "address_pos" => 1,
  "last_location_pos" => 2,
  "age_format" => 1, // adult|child
  "age_pos" => 3,
  "gender_format" => 1, // male|female
  "gender_pos" => 4,
  "race_pos" => 5,
  "hair_color_pos" => 6,
  "marks_pos" => 7,
  "requester_pos" => 8
));

if(!isset($_POST['service_key']) || $_POST['service_key'] !== $conf['service_key']) {
  http_response_code(404);
  echo '404!';
  fail2ban();
  die();
} else { br_accept(); }

function br_accept() {
  global $global, $conf, $formats;
  ob_end_clean();
  $report_format = $formats[$_POST['report_format']];
  $shortname = $_POST['shortname'];
  $status = $_POST['status'];
  $dir = $conf['approot'].'www/tmp/br_cache/';
  $dir2 = generateRandomString();
  $dir3 = $dir.$dir2."/";
  mkdir($dir3, 0777);
  // Read in CSV file.
  $all = readCSV($_FILES['userfile']['tmp_name']);
  // Remove header lines.
  $lines = array_slice($all, $report_format['header_len']);
  $count = count($lines);
  ob_start();
  echo "<b>$count total records to process...</b><br><i>Processing: </i>";
  ob_flush();
  ob_end_flush();
  flush();
  $image_count = 0;
  $count = 0;
  foreach ($lines as $line) {
    $fullname = $line[$report_format['name_pos']];
    $address = $line[$report_format['address_pos']];
    $last_location = $line[$report_format['last_location_pos']];
    $age = $line[$report_format['age_pos']];
    $gender = $line[$report_format['gender_pos']];
    $race = $line[$report_format['race_pos']];
    $hair_color = $line[$report_format['hair_color_pos']];
    $marks = $line[$report_format['marks_pos']];
    $requester = $line[$report_format['requester_pos']];
    // Skip records with empty first field.
    if (empty($fullname)) continue;
    ob_start();
    //echo "processing person: ".$line[0]."<br>";
    // new person
    $p = new person();
    $p->createUUID();
    $p->reporting_user = 1;
    if ($report_format['name_format'] == 1) {
      $p->full_name   = $fullname;
      // split full name into parts
      $e = explode(" ", $fullname);
      $p->given_name   = isset($e[0]) ? $e[0] : '';
      $p->family_name  = isset($e[1]) ? $e[1] : '';
    }
    $p->setEvent($shortname);
    $p->opt_status = $status;
    $p->home_zip = extract_zipcode($address);
    $p->home_address = str_replace($p->home_zip, "", $address);
    $p->last_seen = $last_location;
    if ($report_format['age_format'] == 1) {
      if (strcasecmp($age, 'adult') == 0) {
        $p->maxAge = MAXIMUM_AGE;
        $p->minAge = 18;
      } elseif (strcasecmp($age, 'child') == 0) {
        $p->maxAge = 17;
        $p->minAge = MINIMUM_AGE;
      } 
    }
    if ($report_format['gender_format'] == 1) {
      if (strcasecmp($gender, 'male') == 0) { $p->opt_gender = 'mal'; }
      elseif (strcasecmp($gender, 'female') == 0) { $p->opt_gender = 'fml'; }
    }
    $p->other_comments .= "Ethnicity: $race\n";
    $p->other_comments .= "Hair color: $hair_color\n";
    $p->other_comments .= "Definitive marks: $marks\n";
    $p->other_comments .= "Requester (name, relation, phone): $requester";
    $p->creation_time = date('Y-m-d H:i:s', (time()-1));
    $p->last_updated  = date('Y-m-d H:i:s', (time()+1));
    $p->useNullLastUpdatedDb = false;
    $p->makePfifNote = false;
    $p->arrival_website = true; // show they were reported via the website
    //  Update person with this image.
    //if (!add_image($p, $dir3.$file, $image, 1, $num_names)) {
      //$invalid = true;
      //echo '<br><span style="color: red;">invalid image file: <b>'.$file.'</b></span><br>';
    //}
    $p->insert();
    $count++;
    //$image_count += count($p->images);
    echo "#$count ";
      // purge objects
      $i = null;
      $p = null;
      ob_flush();
      ob_end_flush();
      flush();
    }
  ob_start();
  echo "<br><b>Finished adding $count total persons to event $shortname.";
  taupo_acl_log('BATCH_REPORT', 'user '.$_SESSION['user'].' performed a batch report on event: '.$shortname);
  ob_flush();
  ob_end_flush();
  flush();
  // Different persons may share the same image so delete everything at end.
  exec("rm -rf {$dir3}");
}

function add_image($p, $file, $image, $principal, $num_names) {
  global $global, $conf;
  // Create new image.
  $i = new image();
  $i->init();
  $i->p_uuid = $p->p_uuid;
  $i->fileContent = file_get_contents($file);
  $i->principal = $principal;
  $regions = $image[1];
  $age = $image[2];
  $gender = $image[3];
  $i->face_region = $regions;
  // Only  set person fields for principal image metadata. Ignore metadata for other images.
  if ($principal) {
    if ($age == 'adult') {
      $p->maxAge = MAXIMUM_AGE;
      $p->minAge = 18;
    } elseif ($age == 'youth') {
      $p->maxAge = 17;
      $p->minAge = MINIMUM_AGE;
    } elseif ($age != '-1') { $p->years_old = (int)$age; }
    if ($gender == 'male')       { $p->opt_gender = 'mal'; }
    elseif ($gender == 'female') { $p->opt_gender = 'fml'; }
    if ($num_names == 0) {
      // Extract names from file name.
      $parts = explode(".", $file);
      list($p->given_name, $p->family_name) = explode("_", $parts[0]."_", 2); // appended "_" makes list() happy when not present
    } else {
      $index = rand(1,$num_names);
      $tblname = ($gender == 'female')? 'biu_fnames':'biu_mnames';
      $query = "select given_name,family_name FROM $tblname where id=$index";
      $result = $global['db']->Execute($query);
      if($result === false) {
        $errchk = $global['db']->ErrorMsg();
        die("Error getting names from $tblname: " . $errchk);
      }
      $row = $result->FetchRow();
      $p->given_name = $row['given_name'];
      $p->family_name = $row['family_name'];
    }
  }
  // Add it to person.
  $p->images[] = $i;
  return !$i->invalid;
}

function generateRandomString($length = 8) {
  $characters = 'abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ';
  $randomString = '';
  for($i = 0; $i < $length; $i++) { $randomString .= $characters[rand(0, strlen($characters) - 1)]; }
  return $randomString;
}

function readCSV($csvFile){
  $file_handle = fopen($csvFile, 'r');
  while (!feof($file_handle) ) { $lines_of_text[] = fgetcsv($file_handle); }
  fclose($file_handle);
  return $lines_of_text;
}

function extract_zipcode($address) {
  $zipcode = preg_match("/\b\d{5}(-\d{4})?\b/", $address, $matches);
  return $matches[0];
}

die();
