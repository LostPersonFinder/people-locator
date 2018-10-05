<?
/**
 * @name     BatchImageUploader
 * @author   pl@miernicki.com
 * @about    Developed in whole the U.S. National Library of Medicine
 * @link     https://gitlab.com/tehk/people-locator
 * @license  https://gitlab.com/tehk/people-locator/blob/master/LICENSE
 */

$global['use_stream'] = true; // use lib_stream

function taupo_biu_default() {
  global $global, $conf;
  echo '<div id="adminHolder" class="biuHolder">';
  $error = taupo_biu_perms();
  echo '
    <style>'.file_get_contents($conf['approot']."mod/biu/biu.css").'</style>
    Select an event to upload data into...
  ';
  $q = "
    SELECT *
    FROM `incident`;
  ";
  $result = $global['db']->Execute($q);
  if($result === false) {  daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "biu 0"); }
  $options = array();
  while(!$result->EOF) {
    $options[$result->fields['shortname']] = json_decode($result->fields['translations'])->en->name;
    $result->MoveNext();
  }
  echo '<form action="biu.php" id="myForm" target="hidden_iframe" method="post" enctype="multipart/form-data">';
  echo '<select name="shortname" id="shortname">';
  foreach($options as $opt_value => $desc) {
    $sel = ($opt_value == 'test') ? 'SELECTED' : '';
    echo '<option value="'.$opt_value.'" '.$sel.'>'.$desc.'</option>';
  }
  echo '</select>';
  if(!$error) {
    echo '
      <br>
      <br>
      Choose a .zip file containing .gif .jpg or .png images with a maximum size of '.ini_get("upload_max_filesize").'B 
      or supply a text file containing image paths and image metadata in the ground truth format.
      Each person record created in this fashion is randomly assigned a name and assigned a status of missing.<br>
      <br>
      <input type="hidden" value="myForm" name="'.ini_get("session.upload_progress.name").'">
      <input type="hidden" value="'.$conf['service_key'].'" name="service_key">
      <input type="file" style="border: none; padding: 0;" name="userfile" id="userfile" onchange="enableUploadButton();"><br>
      <br>
      <input type="checkbox" name="expired" id="expired">Expire records<br>
      <br>
      <input class="onWhiteButton bgGreen" style="margin: 0; opacity: 0.2; border: 1px  solid green;" id="startupload" type="submit" value="Start Upload" DISABLED><br>
      <br>
      File upload progress:
      <div id="bar_blank">
           <div id="bar_color"></div>
      </div>
      <div id="status">0%</div>
    </form>
    <br>
    <h2>Batch image upload progress...</h2>
    <iframe id="hidden_iframe" name="hidden_iframe" src="about:blank"></iframe>
    ';
  }
  echo '<script>'.file_get_contents($conf['approot']."mod/biu/biu.js").'</script>';
  echo '</div>';
}

function taupo_biu_progress() {
  $key = ini_get("session.upload_progress.prefix") . "myForm";
  if(!empty($_SESSION[$key])) {
    $current = $_SESSION[$key]["bytes_processed"];
    $total = $_SESSION[$key]["content_length"];
    echo $current < $total ? ceil($current / $total * 100) : 100;
  } else { echo 100; }
}

function taupo_biu_perms() {
  global $global, $conf;
  $error = false;
  $dir = $conf['approot'].'www/tmp/biu_cache/';
  // check read and write permissions
  if(file_exists($dir)) {
    try { $fp = fopen($dir."testFilePermissions","w"); }
    catch (Exception $e2) { $error = true; }
  } else { $error = true; }
  if(get_resource_type($fp) != "stream") { $error = true; }
  if(!$error) {
    fclose($fp);
    unlink($dir."testFilePermissions");
  } else {
    echo "<script>$(document).ready(function(){ toast('Unable to open storage folder tmp/biu_cache. Create the folder and give it read/write permissions by the webserver.'); });</script>";
  }
  return($error);
}
