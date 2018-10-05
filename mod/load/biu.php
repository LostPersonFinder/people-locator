<?
/**
 * @name     BIU Controller
 * @author   pl@miernicki.com
 * @about    Developed in whole by the U.S.National Library of Medicine
 * @link     https://gitlab.com/tehk/people-locator
 * @license  https://gitlab.com/tehk/people-locator/blob/master/LICENSE
 */

error_reporting(0); // Turn off all error reporting
$prevnames = [];

if(!isset($_POST['service_key']) || $_POST['service_key'] !== $conf['service_key']) {
  http_response_code(404);
  echo '404!';
  fail2ban();
  die();
} else { biu_accept(); }

function biu_accept() {
  ob_end_clean();
  global $global, $conf;
  $error = false;
  $dir = $conf['approot'].'www/tmp/biu_cache/';
  $dir2 = generateRandomString();
  $dir3 = $dir.$dir2."/";
  mkdir($dir3, 0777);
  // Is it a zip file of images or ground truth info (FaceMatch).
  $is_zip = (substr($_FILES['userfile']['type'], 0, 12) == 'application/');
  $images = [];
  $ids = [];
  if($is_zip) {
    ob_start();
    $zip = $dir3."biu_".date("Ymd_Gis").".zip";
    move_uploaded_file($_FILES["userfile"]["tmp_name"], $zip);
    chdir($dir3);
    $z = new ZipArchive;
    if($z->open($zip) === TRUE) {
      $z->extractTo($dir3);
      $z->close();
      echo '<b>Zip file is okay.</b><br>';
    } else {
      echo '<b>Invalid zip file! TERMINATED.</b><br>';
      $error = true;
    }
    unlink($zip);
    ob_flush();
    ob_end_flush();
    flush();
    // walk the directory for images
    $files = array_diff(scandir($dir3), array('.', '..'));
    foreach ($files as $file) { $images[] = [$file, '', '-1', 'unknown', '']; }
  } else {
    // Examples of ground truth attributes (could be tab-separated multiples of either)
    // f{[x,y;w,h] i[x,y;w,h] n[x,y;w,h] m[x,y;w,h] e[x,y;w,h] g[male|female] a[youth|adult] d[ID|Name]}
    // f[x,y;w,h]
    // Read in ground truth file.
    $gt = file_get_contents($_FILES['userfile']['tmp_name']);
    // Remove any comment lines.
    $gt = preg_replace("/#.*\n/", "", $gt); 
    // Break out file paths and attributes.
    preg_match_all("/([^\t]+)\t(.+)\n/", $gt, $matches);
    $paths = $matches[1];
    $attrs = $matches[2];
    // Copy files to cache and save attributes.
    foreach ($paths as $index=>$path) {
      // rename as dirname_filename to preserve uniqueness
      $parts = explode('/', $path);
      $n = count($parts);
      $filename = $parts[$n-2].'_'.$parts[$n-1];
      if(!copy($conf['facematch_data'].$path, $dir3.$filename)) { die("Unable to copy ".$conf['facematch_data'].$path." to ".$dir3.$filename); }
      chmod($dir3.$filename, "0777");
      // Parse attributes.
      preg_match_all("/([fp][{[][^}]+[]}])+/", $attrs[$index], $matches);
      foreach ($matches[1] as $match) {
        // Remove identity region, but send remainder on to FM.
        $regions = preg_replace("/\td\[[^\]]+\]/", "", $match);
        if(preg_match("/a\[([^\]]+)\]/", $match, $result))  { $age = $result[1]; }
        else                                                { $age = '-1'; }
        if(preg_match("/g\[([^\]]+)\]/", $match, $result))  { $gender = $result[1]; }
        else                                                { $gender = 'unknown'; }
        if (preg_match("/d\[([^\]]+)\]/", $match, $result)) { $id = $result[1]; }
        else                                                { $id = ''; }
        $images[] = [$filename, $regions, $age, $gender, $id];
        if(!in_array($id, $ids)) { if (!empty($id)) { $ids[] = $id; }}
      }
    }
  }
  if (!$error) {
    // gen count for output
    $count = count($images);
    ob_start();
    echo "<b>$count total images to process...</b><br><i>Processing: </i>";
    ob_flush();
    ob_end_flush();
    flush();
    // Get count of DB of names for GT uploads.
    $num_names = 0;
    if(!$is_zip) {
      $query = "SELECT count(*) AS count from biu_fnames";
      $result = $global['db']->Execute($query);
      if($result === false) {
        $errchk = $global['db']->ErrorMsg();
        die("Error getting count from biu_fnames: " . $errchk);
      }
      $row = $result->FetchRow();
      $num_names = $row['count'];
    }
    $image_count = 0;
    $count = 0;
    foreach ($images as $image) {
      ob_start();
      $file = $image[0];
      //echo "processing file: $file<br>";
      // new person
      $p = new person();
      $p->createUUID();
      $p->reporting_user = 1;
      if (isset($_POST['shortname'])) { $shortname = $_POST['shortname']; }
      else { $shortname = 'test'; }
      $p->setEvent($shortname);
      $p->opt_status = generateRandomStatus();
      $p->creation_time = date('Y-m-d H:i:s', (time()-1));
      $p->last_updated  = date('Y-m-d H:i:s', (time()+1));
      $p->useNullLastUpdatedDb = false;
      if(isset($_POST['expired'])) { $p->expiry_date = "1999-12-31 12:34:56"; } // some arbitrary old date
      $p->makePfifNote = true; // create a pfif note
      $p->arrival_website = true; // show they were reported via the website
      //  Update person with this image.
      $invalid = false;
      if (!add_image($p, $dir3.$file, $image, $ids, $num_names)) {
        $invalid = true;
        echo '<br><span style="color: red;">invalid image file: <b>'.$file.'</b></span><br>';
      }
      // Don't add person if any images are invalid.
      if (!$invalid) {
        $p->insert();
        $count++;
        $image_count += count($p->images);
        echo "#$count ";
      }
      // purge objects
      $i = null;
      $p = null;
      ob_flush();
      ob_end_flush();
      flush();
    }
    ob_start();
    echo "<br><b>Finished adding $count total persons and $image_count total images to event $shortname.";
    taupo_acl_log('BATCH_UPLOAD', 'user '.$_SESSION['user'].' performed a batch upload into event: '.$shortname);
    ob_flush();
    ob_end_flush();
    flush();
  }
  // Different persons may share the same image so delete everything at end.
  exec("rm -rf {$dir3}");
}


