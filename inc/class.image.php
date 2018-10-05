<?
/**
 * @name     image class
 * @author   pl@miernicki.com
 * @about    Developed by the U.S. National Library of Medicine
 * @link     https://gitlab.com/tehk/people-locator
 * @license  https://gitlab.com/tehk/people-locator/blob/master/LICENSE
 */
 
 
class image {
  public $image_id;
  public $p_uuid;
  public $note_record_id;
  public $image_type;
  public $image_height;
  public $image_width;
  public $created;
  public $url;
  public $url_thumb;
  public $original_filename;
  public $principal;
  public $sha1original;
  public $color_channels;
  public $note_id;
  //
  public $fileContentBase64;
  public $fileContent;
  public $fullSizePath;
  public $thumbnailPath;
  //
  public $Oimage_id;
  public $Op_uuid;
  public $Onote_record_id;
  public $Oimage_type;
  public $Oimage_height;
  public $Oimage_width;
  public $Ocreated;
  public $Ourl;
  public $Ourl_thumb;
  public $Ooriginal_filename;
  public $Oprincipal;
  public $Osha1original;
  public $Ocolor_channels;
  public $Onote_id;
  //
  public $OfileContentBase64;
  public $OfileContent;
  public $OfullSizePath;
  public $OthumbnailPath;
  //
  private $sql_image_id;
  private $sql_p_uuid;
  private $sql_note_record_id;
  private $sql_image_type;
  private $sql_image_height;
  private $sql_image_width;
  private $sql_created;
  private $sql_url;
  private $sql_url_thumb;
  private $sql_original_filename;
  private $sql_principal;
  private $sql_sha1original;
  private $sql_color_channels;
  private $sql_note_id;
  //
  private $sql_Oimage_id;
  private $sql_Op_uuid;
  private $sql_Onote_record_id;
  private $sql_Oimage_type;
  private $sql_Oimage_height;
  private $sql_Oimage_width;
  private $sql_Ocreated;
  private $sql_Ourl;
  private $sql_Ourl_thumb;
  private $sql_Ooriginal_filename;
  private $sql_Oprincipal;
  private $sql_Osha1original;
  private $sql_Ocolor_channels;
  private $sql_Onote_id;
  //
  public $update_uid;
  public $invalid;
  public $nonDeleteFlag;
  public $incident_id;
  public $face_region;
  public $source;
  private $modified;
  private $saved;
  
  // constructor
  public function __construct() {
    global $global;
    $this->db = $global['db'];
    $this->image_id              = null;
    $this->p_uuid                = null;
    $this->note_record_id        = null;
    $this->image_type            = null;
    $this->image_height          = null;
    $this->image_width           = null;
    $this->created               = null;
    $this->url                   = null;
    $this->url_thumb             = null;
    $this->original_filename     = null;
    $this->principal             = 1;
    $this->sha1original          = null;
    $this->color_channels        = null;
    $this->note_id               = null;
    $this->fileContentBase64     = null;
    $this->fileContent           = null;
    $this->fullSizePath          = null;
    $this->thumbnailPath         = null;
    $this->Oimage_id             = null;
    $this->Op_uuid               = null;
    $this->Onote_record_id       = null;
    $this->Oimage_type           = null;
    $this->Oimage_height         = null;
    $this->Oimage_width          = null;
    $this->Ocreated              = null;
    $this->Ourl                  = null;
    $this->Ourl_thumb            = null;
    $this->Ooriginal_filename    = null;
    $this->Oprincipal            = 1;
    $this->Osha1original         = null;
    $this->Ocolor_channels       = null;
    $this->Onote_id              = null;
    $this->OfileContentBase64    = null;
    $this->OfileContent          = null;
    $this->OfullSizePath         = null;
    $this->OthumbnailPath        = null;
    $this->sql_image_id          = null;
    $this->sql_p_uuid            = null;
    $this->sql_note_record_id    = null;
    $this->sql_image_type        = null;
    $this->sql_image_height      = null;
    $this->sql_image_width       = null;
    $this->sql_created           = null;
    $this->sql_url               = null;
    $this->sql_url_thumb         = null;
    $this->sql_original_filename = null;
    $this->sql_principal         = null;
    $this->sql_sha1original      = null;
    $this->sql_color_channels    = null;
    $this->sql_note_id           = null;
    $this->sql_Oimage_id         = null;
    $this->sql_Op_uuid           = null;
    $this->sql_Onote_record_id   = null;
    $this->sql_Oimage_type       = null;
    $this->sql_Oimage_height     = null;
    $this->sql_Oimage_width      = null;
    $this->sql_Ocreated          = null;
    $this->sql_Ourl              = null;
    $this->sql_Ourl_thumb        = null;
    $this->sql_Ooriginal_filename= null;
    $this->sql_Oprincipal        = null;
    $this->sql_Osha1original     = null;
    $this->sql_Ocolor_channels   = null;
    $this->sql_Onote_id          = null;
    $this->update_uid            = 1;
    $this->invalid               = false; // false by default, true when the image data turns out to be an invalid mime type
    $this->modified              = false;
    $this->saved                 = false;
    $this->nonDeleteFlag         = false;
    $this->incident_id           = null;
    $this->face_region           = null;
    $this->source                = getPlatformSource();
  }

