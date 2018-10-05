<?
/**
 * @name     note class
 * @author   pl@miernicki.com
 * @about    Developed by the U.S. National Library of Medicine
 * @link     https://gitlab.com/tehk/people-locator
 * @license  https://gitlab.com/tehk/people-locator/blob/master/LICENSE
 */

class note {
  public $note_id;
  //
  public $p_uuid;
  public $note;
  public $when;
  public $stat;
  public $loc;
  public $pfif_note_id;
  public $uid;
  //
  public $Op_uuid;
  public $Onote;
  public $Owhen;
  public $Ostat;
  public $Oloc;
  public $Opfif_note_id;
  public $Ouid;
  //
  public $sql_p_uuid;
  public $sql_note;
  public $sql_when;
  public $sql_stat;
  public $sql_loc;
  public $sql_pfif_note_id;
  public $sql_uid;
  //
  public $sql_Op_uuid;
  public $sql_Onote;
  public $sql_Owhen;
  public $sql_Ostat;
  public $sql_Oloc;
  public $sql_Opfif_note_id;
  public $sql_Ouid;
  //
  private $saved;
  private $modified;
  
  // constructor
  public function __construct() {
    global $global, $conf;
    $this->db            = $global['db'];
    $this->note_id       = null;
    $this->p_uuid        = null;
    $this->uid           = 3;
    $this->author_user   = null;
    $this->note          = null;
    $this->when          = null;
    $this->stat          = null;
    $this->loc           = null;
    $this->pfif_note_id  = null;
    $this->Op_uuid       = null;
    $this->Ouid          = 3;
    $this->Onote         = null;
    $this->Owhen         = null;
    $this->Ostat         = null;
    $this->Oloc          = null;
    $this->Opfif_note_id = null;
    $this->modified      = false;
    $this->saved         = false;
  }

  // destructor
  public function __destruct() {}

  // init values for a new instance // instead of a previous instance
  public function init() {
    $this->saved = false;
  }

