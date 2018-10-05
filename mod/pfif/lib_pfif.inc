<?
/**
 * @name     Person Finder Interchange Format
 * @author   pl@miernicki.com
 * @about    Developed by the U.S. National Library of Medicine
 * @link     https://gitlab.com/tehk/people-locator
 * @license  https://gitlab.com/tehk/people-locator/blob/master/LICENSE
 */

global $global, $conf;
require_once($conf['approot'].'inc/lib_includes.inc');

// save image to database
function pfif_image_to_db($p_uuid,$note_record_id,$image_type,$image_height,$image_width,$channels,$url=null,$thumbUrl=null,$original_filename=null) {
  global $global, $conf;
  $principal = 1;
  $note_id = "NULL"; 
  // if this came with a note, make it principal if first one
  if(!is_null($note_record_id)) {
    // came with a note
    $sql = "SELECT * FROM image WHERE p_uuid = '$p_uuid'";
    $res = $global['db']->GetRow($sql);
    if(count($res) != 0) {
      $principal = 0;
    }
    $note_id = "'$note_record_id'";
  } else {
    // came with person; if note image already exists, make this secondary
    $sql = "SELECT * FROM image WHERE p_uuid = '$p_uuid'";
    $res = $global['db']->GetRow($sql);
    if(count($res) != 0) {
      $principal = 0;
    }
  }
  // insert
  $sql = "
    INSERT INTO image
    (p_uuid, note_record_id, image_type, image_height, image_width, color_channels, url, url_thumb, original_filename, principal, note_id)
    VALUES
    ('$p_uuid', $note_id, '$image_type', '$image_height', '$image_width', '$channels', '$url', '$thumbUrl', '$original_filename', '$principal', NULL);
  ";
  $global['db']->Execute($sql);
  $msg = $global['db']->ErrorMsg();
  if(!empty($msg)) {
    throw new RuntimeException($msg);
  }
}

// Check if the given mime type of (image) is valid
function pfif_check_valid_image_type($type) {
  $mime = $type;
  if(strpos($type,"/")){
    list(,$mime) = explode("/",$type);
  }
  // check types.
  if(array_search($mime,array('png','gif','jpeg','bmp')) === false ){
    return false;
  } else {
    return true;
  }
}

// db Update Array functions
function pfif_db_update($arr,$table, $key) {
  global $global, $conf;
  $sql = "UPDATE $table SET ";
  foreach($arr as $k => $v){
    if($v == '') {
      $sql .= "$k = NULL, ";
    } else {
      $sql .= "$k = ".$global['db']->qstr(trim($v),get_magic_quotes_gpc()).", ";
    }
  }
  $sql = substr($sql,0,strlen($sql)-2);
  if($key) {
    $sql .= " $key";
  }
  if($key) {
    $global['db']->Execute($sql);
  }
}

// db Insert Array functions
function pfif_db_insert($arr, $table, $audit=false, $primary_field='') {
  global $global, $conf;
  $keys = "";
  $values = "";
  $sql = "INSERT INTO $table ";
  foreach($arr as $k => $v) {
    $keys .= "$k , ";
    if($v == '') {
      $values .= "'NULL', ";
    } elseif ($v == 'TRUE') {
      $values .= "TRUE, ";
    } else {
      $values .= $global['db']->qstr(trim($v),get_magic_quotes_gpc()).", ";
    }
  }
  $keys = substr($keys, 0, strlen($keys)-2);
  $values = substr($values, 0, strlen($values)-2);
  $sql .= "( $keys ) VALUES ( $values ) ";
  $global['db']->Execute($sql);
  $msg = $global['db']->ErrorMsg();
  if (!empty($msg)) {
    throw new RuntimeException($msg);
  }
}
