<?
/**
 * @name     BatchReport
 * @author   pl@miernicki.com
 * @about    Developed by the U.S. National Library of Medicine
 * @link     https://gitlab.com/tehk/people-locator
 * @license  https://gitlab.com/tehk/people-locator/blob/master/LICENSE
 */

$global['use_stream'] = true; // use lib_stream

function taupo_br_default() {
  global $global, $conf;
  echo '<div id="adminHolder" class="biuHolder">';
  $error = taupo_br_perms();
  echo '<style>'.file_get_contents($conf['approot']."mod/br/br.css").'</style>';
  $q = "
    SELECT *
    FROM `incident`;
  ";
  $result = $global['db']->Execute($q);
  if ($result === false) {daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "br 0");}
  $options = array();
  while (!$result->EOF) {
    $options[$result->fields['shortname']] = json_decode($result->fields['translations'])->en->name;
    $result->MoveNext();
  }
  echo '<form action="br.php" id="myForm" target="hidden_iframe" method="post" enctype="multipart/form-data">';
  echo 'Select an event to upload reports to:';
  echo '<br><select name="shortname" id="shortname">';
  foreach ($options as $opt_value => $desc) {
    $sel = ($opt_value == 'test') ? 'SELECTED' : '';
    echo '<option value="'.$opt_value.'" '.$sel.'>'.$desc.'</option>';
  }
  echo '</select>';
  echo '<br><br>Select the report format:';
  echo '<br><select name="report_format" id="report_format">';
  echo '<option value="sde">San Diego EMS</option>';
  echo '</select>';
  echo '<br><br>Select the reportee\'s status:';
  $options = array('mis'=>'Missing','ali'=>'Found');
  echo '<br><select name="status" id="status">';
  foreach ($options as $opt_value => $desc) {
    $sel = ($opt_value == 'mis') ? 'SELECTED' : '';
    echo '<option value="'.$opt_value.'" '.$sel.'>'.$desc.'</option>';
  }
  echo '</select>';
  if(!$error) {
    echo '
      <br>
      <br>
      Choose a .csv file containing person records in the selected report format: 
      <br><br>
      <input type="hidden" value="myForm" name="'.ini_get("session.upload_progress.name").'">
      <input type="hidden" value="'.$conf['service_key'].'" name="service_key">
      <input type="file" style="border: none; padding: 0;" name="userfile" id="userfile" onchange="enableUploadButton();"><br>
      <br>
      <input class="onWhiteButton bgGreen" style="margin: 0; opacity: 0.2; border: 1px  solid green;" id="startupload" type="submit" value="Start Upload" DISABLED><br>
    </form>
    <br>
    <h2>Batch report progress...</h2>
    <iframe id="hidden_iframe" name="hidden_iframe" src="about:blank"></iframe>
    ';
  }
  echo '<script>'.file_get_contents($conf['approot']."mod/br/br.js").'</script>';
  echo '</div>';
}

function taupo_br_perms() {
  global $global, $conf;
  $error = false;
  $dir = $conf['approot'].'www/tmp/br_cache/';
  // check read and write permissions
  if(file_exists($dir)) {
    try { $fp = fopen($dir."testFilePermissions","w"); }
    catch (Exception $e2) { $error = true; }
  } else { $error = true; }
  if(get_resource_type($fp) != "stream") { $error = true; }
  if(!$error) { 
    fclose($fp);
    unlink($dir."testFilePermissions");
  } else { echo "<script>$(document).ready(function(){ toast('Unable to open storage folder tmp/br_cache. Create the folder and give it read/write permissions by the webserver.'); });</script>"; }
  return($error);
}