  // destructor
  public function __destruct() {}

  // new image defaults
  public function init() {
    global $global, $conf;
    $this->saved = false;
  }

  // load from db
  public function load() {
    global $global, $conf;
    $q = "
      SELECT *
      FROM image
      WHERE image_id = ".$global['db']->qstr((string)$this->image_id).";
    ";
    $result = $this->db->Execute($q);
    if($result === false) { daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $this->db->ErrorMsg(), "image load 1 ((".$q."))"); }
    if($result != NULL && !$result->EOF) {
      $this->p_uuid                = $result->fields['p_uuid'];
      $this->note_record_id        = $result->fields['note_record_id'];
      $this->image_id              = $result->fields['image_id'];
      $this->image_type            = $result->fields['image_type'];
      $this->image_width           = $result->fields['image_width'];
      $this->image_height          = $result->fields['image_height'];
      $this->created               = $result->fields['created'];
      $this->url                   = $result->fields['url'];
      $this->url_thumb             = $result->fields['url_thumb'];
      $this->original_filename     = $result->fields['original_filename'];
      $this->principal             = $result->fields['principal'];
      $this->sha1original          = $result->fields['sha1original'];
      $this->color_channels        = $result->fields['color_channels'];
      $this->note_id               = $result->fields['note_id'];
      $this->fullSizePath          = $conf['approot']."www/".$result->fields['url'];
      $this->thumbnailPath         = $conf['approot']."www/".$result->fields['url_thumb'];
      // copy the original values for use in diff'ing an update
      $this->Op_uuid               = $this->p_uuid;
      $this->Onote_record_id       = $this->note_record_id;
      $this->Oimage_id             = $this->image_id;
      $this->Oimage_type           = $this->image_type;
      $this->Oimage_width          = $this->image_width;
      $this->Oimage_height         = $this->image_height;
      $this->Ocreated              = $this->created;
      $this->Ourl                  = $this->url;
      $this->Ourl_thumb            = $this->url_thumb;
      $this->Ooriginal_filename    = $this->original_filename;
      $this->Oprincipal            = $this->principal;
      $this->Osha1original         = $this->sha1original;
      $this->Ocolor_channels       = $this->color_channels;
      $this->Onote_id              = $this->note_id;
      $this->OfullSizePath         = $this->fullSizePath;
      $this->OthumbnailPath        = $this->thumbnailPath;
      $this->OfileContent          = $this->fileContent;
      $this->OfileContentBase64    = $this->fileContentBase64;
      // object exists in the db
      $this->saved = true;
    }
  }

  // for return of web services
  public function makeArrayObject() {
    $r = array();
    $r['image_id']  = $this->image_id;
    $r['image_url'] = $this->url;
    $r['thumb_url'] = $this->url_thumb;
    return $r;
  }
  
  // delete function
  public function delete() {
    // remove from filesystem this image
    $this->unwrite();
    // delete from db
    $q = "
      DELETE FROM image
      WHERE image_id = '".$this->image_id."';
    ";
    $result = $this->db->Execute($q);
    if($result === false) { daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $this->db->ErrorMsg(), "person image delete 1 ((".$q."))"); }
  }

  // synchronize SQL value strings with public attributes
  private function sync() {
    // build SQL strings from values
    $this->sql_image_id          = ($this->image_id          === null) ? "NULL" : "'".(int)$this->image_id."'";
    $this->sql_p_uuid            = ($this->p_uuid            === null) ? "NULL" : $this->db->qstr((string)$this->p_uuid);
    $this->sql_note_record_id    = ($this->note_record_id    === null) ? "NULL" : $this->db->qstr((string)$this->note_record_id);
    $this->sql_image_type        = ($this->image_type        === null) ? "NULL" : $this->db->qstr((string)$this->image_type);
    $this->sql_image_height      = ($this->image_height      === null) ? "NULL" : "'".(int)$this->image_height."'";
    $this->sql_image_width       = ($this->image_width       === null) ? "NULL" : "'".(int)$this->image_width."'";
    $this->sql_created           = ($this->created           === null) ? "NULL" : $this->db->qstr((string)$this->created);
    $this->sql_url               = ($this->url               === null) ? "NULL" : $this->db->qstr((string)$this->url);
    $this->sql_url_thumb         = ($this->url_thumb         === null) ? "NULL" : $this->db->qstr((string)$this->url_thumb);
    $this->sql_original_filename = ($this->original_filename === null) ? "NULL" : $this->db->qstr((string)$this->original_filename);
    $this->sql_principal         = ($this->principal         === null) ? "'1'"  : $this->db->qstr((int)$this->principal);
    $this->sql_sha1original      = ($this->sha1original      === null) ? "NULL" : $this->db->qstr((string)$this->sha1original);
    $this->sql_color_channels    = ($this->color_channels    === null) ? "'3'"  : $this->db->qstr((string)$this->color_channels);
    $this->sql_note_id           = ($this->note_id           === null) ? "NULL" : $this->db->qstr((string)$this->note_id);
    $this->sql_Oimage_id         = ($this->Oimage_id         === null) ? "NULL" : "'".(int)$this->Oimage_id."'";
    $this->sql_Op_uuid           = ($this->Op_uuid           === null) ? "NULL" : $this->db->qstr((string)$this->Op_uuid);
    $this->sql_Onote_record_id   = ($this->Onote_record_id   === null) ? "NULL" : $this->db->qstr((string)$this->Onote_record_id);
    $this->sql_Oimage_type       = ($this->Oimage_type       === null) ? "NULL" : $this->db->qstr((string)$this->Oimage_type);
    $this->sql_Oimage_height     = ($this->Oimage_height     === null) ? "NULL" : "'".(int)$this->Oimage_height."'";
    $this->sql_Oimage_width      = ($this->Oimage_width      === null) ? "NULL" : "'".(int)$this->Oimage_width."'";
    $this->sql_Ocreated          = ($this->Ocreated          === null) ? "NULL" : $this->db->qstr((string)$this->Ocreated);
    $this->sql_Ourl              = ($this->Ourl              === null) ? "NULL" : $this->db->qstr((string)$this->Ourl);
    $this->sql_Ourl_thumb        = ($this->Ourl_thumb        === null) ? "NULL" : $this->db->qstr((string)$this->Ourl_thumb);
    $this->sql_Ooriginal_filename= ($this->Ooriginal_filename=== null) ? "NULL" : $this->db->qstr((string)$this->Ooriginal_filename);
    $this->sql_Oprincipal        = ($this->Oprincipal        === null) ? "'1'"  : $this->db->qstr((int)$this->Oprincipal);
    $this->sql_Osha1original     = ($this->Osha1original     === null) ? "NULL" : $this->db->qstr((string)$this->Osha1original);
    $this->sql_Ocolor_channels   = ($this->Ocolor_channels   === null) ? "'3'"  : $this->db->qstr((string)$this->Ocolor_channels);
    $this->sql_Onote_id          = ($this->Onote_id          === null) ? "NULL" : $this->db->qstr((string)$this->Onote_id);
  }

  // base64 to bin
  public function decode() {
    global $conf;
    $this->fileContent = base64_decode($this->fileContentBase64);
    $this->fileContentBase64 = null;
    $this->sha1original = sha1($this->fileContent);
  }

  // bin to base64
  public function encode() {
    $this->fileContentBase64 = base64_encode($this->fileContent);
  }

  // remove from filesystem
  private function unwrite() {
    global $global, $conf;
    $webroot = $conf['approot']."www/";
    $file    = $webroot.$this->url;
    $thumb   = $webroot.$this->url_thumb;
    if(!unlink($file)) { daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, "unable to delete file >> ".$file, "person image unwrite 1 ((".$file."))"); }
    // delete the thumb if its different from the fullsized
    if($thumb != $file) {
      if(!unlink($thumb)) {
        daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, "unable to delete file >> ".$thumb, "person image unwrite 2 ((".$thumb."))");
      }
    }
    // report this image deletion to facematch if the search by image service is enabled
    if($conf['image_search']) {
      try {
        taupo_facematch_remove($this->url, $this->incident_id, $this->source);
      } catch (RuntimeException $e) {
        // fm down
      }
    }
  }

  // save to disk
  private function write($age = null, $gender = null, $minAge = null, $maxAge = null, $animal = null) {
    global $global, $conf;
    if($this->fileContentBase64 != null) { $this->decode(); }
    $filename = str_replace(array("/","~"), array("SLASH","TILDE"), $this->p_uuid); // make pl.nlm.nih.gov/person.123456 into pl.nlm.nih.govSLASHperson.123456
    $filename = $filename."__".$this->image_id."_";             // filename now like pl.nlm.nih.gov_person.123456__112233_
    $path = $conf['approot']."www/tmp/rest_cache/".$filename; // path is now like /opt/pl/www/tmp/rest_cache/pl.nlm.nih.gov_person.123456__112233_
    // temporarily save original w/out extension like /opt/pl/www/tmp/rest_cache/pl.nlm.nih.gov_person.123456_112233_original
    file_put_contents($path."original", $this->fileContent);
    // save SHA1 of the file for later identification
    // Leif: already done in decode()
    //$this->sha1original = sha1($this->fileContent);
    // get image information from saved file
    try { $info = getimagesize($path."original"); } catch (Exception $e) { $this->invalid = true; }
    if(!$this->invalid) {
      $this->image_width  = $info[0];
      $this->image_height = $info[1];
      $this->color_channels = isset($info['channels']) ? $info['channels'] : 3;
      list(,$mime) = explode("/",$info['mime']);
      $mime = strtolower($mime);
      $this->image_type = $mime;
      if(stripos($mime,"png") !== FALSE) { $ext = ".png"; }
      elseif(stripos($mime,"tiff") !== FALSE) { $ext = ".gif"; }
      elseif(stripos($mime,"jpeg") !== FALSE) { $ext = ".jpg"; }
      else { $this->invalid = true; } // invalid mime type
    }
    if(!$this->invalid) {
      // set paths
      $this->fullSizePath  = $path."full".$ext;
      $this->thumbnailPath = $path."thumb".$ext;
      // set jpegtran location
      if(isset($conf['jpegtran'])) { $jpegtran = $conf['jpegtran']; }
      else { $jpegtran = '/usr/bin/jpegtran'; }
      // jpegtran optimize; NOTE: thumbnail processing obsolete and should be removed (PL-1861)
      if($ext == ".jpg" && file_exists($jpegtran)) {
        // optimize full image
        system($jpegtran.' -copy none -optimize -progressive '.$path."original".' > '.$this->fullSizePath, $ret);
        // save 566px width thumbnail
        taupo_image_resize_height($path."original", $path."original_thumb", 566); // 566 is the new thumbnail width
        // optimize thumb
        system($jpegtran.' -copy none -optimize -progressive '.$path."original_thumb".' > '.$this->thumbnailPath, $ret);
        // delete original thumb
        unlink($path."original_thumb");
      // tiif or png // optipng is way too slow to use!!
      } else {
        // rename original like /opt/pl/www/tmp/rest_cache/pl.nlm.nih.gov_person.123456_112233_full.ext
        rename($path."original", $this->fullSizePath);
        // save thumb resampled image (566px height) like /opt/pl/www/tmp/rest_cache/pl.nlm.nih.gov_person.123456_112233_thumb.ext
        taupo_image_resize_height($this->fullSizePath, $this->thumbnailPath, 566);
      }
      // update URLs
      $this->url       = "tmp/rest_cache/".$filename."full".$ext;
      $this->url_thumb = "tmp/rest_cache/".$filename."thumb".$ext;
      // make the files world writeable for other users/applications and to handle deletes
      chmod($this->fullSizePath,  0777);
      chmod($this->thumbnailPath, 0777);
      // report this image to facematch
      if(isset($conf['image_search']) && $conf['image_search'] == true) {
        // go with direct age if present otherwise validate min/max and take average
        if($age == null || $age < 0 || $age > 120) {
          if( $minAge == null || $minAge < MINIMUM_AGE || $minAge > MAXIMUM_AGE || 
              $maxAge == null || $maxAge < MINIMUM_AGE || $maxAge > MAXIMUM_AGE ||
              $maxAge < $minAge) {
            $age = -1;
          } else {
            $age = ($minAge + $maxAge) / 2;
          }
        }
        if($gender == 'mal') { $gender = "male"; }
        elseif($gender == 'fml') { $gender = "female"; }
        else { $gender = "unknown"; }
        $attrs = array();
        $attrs['gender'] = $gender;
        $attrs['age'] = $age;
        $attrs['animal'] = $animal;
        try { taupo_facematch_ingest($this->url, $this->incident_id, $this->source, $attrs, $this->face_region); }
        catch (RuntimeException $e) {} // facematch down
      }
    }
  }

  // save
  public function insert($age = null, $gender = null, $minAge = null, $maxAge = null, $animal = null) {
    // if this object is in the db, update it instead
    if($this->saved) { $this->update(); }
    else {
      // save to disk
      $this->write($age, $gender, $minAge, $maxAge, $animal);
      // db insert only a valid image
      if(!$this->invalid) {
        $this->sync();
        $q = "
          INSERT INTO image (
            p_uuid,
            note_record_id,
            image_type,
            image_height,
            image_width,
            url,
            url_thumb,
            original_filename,
            principal,
            sha1original,
            color_channels,
            note_id )
          VALUES (
            ".$this->sql_p_uuid.",
            ".$this->sql_note_record_id.",
            ".$this->sql_image_type.",
            ".$this->sql_image_height.",
            ".$this->sql_image_width.",
            ".$this->sql_url.",
            ".$this->sql_url_thumb.",
            ".$this->sql_original_filename.",
            ".$this->sql_principal.",
            ".$this->sql_sha1original.",
            ".$this->sql_color_channels.",
            ".$this->sql_note_id." );
        ";
        $result = $this->db->Execute($q);  
        if($result === false) { daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $this->db->ErrorMsg(), "image insert ((".$q."))"); }
      }
      $this->saved = true;
      $this->modified = false;
    }
  }

  // obfuscate old image url
  public function randomize() {
    global $global, $conf;
    $this->url       = str_replace(".jpg", "_".$this->generateRandomString().".jpg", $this->url);
    $this->url_thumb = str_replace(".jpg", "_".$this->generateRandomString().".jpg", $this->url_thumb);
    $this->url       = str_replace(".gif", "_".$this->generateRandomString().".gif", $this->url);
    $this->url_thumb = str_replace(".gif", "_".$this->generateRandomString().".gif", $this->url_thumb);
    $this->url       = str_replace(".png", "_".$this->generateRandomString().".png", $this->url);
    $this->url_thumb = str_replace(".png", "_".$this->generateRandomString().".png", $this->url_thumb);
    rename($conf['approot']."www/".$this->Ourl,       $conf['approot']."www/".$this->url);
    rename($conf['approot']."www/".$this->Ourl_thumb, $conf['approot']."www/".$this->url_thumb);
  }
  
  // class lib func
  private function generateRandomString($length = 16) {
    return substr(str_shuffle("0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ"), 0, $length);
  }

  // save the person // subsequent save
  public function update($age = null, $gender = null, $minAge = null, $maxAge = null, $animal = null) {
    // if we've never saved this record before, we can't update it, so insert() instead
    if(!$this->saved) { $this->insert($age, $gender, $minAge, $maxAge, $animal); }
    else {
      $this->sync();
      $this->saveRevisions();
      if($this->modified) {
        $q = "
          UPDATE image
          SET
            p_uuid            = ".$this->sql_p_uuid.",
            note_record_id    = ".$this->sql_note_record_id.",
            image_type        = ".$this->sql_image_type.",
            image_height      = ".$this->sql_image_height.",
            image_width       = ".$this->sql_image_width.",
            url               = ".$this->sql_url.",
            url_thumb         = ".$this->sql_url_thumb.",
            original_filename = ".$this->sql_original_filename.",
            principal         = ".$this->sql_principal."
          WHERE image_id = ".$this->sql_image_id.";
        ";
        $result = $this->db->Execute($q);
        if($result === false) { daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $this->db->ErrorMsg(), "image update ((".$q."))"); }
      }
      $this->modified = false;
    }
  }

  // save any changes since object was loaded as revisions
  function saveRevisions() {
    if($this->p_uuid            != $this->Op_uuid)            { $this->saveRevision($this->sql_p_uuid,            $this->sql_Op_uuid,            'image', 'p_uuid'            ); }
    if($this->note_record_id    != $this->Onote_record_id)    { $this->saveRevision($this->sql_note_record_id,    $this->sql_Onote_record_id,    'image', 'note_record_id'    ); }
    if($this->image_type        != $this->Oimage_type)        { $this->saveRevision($this->sql_image_type,        $this->sql_Oimage_type,        'image', 'image_type'        ); }
    if($this->image_height      != $this->Oimage_height)      { $this->saveRevision($this->sql_image_height,      $this->sql_Oimage_height,      'image', 'image_height'      ); }
    if($this->image_width       != $this->Oimage_width)       { $this->saveRevision($this->sql_image_width,       $this->sql_Oimage_width,       'image', 'image_width'       ); }
    if($this->url               != $this->Ourl)               { $this->saveRevision($this->sql_url,               $this->sql_Ourl,               'image', 'url'               ); }
    if($this->url_thumb         != $this->Ourl_thumb)         { $this->saveRevision($this->sql_url_thumb,         $this->sql_Ourl_thumb,         'image', 'url_thumb'         ); }
    if($this->original_filename != $this->Ooriginal_filename) { $this->saveRevision($this->sql_original_filename, $this->sql_Ooriginal_filename, 'image', 'original_filename' ); }
    if($this->principal         != $this->Oprincipal)         { $this->saveRevision($this->sql_principal,         $this->sql_Oprincipal,         'image', 'principal'         ); }
    if($this->color_channels    != $this->Ocolor_channels)    { $this->saveRevision($this->sql_color_channels,    $this->sql_Ocolor_channels,    'image', 'color_channels'    ); }
    if($this->note_id           != $this->Onote_id)           { $this->saveRevision($this0>sql_note_id,           $this->sql_Onote_id,           'image', 'note_id'           ); }
  }

  // note revision
  function saveRevision($newValue, $oldValue, $table, $column) {
    $this->modified = true;
    $q = "
      INSERT into person_updates (`p_uuid`, `updated_table`, `updated_column`, `old_value`, `new_value`, `uid`)
      VALUES (".$this->sql_p_uuid.", '".$table."', '".$column."', ".$oldValue.", ".$newValue.", '".$this->update_uid."');
    ";
    $result = $this->db->Execute($q);
    if($result === false) { daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $this->db->ErrorMsg(), "image saveRevision ((".$q."))"); }
  }
}