function add_image($p, $file, $image, $ids, $num_names) {
  global $global, $conf, $prevnames;
  // Create new image.
  $i = new image();
  $i->init();
  $i->p_uuid = $p->p_uuid;
  $i->fileContent = file_get_contents($file);
  $i->principal = 1;
  $regions = $image[1];
  $age = $image[2];
  $gender = $image[3];
  $i->face_region = $regions;
  if ($age == 'adult') {
    $p->maxAge = MAXIMUM_AGE;
    $p->minAge = 18;
  } elseif ($age == 'youth') {
    $p->maxAge = 17;
    $p->minAge = MINIMUM_AGE;
  } elseif ($age != '-1') { $p->years_old = (int)$age;}
  if ($gender == 'male') { $p->opt_gender = 'mal'; }
  elseif ($gender == 'female') { $p->opt_gender = 'fml'; }
  if ($num_names == 0) {
    // Extract names from file name.
    $parts = explode(".", $file);
    list($p->given_name, $p->family_name) = explode("_", $parts[0]."_", 2); // appended "_" makes list() happy when not present
  } else {
    // If this ID already encountered, used saved name.
    if (array_key_exists($image[4], $prevnames)) { $p->full_name = $prevnames[$image[4]][0]; }
    else {
      $index = rand(1,$num_names);
      $tblname = ($gender == 'female')? 'biu_fnames':'biu_mnames';
      $query = "select given_name,family_name FROM $tblname where id=$index";
        $result = $global['db']->Execute($query);
        if($result === false) {
          $errchk = $global['db']->ErrorMsg();
          die("Error getting names from $tblname: " . $errchk);
        }
        $row = $result->FetchRow();
        $p->full_name = $row['given_name'].' '.$row['family_name'];
        // Associate these name with this ID.
        if (!empty($image[4]) && !array_key_exists($image[4], $prevnames)) {
          $prevnames[$image[4]] = [];
          $prevnames[$image[4]][0] = $row['given_name'].' '.$row['family_name'];
        }
      }
  }
  // Add it to person.
  $p->images[] = $i;
  return !$i->invalid;
}


function generateRandomString($length = 8) {
  $characters = 'abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ';
  $randomString = '';
  for ($i = 0; $i < $length; $i++) { $randomString .= $characters[rand(0, strlen($characters) - 1)]; }
  return $randomString;
}

function generateRandomStatus() {
  $statuses=["unk","ali","mis","dec"];
  $random_key=array_rand($statuses);
  return $statuses[$random_key];
}

die();