  // load from db
  public function load() {
    global $global;
    $q = "
      SELECT *
      FROM   `person_notes`
      WHERE  `note_id` = ".$global['db']->qstr((string)$this->note_id).";
    ";
    $result = $this->db->Execute($q);
    if($result === false) { daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $this->db->ErrorMsg(), "note load 1 ((".$q."))"); }
    if($result != NULL && !$result->EOF) {
      $this->p_uuid        = $result->fields['p_uuid'];
      $this->note          = $result->fields['note'];
      $this->when          = $result->fields['when'];
      $this->stat          = $result->fields['stat'];
      $this->loc           = $result->fields['loc'];
      $this->pfif_note_id  = $result->fields['pfif_note_id'];
      $this->uid           = $result->fields['uid'];
      // original values for updates
      $this->Op_uuid       = $result->fields['p_uuid'];
      $this->Onote         = $result->fields['note'];
      $this->Owhen         = $result->fields['when'];
      $this->Ostat         = $result->fields['stat'];
      $this->Oloc          = $result->fields['loc'];
      $this->Opfif_note_id = $result->fields['pfif_note_id'];
      $this->Ouid          = $result->fields['uid'];
      $this->saved = true;
    // load failure
    } else { return false; }
  }
  
  // additional note data
  public function loadExtraData($gid) {
    global $global;
    // if admin
    if($gid == 1) {
      // get comment user
      $q = "
        SELECT * 
        FROM   `users`
        WHERE  `uid` = ".$this->db->qstr((string)$this->uid)."
        LIMIT  1;
      ";
      $result = $this->db->Execute($q);
      if($result === false) { daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $this->db->ErrorMsg(), "person notes loadExtras 1 ((".$q."))"); }
      $this->author_user = $result->fields['user'];
    }
  }
  
  // for json output
  public function makeArrayObject() {
    global $conf;
    $r = array();
    $ld = json_decode($this->loc);
    if($ld == null) {
      $lat = 0;
      $lon = 0;
    } else {
      $lat = $ld->lat;
      $lon = $ld->lon;
    }
    if(isset($ld->stat)) { $sug = $ld->stat; }
    else { $sug = ''; }
    $r['id']        = (int)$this->note_id;
    $r['comment']   = (string)strip_tags(preg_replace('/[\x00-\x1F\x80-\x9F]/u', '', $this->note));
    $r['stat']      = (string)$this->stat;
    $r['latitude']  = (float)$lat;
    $r['longitude'] = (float)$lon;
    $r['image_url'] = null;
    $r['thumb_url'] = null;
    $this->when == null ? $r['when'] = null : $r['when'] = (string)date('c', strtotime($this->when));
    if($this->author_user !== null) {
      $r['author_uuid'] = (string)$this->uid;
      $r['author_user'] = (string)$this->author_user;
    }
    return $r;
  }

  // delete function
  public function delete() {
    $this->sync();
    $q = "
      DELETE FROM `person_notes`
      WHERE `note_id` = ".$this->sql_note_id.";
    ";
    $result = $this->db->Execute($q);
    if($result === false) { daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $this->db->ErrorMsg(), "person note delete 1 ((".$q."))"); }
  }

  // synchronize pre db op
  private function sync() {
    global $global;
    // validate
    if($this->stat != "ali" && $this->stat != "mis" && $this->stat != "inj" && $this->stat != "dec" && $this->stat != "unk" && $this->stat != "fnd") { $this->stat = null; }
    // truncate large notes
    if(strlen($this->note) > 1024) { $this->note = substr($this->note, 0, 1023);}
    $this->sql_note_id       = ($this->note_id       === null) ? "NULL" : (int)$this->note_id;
    $this->sql_p_uuid        = ($this->p_uuid        === null) ? "NULL" : $global['db']->qstr((string)$this->p_uuid);
    $this->sql_note          = ($this->note          === null) ? "NULL" : $global['db']->qstr((string)$this->note);
    $this->sql_when          = ($this->when          === null) ? "NULL" : $global['db']->qstr((string)$this->when);
    $this->sql_stat          = ($this->stat          === null) ? "NULL" : "'".$this->stat."'";
    $this->sql_loc           = ($this->loc           === null) ? "NULL" : $global['db']->qstr((string)$this->loc);
    $this->sql_pfif_note_id  = ($this->pfif_note_id  === null) ? "NULL" : $global['db']->qstr((string)$this->pfif_note_id);
    $this->sql_uid           = ($this->uid           === null) ? "NULL" : $global['db']->qstr((string)$this->uid);
    $this->sql_Op_uuid       = ($this->Op_uuid       === null) ? "NULL" : $global['db']->qstr((string)$this->Op_uuid);
    $this->sql_Onote         = ($this->Onote         === null) ? "NULL" : $global['db']->qstr((string)$this->Onote);
    $this->sql_Owhen         = ($this->Owhen         === null) ? "NULL" : $global['db']->qstr((string)$this->Owhen);
    $this->sql_Ostat         = ($this->Ostat         === null) ? "NULL" : "'".$this->Ostat."'";
    $this->sql_Oloc          = ($this->Oloc          === null) ? "NULL" : $global['db']->qstr((string)$this->Oloc);
    $this->sql_Opfif_note_id = ($this->Opfif_note_id === null) ? "NULL" : $global['db']->qstr((string)$this->Opfif_note_id);
    $this->sql_Ouid          = ($this->Ouid          === null) ? "NULL" : $global['db']->qstr((string)$this->Ouid);
  }

  // initial save
  public function insert() {
    if($this->saved) { $this->update(); }
    else {
      $this->sync();
      $q = "
        INSERT INTO `person_notes` (
          p_uuid,
          note,
          `when`,
          stat,
          loc,
          pfif_note_id,
          uid )
        VALUES (
          ".$this->sql_p_uuid.",
          ".$this->sql_note.",
          ".$this->sql_when.",
          ".$this->sql_stat.",
          ".$this->sql_loc.",
          ".$this->sql_pfif_note_id.",
          ".$this->sql_uid." );
      ";
      $result = $this->db->Execute($q);
      if($result === false) { daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $this->db->ErrorMsg(), "note insert ((".$q."))"); }
      $this->saved = true;
    }
  }

  // subsequent save
  public function update() {
    if(!$this->saved) { $this->insert(); }
    else {
      $this->sync();
      $this->saveRevisions();
      if($this->modified) {
        $q = "
          UPDATE person_notes
          SET
            p_uuid       = ".$this->sql_p_uuid.",
            note         = ".$this->sql_note.",
            `when`       = ".$this->sql_when.",
            stat         = ".$this->sql_stat.",
            loc          = ".$this->sql_loc.",
            pfif_note_id = ".$this->sql_pfif_note_id.",
            uid          = ".$this->sql_uid."
          WHERE note_id  = ".$this->sql_note_id.";
        ";
        $result = $this->db->Execute($q);
        if($result === false) { daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $this->db->ErrorMsg(), "person note update ((".$q."))"); }
      }
      $this->modified = false;
    }
  }

  // save any changes since object was loaded as revisions
  function saveRevisions() {
    if($this->p_uuid       != $this->Op_uuid)       { $this->saveRevision($this->sql_p_uuid,       $this->sql_Op_uuid,       'person_notes', 'p_uuid'      ); }
    if($this->note         != $this->Onote)         { $this->saveRevision($this->sql_note,         $this->sql_Onote,         'person_notes', 'note'        ); }
    if($this->when         != $this->Owhen)         { $this->saveRevision($this->sql_when,         $this->sql_Owhen,         'person_notes', 'when'        ); }
    if($this->stat         != $this->Ostat)         { $this->saveRevision($this->sql_stat,         $this->sql_Ostat,         'person_notes', 'stat'        ); }
    if($this->loc          != $this->Oloc)          { $this->saveRevision($this->sql_loc,          $this->sql_Oloc,          'person_notes','loc'          ); }
    if($this->pfif_note_id != $this->Opfif_note_id) { $this->saveRevision($this->sql_pfif_note_id, $this->sql_Opfif_note_id, 'person_notes', 'pfif_note_id'); }
    if($this->uid          != $this->Ouid)          { $this->saveRevision($this->sql_uid,          $this->sql_Ouid,          'person_notes', 'uid'         ); }
  }

  // save revision
  function saveRevision($newValue, $oldValue, $table, $column) {
    $this->modified = true;
    $q = "
      INSERT INTO `person_updates` (`p_uuid`, `updated_table`, `updated_column`, `old_value`, `new_value`, `uid`)
      VALUES (".$this->sql_p_uuid.", '".$table."', '".$column."', ".$oldValue.", ".$newValue.", '".$this->uid."');
    ";
    $result = $this->db->Execute($q);
    if($result === false) { daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $this->db->ErrorMsg(), "person note saveRevision ((".$q."))"); }
  }
}
