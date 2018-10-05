<?
/**
 * @name     person class
 * @author   pl@miernicki.com
 * @about    Developed by the U.S. National Library of Medicine
 * @link     https://gitlab.com/tehk/people-locator
 * @license  https://gitlab.com/tehk/people-locator/blob/master/LICENSE
 */

class person {
  // payload
  public $theString;
  public $payloadFormat;
  public $a;
  // table person_uuid
  public $p_uuid;
  public $full_name;
  public $family_name;
  public $given_name;
  public $alternate_names;
  public $profile_urls;
  public $incident_id;
  public $expiry_date;
  public $animal;
  private $Op_uuid;
  private $Ofull_name;
  private $Ofamily_name;
  private $Ogiven_name;
  private $Oalternate_names;
  private $Oprofile_urls;
  private $Oincident_id;
  private $Oexpiry_date;
  private $Oanimal;
  // table person_status
  public $opt_status;
  public $last_updated;
  public $creation_time;
  public $latitude;
  public $longitude;
  private $Oopt_status;
  private $Olast_updated;
  private $Ocreation_time;
  private $Olatitude;
  private $Olongitude;
  // table contact
  public $home_address;
  public $home_zip;
  private $Ohome_address;
  private $Ohome_zip;
  // table person_details
  public $birth_date;
  public $opt_race;
  public $opt_religion;
  public $opt_gender;
  public $years_old;
  public $minAge;
  public $maxAge;
  public $last_seen;
  public $last_clothing;
  public $other_comments;
  private $Obirth_date;
  private $Oopt_race;
  private $Oopt_religion;
  private $Oopt_gender;
  private $Oyears_old;
  private $OminAge;
  private $OmaxAge;
  private $Olast_seen;
  private $Olast_clothing;
  private $Oother_comments;
  // when true we set the last_updated_db to null // holds record from solr indexing
  public $useNullLastUpdatedDb;
  // ignore duplicate check
  public $ignoreDupeUuid;
  // reporting user
  public $reporting_user;
  private $Oreporting_user;
  // to add reporter full name to search
  public $reporter_user;
  // adds editable attribute to search results
  public $is_editable;
  // record images
  public $images;
  // record notes
  public $person_notes;
  // sql strings of the object's attributes
  private $sql_p_uuid;
  private $sql_full_name;
  private $sql_family_name;
  private $sql_given_name;
  private $sql_alternate_names;
  private $sql_profile_urls;
  private $sql_incident_id;
  private $sql_expiry_date;
  private $sql_animal;
  private $sql_opt_status;
  private $sql_last_updated;
  private $sql_creation_time;
  private $sql_latitude;
  private $sql_longitude;
  private $sql_home_address;
  private $sql_home_zip;
  private $sql_birth_date;
  private $sql_opt_race;
  private $sql_opt_religion;
  private $sql_opt_gender;
  private $sql_years_old;
  private $sql_minAge;
  private $sql_maxAge;
  private $sql_last_seen;
  private $sql_last_clothing;
  private $sql_other_comments;
  private $sql_reporting_user;
  // ...and for original values
  private $sql_Op_uuid;
  private $sql_Ofull_name;
  private $sql_Ofamily_name;
  private $sql_Ogiven_name;
  private $sql_Oalternate_names;
  private $sql_Oprofile_urls;
  private $sql_Oincident_id;
  private $sql_Oexpiry_date;
  private $sql_Oanimal;
  private $sql_Oopt_status;
  private $sql_Olast_updated;
  private $sql_Ocreation_time;
  private $sql_Olatitude;
  private $sql_Olongitude;
  private $sql_Obirth_date;
  private $sql_Oopt_race;
  private $sql_Oopt_religion;
  private $sql_Oopt_gender;
  private $sql_Oyears_old;
  private $sql_OminAge;
  private $sql_OmaxAge;
  private $sql_Olast_seen;
  private $sql_Olast_clothing;
  private $sql_Oother_comments;
  private $sql_Oreporting_user;
  // used to make a pfif_note
  public $author_name;
  public $author_email;
  // whether to make a static PFIF not upon insertion
  public $makePfifNote;
  // if we encounter an error anywhere along the way // no error = 0
  public $ecode;
  // if this object has been modified or saved/inserted
  private $modified;
  private $saved;
  // uid of user revising record
  public $update_uid;
  // boolean values to denote the origin of the person // statistical purposes
  public $arrival_reunite;
  public $arrival_website;
  public $arrival_pfif;
  public $arrival_vanilla_email;
  // current user is following this record when true
  public $following;
  
  //  constructor
  public function  __construct() {
    global $global;
    $this->db = $global['db'];
    $this->theString       = null;
    $this->payloadFormat   = null;
    $this->a               = null;
    $this->p_uuid          = null;
    $this->full_name       = null;
    $this->family_name     = null;
    $this->given_name      = null;
    $this->alternate_names = null;
    $this->profile_urls    = null;
    $this->incident_id     = null;
    $this->expiry_date     = null;
    $this->animal          = null;
    $this->opt_status      = null;
    $this->last_updated    = null;
    $this->creation_time   = null;
    $this->latitude        = null;
    $this->longitude       = null;
    $this->home_address    = null;
    $this->home_zip        = null;
    $this->birth_date      = null;
    $this->opt_race        = null;
    $this->opt_religion    = null;
    $this->opt_gender      = null;
    $this->years_old       = null;
    $this->minAge          = null;
    $this->maxAge          = null;
    $this->last_seen       = null;
    $this->last_clothing   = null;
    $this->other_comments  = null;
    $this->reporting_user  = 1;
    $this->reporter_user   = null;
    $this->is_editable     = 0;
    $this->Op_uuid         = null;
    $this->Ofull_name      = null;
    $this->Ofamily_name    = null;
    $this->Ogiven_name     = null;
    $this->Oalternate_names= null;
    $this->Oprofile_urls   = null;
    $this->Oincident_id    = null;
    $this->Oexpiry_date    = null;
    $this->Oanimal         = null;
    $this->Oopt_status     = null;
    $this->Olast_updated   = null;
    $this->Ocreation_time  = null;
    $this->Olatitude       = null;
    $this->Olongitude      = null;
    $this->Ohome_address   = null;
    $this->Ohome_zip       = null;
    $this->Obirth_date     = null;
    $this->Oopt_race       = null;
    $this->Oopt_religion   = null;
    $this->Oopt_gender     = null;
    $this->Oyears_old      = null;
    $this->OminAge         = null;
    $this->OmaxAge         = null;
    $this->Olast_seen      = null;
    $this->Olast_clothing  = null;
    $this->Oother_comments = null;
    $this->Oreporting_user = 1;
    $this->images          = array();
    $this->person_notes    = array();
    $this->sql_p_uuid         = null;
    $this->sql_full_name      = null;
    $this->sql_family_name    = null;
    $this->sql_given_name     = null;
    $this->sql_alternate_names= null;
    $this->sql_profile_urls   = null;
    $this->sql_incident_id    = null;
    $this->sql_expiry_date    = null;
    $this->sql_animal         = null;
    $this->sql_opt_status     = null;
    $this->sql_last_updated   = null;
    $this->sql_creation_time  = null;
    $this->sql_latitude       = null;
    $this->sql_longitude      = null;
    $this->sql_home_address   = null;
    $this->sql_home_zip       = null;
    $this->sql_birth_date     = null;
    $this->sql_opt_race       = null;
    $this->sql_opt_religion   = null;
    $this->sql_opt_gender     = null;
    $this->sql_years_old      = null;
    $this->sql_minAge         = null;
    $this->sql_maxAge         = null;
    $this->sql_last_seen      = null;
    $this->sql_last_clothing  = null;
    $this->sql_other_comments = null;
    $this->sql_reporting_user    = 1;
    $this->sql_Op_uuid           = null;
    $this->sql_Ofull_name        = null;
    $this->sql_Ofamily_name      = null;
    $this->sql_Ogiven_name       = null;
    $this->sql_Oalternate_names  = null;
    $this->sql_Oprofile_urls     = null;
    $this->sql_Oincident_id      = null;
    $this->sql_Oexpiry_date      = null;
    $this->sql_Oanimal           = null;
    $this->sql_Oopt_status       = null;
    $this->sql_Olast_updated     = null;
    $this->sql_Ocreation_time    = null;
    $this->sql_Olatitude         = null;
    $this->sql_Olongitude        = null;
    $this->sql_Ohome_address     = null;
    $this->sql_Ohome_zip         = null;
    $this->sql_Obirth_date       = null;
    $this->sql_Oopt_race         = null;
    $this->sql_Oopt_religion     = null;
    $this->sql_Oopt_gender       = null;
    $this->sql_Oyears_old        = null;
    $this->sql_OminAge           = null;
    $this->sql_OmaxAge           = null;
    $this->sql_Olast_seen        = null;
    $this->sql_Olast_clothing    = null;
    $this->sql_Oother_comments   = null;
    $this->sql_Oreporting_user   = 1;
    $this->author_name           = null;
    $this->author_email          = null;
    $this->makePfifNote          = true;
    $this->useNullLastUpdatedDb  = false;
    $this->ignoreDupeUuid        = false;
    $this->ecode                 = 0;
    $this->update_uid            = null;
    $this->saved                 = false;
    $this->modified              = false;
    $this->arrival_reunite       = false;
    $this->arrival_website       = false;
    $this->arrival_pfif          = false;
    $this->arrival_vanilla_email = false;
    $this->following = false;
  }
  
  // destructor
  public function __destruct() {}
  
  // initialize
  public function init() {
    $this->saved = false;
  }
  
  // loads the data from a person in the database
  public function load() {
    global $global, $conf;
    $q = "
      SELECT *
      FROM person_uuid
      WHERE p_uuid = ".$this->db->qstr((string)$this->p_uuid).";
    ";
    $result = $this->db->Execute($q);
    if($result === false) { daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $this->db->ErrorMsg(), "person load person_uuid ((".$q."))"); }
    if($result != NULL && !$result->EOF) {
      $this->full_name       = $result->fields['full_name'];
      $this->family_name     = $result->fields['family_name'];
      $this->given_name      = $result->fields['given_name'];
      $this->alternate_names = $result->fields['alternate_names'];
      $this->profile_urls    = $result->fields['profile_urls'];
      $this->incident_id     = $result->fields['incident_id'];
      $this->expiry_date     = $result->fields['expiry_date'];
      $this->animal          = $result->fields['animal'];
      $this->reporting_user  = $result->fields['reporting_user'];
      // save original values
      $this->Ofull_name       = $result->fields['full_name'];
      $this->Ofamily_name     = $result->fields['family_name'];
      $this->Ogiven_name      = $result->fields['given_name'];
      $this->Oalternate_names = $result->fields['alternate_names'];
      $this->Oprofile_urls    = $result->fields['profile_urls'];
      $this->Oincident_id     = $result->fields['incident_id'];
      $this->Oexpiry_date     = $result->fields['expiry_date'];
      $this->Oanimal          = $result->fields['animal'];
      $this->Oreporting_user  = $result->fields['reporting_user'];
    // load error
    } else { $this->ecode = 9900; }
    // load status
    $q = "
      SELECT *
      FROM person_status
      WHERE p_uuid = ".$this->db->qstr((string)$this->p_uuid).";
    ";
    $result = $this->db->Execute($q);
    if($result === false) { daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $this->db->ErrorMsg(), "person load person_status ((".$q."))"); }
    if($result != NULL && !$result->EOF) {
      $this->opt_status    = $result->fields['opt_status'];
      $this->last_updated  = $result->fields['last_updated'];
      $this->creation_time = $result->fields['creation_time'];
      $this->latitude      = $result->fields['latitude'];
      $this->longitude     = $result->fields['longitude'];
      // save original values
      $this->Oopt_status    = $result->fields['opt_status'];
      $this->Olast_updated  = $result->fields['last_updated'];
      $this->Ocreation_time = $result->fields['creation_time'];
      $this->Olatitude      = $result->fields['latitude'];
      $this->Olongitude     = $result->fields['longitude'];
    // load error
    } else { $this->ecode = 9901; }
    // load details
    $q = "
      SELECT *
      FROM person_details
      WHERE p_uuid = ".$this->db->qstr((string)$this->p_uuid).";
    ";
    $result = $this->db->Execute($q);
    if($result === false) { daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $this->db->ErrorMsg(), "person load person_details ((".$q."))"); }
    if($result != NULL && !$result->EOF) {
      $this->birth_date     = $result->fields['birth_date'];
      $this->opt_race       = $result->fields['opt_race'];
      $this->opt_religion   = $result->fields['opt_religion'];
      $this->opt_gender     = $result->fields['opt_gender'];
      $this->years_old      = $result->fields['years_old'];
      $this->minAge         = $result->fields['minAge'];
      $this->maxAge         = $result->fields['maxAge'];
      $this->last_seen      = $result->fields['last_seen'];
      $this->last_clothing  = $result->fields['last_clothing'];
      $this->other_comments = $result->fields['other_comments'];
      // save original values
      $this->Obirth_date     = $result->fields['birth_date'];
      $this->Oopt_race       = $result->fields['opt_race'];
      $this->Oopt_religion   = $result->fields['opt_religion'];
      $this->Oopt_gender     = $result->fields['opt_gender'];
      $this->Oyears_old      = $result->fields['years_old'];
      $this->OminAge         = $result->fields['minAge'];
      $this->OmaxAge         = $result->fields['maxAge'];
      $this->Olast_seen      = $result->fields['last_seen'];
      $this->Olast_clothing  = $result->fields['last_clothing'];
      $this->Oother_comments = $result->fields['other_comments'];
    // load error
    } else { $this->ecode = 9902; }
    // object exists in the db
    if($this->ecode != 9999) { $this->saved = true; }
    // load other data
    $this->loadImages();
    $this->loadNotes();
  }
  
  // load additional record data
  public function loadExtraData($uid, $gid) {
    global $global;
    // get reporter user and name
    $q = "
      SELECT *
      FROM `users`
      WHERE `uid` = ".$this->db->qstr((string)$uid)."
      LIMIT 1;
    ";
    $result = $this->db->Execute($q);
    if($result === false) { daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $this->db->ErrorMsg(), "person loadExtras 1 ((".$q."))"); }
    // if admin they have edit rights and can see reporter
    if($gid == 1) {
      $this->is_editable = 1;
      $this->reporter_user = $result->fields['user'];
    // reporting user can also edit
    } elseif(((int)$this->reporting_user == (int)$uid) && ((int)$this->reporting_user !== (int)3)) { $this->is_editable = 1; }
    // get following status
    $q = "
      SELECT COUNT(*) 
      FROM `following`
      WHERE `uid` = ".$this->db->qstr((string)$uid)."
      AND `p_uuid` = ".$this->db->qstr((string)$this->p_uuid)."
      LIMIT 1;
    ";
    $result = $this->db->Execute($q);
    if($result === false) { daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $this->db->ErrorMsg(), "person loadExtras 3 ((".$q."))"); }
    if((int)$result->fields['COUNT(*)'] > 0) { $this->following = true; }
    else { $this->following = false; }
    $this->loadNotesExtraData($gid);
  }
  
  // load all images
  private function loadImages() {
    $q = "
      SELECT *
      FROM image
      WHERE p_uuid = ".$this->db->qstr((string)$this->p_uuid).";
    ";
    $result = $this->db->Execute($q);
    if($result === false) { daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $this->db->ErrorMsg(), "loadImages 1"); }
    while(!$result == NULL && !$result->EOF) {
      $i = new image();
      $i->p_uuid = $this->p_uuid;
      $i->update_uid = $this->update_uid;
      $i->image_id = $result->fields['image_id'];
      $i->load();
      $i->incident_id = $this->incident_id;
      $this->images[] = $i;
      $result->MoveNext();
    }
  }
  
  // load all notes
  private function loadNotes() {
    $q = "
      SELECT *
      FROM `person_notes`
      WHERE `p_uuid` = ".$this->db->qstr((string)$this->p_uuid).";
    ";
    $result = $this->db->Execute($q);
    if($result === false) { daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $this->db->ErrorMsg(), "loadNotes 1"); }
    while(!$result == NULL && !$result->EOF) {
      $pn = new note();
      $pn->p_uuid   = $this->p_uuid;
      $pn->note_uid = $this->update_uid;
      $pn->note_id  = $result->fields['note_id'];
      $pn->load();
      $this->person_notes[] = $pn;
      $result->MoveNext();
    }
  }
  
  // load all note extra data
  private function loadNotesExtraData($gid) {
    foreach($this->person_notes as $person_note) {
      $person_note->loadExtraData($gid);
    }
  }
  
  // save the person // initial save
  public function insert($uid = null, $gid = null) {
    global $conf;
    // if this object is in the db uptate not insert
    if($this->saved) { $this->update(); }
    else { 
      $this->sync();
      // insert person
      $q = "
        INSERT INTO person_uuid (
          p_uuid,
          full_name,
          family_name,
          given_name,
          alternate_names,
          profile_urls,
          incident_id,
          expiry_date,
          animal,
          reporting_user)
        VALUES (
          ".$this->sql_p_uuid.",
          ".$this->sql_full_name.",
          ".$this->sql_family_name.",
          ".$this->sql_given_name.",
          ".$this->sql_alternate_names.",
          ".$this->sql_profile_urls.",
          ".$this->sql_incident_id.",
          ".$this->sql_expiry_date.",
          ".$this->sql_animal.",
          ".$this->sql_reporting_user."
        );
      ";
      $result = $this->db->Execute($q);
      if($result === false) { daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $this->db->ErrorMsg(), "person person_uuid insert ((".$q."))"); }
      // last updated db hack
      if($this->useNullLastUpdatedDb) { $ludb = "NULL"; }
      else { $ludb = "'".date('Y-m-d H:i:s')."'"; }
      // insert details
      $q = "
        INSERT INTO person_details (
          p_uuid,
          birth_date,
          opt_race,
          opt_religion,
          opt_gender,
          years_old,
          minAge,
          maxAge,
          last_seen,
          last_clothing,
          other_comments )
        VALUES (
          ".$this->sql_p_uuid.",
          ".$this->sql_birth_date.",
          ".$this->sql_opt_race.",
          ".$this->sql_opt_religion.",
          ".$this->sql_opt_gender.",
          ".$this->sql_years_old.",
          ".$this->sql_minAge.",
          ".$this->sql_maxAge.",
          ".$this->sql_last_seen.",
          ".$this->sql_last_clothing.",
          ".$this->sql_other_comments."
        );
      ";
      $result = $this->db->Execute($q);
      if($result === false) { daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $this->db->ErrorMsg(), "person person_details insert ((".$q."))"); }
      // insert status
      $q = "
        INSERT INTO person_status (
          p_uuid,
          opt_status,
          last_updated,
          creation_time,
          last_updated_db,
          latitude,
          longitude
        )
        VALUES (
          ".$this->sql_p_uuid.",
          ".$this->sql_opt_status.",
          ".$this->sql_last_updated.",
          ".$this->sql_creation_time.",
          ".$ludb.",
          ".$this->sql_latitude.",
          ".$this->sql_longitude."
        );
      ";
      $result = $this->db->Execute($q);
      if($result === false) { daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $this->db->ErrorMsg(), "person person_status insert ((".$q."))"); }
      // insert home address
      $q = "
        INSERT INTO contact (
          p_uuid,
          opt_contact_type,
          contact_value)
        VALUES (
          ".$this->sql_p_uuid.",
          'home',
          ".$this->sql_home_address."
        );
      ";
      $result = $this->db->Execute($q);
      if($result === false) { daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $this->db->ErrorMsg(), "contact insert ((".$q."))"); }
      // insert home zip
      $q = "
        INSERT INTO contact (
          p_uuid,
          opt_contact_type,
          contact_value)
        VALUES (
          ".$this->sql_p_uuid.",
          'zip',
          ".$this->sql_home_zip."
        );
      ";
      $result = $this->db->Execute($q);
      if($result === false) { daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $this->db->ErrorMsg(), "contact insert ((".$q."))"); }
      // insert others
      $this->insertImages();
      $this->makeStaticPfifNote();
      $this->insertNotes();
      // set final last_updated_db
      $ludb = "'".date('Y-m-d H:i:s')."'";
      $q =  "UPDATE person_status SET last_updated_db=$ludb WHERE p_uuid = ".$this->sql_p_uuid."; ";
      $result = $this->db->Execute($q);
      if($result === false) { daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $this->db->ErrorMsg(), "person marked with last_updated_db ((".$q."))"); }
      // flag this record as complete.
      $q = "UPDATE person_uuid SET complete=1 WHERE p_uuid = ".$this->sql_p_uuid."; ";
      $result = $this->db->Execute($q);
      if($result === false) { daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $this->db->ErrorMsg(), "person marked as complete ((".$q."))"); }
      $this->saved = true;
      $this->modified = false;
      // keep arrival rate stats
      updateArrivalRate($this->p_uuid, $this->incident_id, 0, $this->arrival_reunite, $this->arrival_website, $this->arrival_pfif, $this->arrival_vanilla_email);
      // follow record if not anonymously reported
      if($gid != 3 && $gid != null) { $this->followRecord($uid); }
      // perform solr update
      if($conf['SOLR_on'] == true) { taupo_solr_add_person($this->p_uuid); }
    }
  }
  
  // perform follow action
  public function followRecord($uid) {
    $q = "
      INSERT INTO following ( uid, p_uuid )
      VALUES (".$uid.", ".$this->sql_p_uuid.");
    ";
    $result = $this->db->Execute($q);
    if($result === false) { daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $this->db->ErrorMsg(), "person person_uuid insert ((".$q."))"); } 
  }
  
  // save images
  private function insertImages() {
    foreach($this->images as $image) {
      $image->incident_id = $this->incident_id;
      $image->insert($this->years_old, $this->opt_gender, $this->minAge, $this->maxAge, $this->animal);
    }
  }
  
  // save notes
  private function insertNotes() {
    foreach($this->person_notes as $person_note) {
      $person_note->insert();
    }
  }
  
  // save pfif
  public function makeStaticPfifNote() {
    // make the note unless we are explicitly asked not to
    if(!$this->makePfifNote) { return; }
    global $global, $conf;
    require_once($conf['approot']."mod/pfif/pfif.inc");
    require_once($conf['approot']."mod/pfif/util.inc");
    $p = new Pfif();
    $n = new Pfif_Note();
    $n->note_record_id          = taupo_create_uuid('pfif_note');
    $n->person_record_id        = $this->p_uuid;
    $n->linked_person_record_id = null;
    $n->source_date             = $this->last_updated; // since we are now creating the note
    $n->entry_date              = $this->last_updated; // we use the last_updated for both values
    $n->author_phone            = null;
    $n->email_of_found_person   = null;
    $n->phone_of_found_person   = null;
    $n->last_known_location     = $this->last_seen;
    $n->text                    = $this->other_comments;
    $n->found                   = null; // we have no way to know if the reporter had direct contact // hence leave this null
    // figure out the pfif status
    $n->status = taupo_map_status_to_pfif($this->opt_status);
    // find author email
    $q = "
      SELECT *
      FROM `users`
      WHERE `uid` = '".$this->reporting_user."';
    ";
    $result = $this->db->Execute($q);
    if($result === false) { daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $this->db->ErrorMsg(), "person pfif note ((".$q."))"); }
    if($result != NULL && !$result->EOF) {
      $n->author_name  = '';
      $n->author_email = $result->fields['user'];
    } else {
      $n->author_name  = null;
      $n->author_email = null;
    }
    $p->setNote($n);
    $p->setIncidentId($this->incident_id);
    $p->storeNotesInDatabase();
  }
  
  // delete function
  public function delete() {
    global $conf;
    $this->sync();
    $this->deleteImages();
    $this->deleteNotes();
    $q = "
      DELETE FROM person_uuid
      WHERE  p_uuid = ".$this->sql_p_uuid.";
    ";
    $result = $this->db->Execute($q);
    if($result === false) { daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $this->db->ErrorMsg(), "person delete person 1 ((".$q."))"); }
    $this->saved = false;
    $this->deletePfif();
    if($conf['SOLR_on'] == true) {
      taupo_solr_delete_person($this->p_uuid);
    }
  }
  
  // delete pfif record
  private function deletePfif() {
    $q = "
      DELETE FROM pfif_person
      WHERE  p_uuid = ".$this->sql_p_uuid.";
    ";
    $result = $this->db->Execute($q);
    if($result === false) { daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $this->db->ErrorMsg(), "person delete pfif 1 ((".$q."))"); }
    $q = "
      DELETE FROM pfif_note
      WHERE  p_uuid = ".$this->sql_p_uuid.";
    ";
    $result = $this->db->Execute($q);
    if($result === false) { daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $this->db->ErrorMsg(), "person delete pfif 2 ((".$q."))"); }
    $q = "
      UPDATE pfif_note
      SET linked_person_record_id = NULL
      WHERE linked_person_record_id = ".$this->sql_p_uuid.";
    ";
    $result = $this->db->Execute($q);
    if($result === false) { daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $this->db->ErrorMsg(), "person delete pfif 3 ((".$q."))"); }
  }
  
  // delete images
  private function deleteImages() {
    foreach($this->images as $image) {
      $image->delete();
    }
  }
  
  // delete notes
  private function deleteNotes() {
    foreach($this->person_notes as $person_note) { $person_note->delete(); }
  }
  
  // save the person // subsequent save
  public function update($skipStatusUpdate = false) {
    global $conf;
    // first save // cannot update // insert instead
    if(!$this->saved) { $this->insert(); }
    else {
      $this->sync();
      $this->saveRevisions();
      // update person
      $q = "
        UPDATE person_uuid
        SET
          full_name       = ".$this->sql_full_name.",
          family_name     = ".$this->sql_family_name.",
          given_name      = ".$this->sql_given_name.",
          alternate_names = ".$this->sql_alternate_names.",
          profile_urls    = ".$this->sql_profile_urls.",
          incident_id     = ".$this->sql_incident_id.",
          expiry_date     = ".$this->sql_expiry_date.",
          animal          = ".$this->sql_animal.",
          reporting_user  = ".$this->sql_reporting_user."
        WHERE p_uuid      = ".$this->sql_p_uuid.";
      ";
      $result = $this->db->Execute($q);
      if($result === false) { daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $this->db->ErrorMsg(), "person person_uuid update ((".$q."))"); }
      // update details
      $q = "
        UPDATE person_details
        SET
          birth_date     = ".$this->sql_birth_date.",
          opt_race       = ".$this->sql_opt_race.",
          opt_religion   = ".$this->sql_opt_religion.",
          opt_gender     = ".$this->sql_opt_gender.",
          years_old      = ".$this->sql_years_old.",
          minAge         = ".$this->sql_minAge.",
          maxAge         = ".$this->sql_maxAge.",
          last_seen      = ".$this->sql_last_seen.",
          last_clothing  = ".$this->sql_last_clothing.",
          other_comments = ".$this->sql_other_comments."
        WHERE p_uuid     = ".$this->sql_p_uuid.";
      ";
      $result = $this->db->Execute($q);
      if($result === false) { daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $this->db->ErrorMsg(), "person person_details update ((".$q."))"); }
      // update others
      $this->updateNotes();
      $this->updateImages();
      // all changes saved // this object is unmodified
      $this->modified = false;
      // always update the last_updated to current time when saving // unless this is a new PFIF note in which case the person_status was already updated
      if($skipStatusUpdate == false) {
        $q = "
          UPDATE person_status
          SET
            opt_status      = ".$this->sql_opt_status.",
            last_updated    = '".date('Y-m-d H:i:s')."',
            creation_time   = ".$this->sql_creation_time.",
            last_updated_db = '".date('Y-m-d H:i:s')."',
            latitude        = ".$this->sql_latitude.",
            longitude       = ".$this->sql_longitude."
          WHERE p_uuid = ".$this->sql_p_uuid.";
        ";
        $result = $this->db->Execute($q);
        if($result === false) { daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $this->db->ErrorMsg(), "person person_status update ((".$q."))"); }
        // update home address
        $q = "
          UPDATE contact
          SET
            contact_value = ".$this->sql_home_address."
            WHERE opt_contact_type = 'home' AND p_uuid = ".$this->sql_p_uuid.";
        ";
        $result = $this->db->Execute($q);
        if($result === false) { daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $this->db->ErrorMsg(), "person contact update ((".$q."))"); }
        // update home zip
        $q = "
          UPDATE contact
          SET
            contact_value = ".$this->sql_home_zip."
            WHERE opt_contact_type = 'zip' AND p_uuid = ".$this->sql_p_uuid.";
        ";
        $result = $this->db->Execute($q);
        if($result === false) { daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $this->db->ErrorMsg(), "person contact update ((".$q."))"); }
      }
    }
    // perform solr update
    if($conf['SOLR_on'] == true) { taupo_solr_add_person($this->p_uuid); }
  }
  
  // set principal to 0 for all current images // records only have one image but we keep all images previous to revisions // PL-1853
  private function demoteAllImages() {
    foreach($this->images as $image) {
      if($image !== null) {
        $image->principal = 0;
      }
    }
  }

  // update images
  private function updateImages() {
    foreach($this->images as $image) {
      if($image !== null) {
        $image->update_uid = $this->update_uid;
        $image->update($this->years_old, $this->opt_gender, $this->minAge, $this->maxAge, $this->animal);
      }
    }
  }
  
  // update notes
  private function updateNotes() {
    foreach($this->person_notes as $person_note) {
      if($person_note !== null) {
        $person_note->update();
      }
    }
  }
  
  // save any changes since object was loaded as revisions
  function saveRevisions() {
    global $global;
    global $revisionCount;
    // save person revisions
    if($this->full_name       != $this->Ofull_name)       { $this->saveRevision($this->sql_full_name,       $this->sql_Ofull_name,       'person_uuid',      'full_name'     ); }
    if($this->family_name     != $this->Ofamily_name)     { $this->saveRevision($this->sql_family_name,     $this->sql_Ofamily_name,     'person_uuid',      'family_name'   ); }
    if($this->given_name      != $this->Ogiven_name)      { $this->saveRevision($this->sql_given_name,      $this->sql_Ogiven_name,      'person_uuid',      'given_name'    ); }
    if($this->alternate_names != $this->Oalternate_names) { $this->saveRevision($this->sql_alternate_names, $this->sql_Oalternate_names, 'person_uuid',      'alternate_names');}
    if($this->profile_urls    != $this->Oprofile_urls)    { $this->saveRevision($this->sql_profile_urls,    $this->sql_Oprofile_urls,    'person_uuid',      'profile_urls'  ); }
    if($this->incident_id     != $this->Oincident_id)     { $this->saveRevision($this->sql_incident_id,     $this->sql_Oincident_id,     'person_uuid',      'incident_id'   ); }
    if($this->expiry_date     != $this->Oexpiry_date)     { $this->saveRevision($this->sql_expiry_date,     $this->sql_Oexpiry_date,     'person_uuid',      'expiry_date'   ); }
    if($this->animal          != $this->Oanimal)          { $this->saveRevision($this->sql_animal,          $this->sql_Oanimal,          'person_uuid',      'animal'        ); }
    if($this->reporting_user  != $this->Oreporting_user)  { $this->saveRevision($this->sql_reporting_user,  $this->sql_Oreporting_user,  'person_uuid',      'reporting_user'); }
    // update person status
    if($this->opt_status      != $this->Oopt_status)      { $this->saveRevision($this->sql_opt_status,      $this->sql_Oopt_status,      'person_status',    'opt_status'    ); }
    if($this->creation_time   != $this->Ocreation_time)   { $this->saveRevision($this->sql_creation_time,   $this->sql_Ocreation_time,   'person_status',    'creation_time' ); }
    if($this->latitude        != $this->Olatitude)        { $this->saveRevision($this->sql_latitude,        $this->sql_Olatitude,        'person_status',    'latitude'      ); }
    if($this->longitude       != $this->Olongitude)       { $this->saveRevision($this->sql_longitude,       $this->sql_Olongitude,       'person_status',    'longitude'     ); }
    // update person home_address and home_zip
    if($this->home_address    != $this->Ohome_address)    { $this->saveRevision($this->sql_home_address,    $this->sql_Ohome_address,    'contact',          'home'  ); }
    if($this->home_zip        != $this->Ohome_zip)        { $this->saveRevision($this->sql_home_zip,        $this->sql_Ohome_zip,        'contact',          'zip'      ); }
    // update person details
    if($this->birth_date      != $this->Obirth_date)      { $this->saveRevision($this->sql_birth_date,      $this->sql_Obirth_date,      'person_details',   'birth_date'    ); }
    if($this->opt_race        != $this->Oopt_race)        { $this->saveRevision($this->sql_opt_race,        $this->sql_Oopt_race,        'person_details',   'opt_race'      ); }
    if($this->opt_religion    != $this->Oopt_religion)    { $this->saveRevision($this->sql_opt_religion,    $this->sql_Oopt_religion,    'person_details',   'opt_religion'  ); }
    if($this->opt_gender      != $this->Oopt_gender)      { $this->saveRevision($this->sql_opt_gender,      $this->sql_Oopt_gender,      'person_details',   'opt_gender'    ); }
    if($this->years_old       != $this->Oyears_old)       { $this->saveRevision($this->sql_years_old,       $this->sql_Oyears_old,       'person_details',   'years_old'     ); }
    if($this->minAge          != $this->OminAge)          { $this->saveRevision($this->sql_minAge,          $this->sql_OminAge,          'person_details',   'minAge'        ); }
    if($this->maxAge          != $this->OmaxAge)          { $this->saveRevision($this->sql_maxAge,          $this->sql_OmaxAge,          'person_details',   'maxAge'        ); }
    if($this->last_seen       != $this->Olast_seen)       { $this->saveRevision($this->sql_last_seen,       $this->sql_Olast_seen,       'person_details',   'last_seen'     ); $this->makeStaticPfifNote(); }
    if($this->last_clothing   != $this->Olast_clothing)   { $this->saveRevision($this->sql_last_clothing,   $this->sql_Olast_clothing,   'person_details',   'last_clothing' ); }
    if($this->other_comments  != $this->Oother_comments)  { $this->saveRevision($this->sql_other_comments,  $this->sql_Oother_comments,  'person_details',   'other_comments'); }
  }
  
  // save revision
  function saveRevision($newValue, $oldValue, $table, $column) {
    $this->modified = true;
    $q = "
      INSERT into person_updates (`p_uuid`, `updated_table`, `updated_column`, `old_value`, `new_value`, `uid`)
      VALUES (".$this->sql_p_uuid.", '".$table."', '".$column."', ".$oldValue.", ".$newValue.", ".$this->update_uid.");
    ";
    $result = $this->db->Execute($q);
    if($result === false) { daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $this->db->ErrorMsg(), "person saveRevision ((".$q."))"); }
  }
  
  // synchronize SQL value strings with class attributes
  private function sync() {
    global $global;
    // validate window.r.expiry_date
    if(date('Y-m-d H:i:s', strtotime($this->expiry_date)) == $this->expiry_date) {}
    else { $this->expiry_date = null; }
    // set reporter to anonymous user if null
    if($this->reporting_user === null) { $this->reporting_user = 3; }
    // build SQL strings
    $this->sql_p_uuid         = ($this->p_uuid         === null) ? "NULL" : $global['db']->qstr((string)$this->p_uuid);
    $this->sql_full_name      = ($this->full_name      === null) ? "NULL" : $global['db']->qstr((string)$this->full_name);
    $this->sql_family_name    = ($this->family_name    === null) ? "NULL" : $global['db']->qstr((string)$this->family_name);
    $this->sql_given_name     = ($this->given_name     === null) ? "NULL" : $global['db']->qstr((string)$this->given_name);
    $this->sql_alternate_names= ($this->alternate_names=== null) ? "NULL" : $global['db']->qstr((string)$this->alternate_names);
    $this->sql_profile_urls   = ($this->profile_urls   === null) ? "NULL" : $global['db']->qstr((string)$this->profile_urls);
    $this->sql_incident_id    = ($this->incident_id    === null) ? "NULL" : (int)$this->incident_id;
    $this->sql_expiry_date    = ($this->expiry_date    === null) ? "NULL" : $global['db']->qstr((string)$this->expiry_date);
    $this->sql_animal         = ($this->animal         === null) ? "NULL" : $global['db']->qstr((string)$this->animal);
    $this->sql_reporting_user = ($this->reporting_user === null) ? "NULL" : $global['db']->qstr((string)$this->reporting_user);
    $this->sql_opt_status     = ($this->opt_status     === null) ? "NULL" : $global['db']->qstr((string)$this->opt_status);
    $this->sql_last_updated   = ($this->last_updated   === null) ? "NULL" : $global['db']->qstr((string)$this->last_updated);
    $this->sql_creation_time  = ($this->creation_time  === null) ? "NULL" : $global['db']->qstr((string)$this->creation_time);
    $this->sql_latitude       = ($this->latitude       === null) ? "NULL" : $global['db']->qstr((string)$this->latitude);
    $this->sql_longitude      = ($this->longitude      === null) ? "NULL" : $global['db']->qstr((string)$this->longitude);
    $this->sql_home_address   = ($this->home_address   === null) ? "NULL" : $global['db']->qstr((string)$this->home_address);
    $this->sql_home_zip       = ($this->home_zip       === null) ? "NULL" : $global['db']->qstr((string)$this->home_zip);
    $this->sql_birth_date     = ($this->birth_date     === null) ? "NULL" : $global['db']->qstr((string)$this->birth_date);
    $this->sql_opt_race       = ($this->opt_race       === null) ? "NULL" : $global['db']->qstr((string)$this->opt_race);
    $this->sql_opt_religion   = ($this->opt_religion   === null) ? "NULL" : $global['db']->qstr((string)$this->opt_religion);
    $this->sql_opt_gender     = ($this->opt_gender     === null) ? "NULL" : $global['db']->qstr((string)$this->opt_gender);
    $this->sql_years_old      = ($this->years_old      === null) ? "NULL" : (int)$this->years_old;
    $this->sql_minAge         = ($this->minAge         === null) ? "NULL" : (int)$this->minAge;
    $this->sql_maxAge         = ($this->maxAge         === null) ? "NULL" : (int)$this->maxAge;
    $this->sql_last_seen      = ($this->last_seen      === null) ? "NULL" : $global['db']->qstr((string)$this->last_seen);
    $this->sql_last_clothing  = ($this->last_clothing  === null) ? "NULL" : $global['db']->qstr((string)$this->last_clothing);
    $this->sql_other_comments = ($this->other_comments === null) ? "NULL" : $global['db']->qstr((string)$this->other_comments);
    // do the same for original values...
    $this->sql_Op_uuid         = ($this->Op_uuid         === null) ? "NULL" : $global['db']->qstr((string)$this->Op_uuid);
    $this->sql_Ofull_name      = ($this->Ofull_name      === null) ? "NULL" : $global['db']->qstr((string)$this->Ofull_name);
    $this->sql_Ofamily_name    = ($this->Ofamily_name    === null) ? "NULL" : $global['db']->qstr((string)$this->Ofamily_name);
    $this->sql_Ogiven_name     = ($this->Ogiven_name     === null) ? "NULL" : $global['db']->qstr((string)$this->Ogiven_name);
    $this->sql_Oalternate_names= ($this->Oalternate_names=== null) ? "NULL" : $global['db']->qstr((string)$this->Oalternate_names);
    $this->sql_Oprofile_urls   = ($this->Oprofile_urls   === null) ? "NULL" : $global['db']->qstr((string)$this->Oprofile_urls);
    $this->sql_Oincident_id    = ($this->Oincident_id    === null) ? "NULL" : (int)$this->Oincident_id;
    $this->sql_Oexpiry_date    = ($this->Oexpiry_date    === null) ? "NULL" : $global['db']->qstr((string)$this->Oexpiry_date);
    $this->sql_Oanimal         = ($this->Oanimal         === null) ? "NULL" : $global['db']->qstr((string)$this->Oanimal);
    $this->sql_Oreporting_user = ($this->Oreporting_user === null) ? "NULL" : $global['db']->qstr((string)$this->Oreporting_user);
    $this->sql_Oopt_status     = ($this->Oopt_status     === null) ? "NULL" : $global['db']->qstr((string)$this->Oopt_status);
    $this->sql_Olast_updated   = ($this->Olast_updated   === null) ? "NULL" : $global['db']->qstr((string)$this->Olast_updated);
    $this->sql_Ocreation_time  = ($this->Ocreation_time  === null) ? "NULL" : $global['db']->qstr((string)$this->Ocreation_time);
    $this->sql_Olatitude       = ($this->Olatitude       === null) ? "NULL" : $global['db']->qstr((string)$this->Olatitude);
    $this->sql_Olongitude      = ($this->Olongitude      === null) ? "NULL" : $global['db']->qstr((string)$this->Olongitude);
    $this->sql_Ohome_address   = ($this->Ohome_address   === null) ? "NULL" : $global['db']->qstr((string)$this->Ohome_address);
    $this->sql_Ohome_zip       = ($this->Ohome_zip       === null) ? "NULL" : $global['db']->qstr((string)$this->Ohome_zip);
    $this->sql_Obirth_date     = ($this->Obirth_date     === null) ? "NULL" : $global['db']->qstr((string)$this->Obirth_date);
    $this->sql_Oopt_race       = ($this->Oopt_race       === null) ? "NULL" : $global['db']->qstr((string)$this->Oopt_race);
    $this->sql_Oopt_religion   = ($this->Oopt_religion   === null) ? "NULL" : $global['db']->qstr((string)$this->Oopt_religion);
    $this->sql_Oopt_gender     = ($this->Oopt_gender     === null) ? "NULL" : $global['db']->qstr((string)$this->Oopt_gender);
    $this->sql_Oyears_old      = ($this->Oyears_old      === null) ? "NULL" : (int)$this->Oyears_old;
    $this->sql_OminAge         = ($this->OminAge         === null) ? "NULL" : (int)$this->OminAge;
    $this->sql_OmaxAge         = ($this->OmaxAge         === null) ? "NULL" : (int)$this->OmaxAge;
    $this->sql_Olast_seen      = ($this->Olast_seen      === null) ? "NULL" : $global['db']->qstr((string)$this->Olast_seen);
    $this->sql_Olast_clothing  = ($this->Olast_clothing  === null) ? "NULL" : $global['db']->qstr((string)$this->Olast_clothing);
    $this->sql_Oother_comments = ($this->Oother_comments === null) ? "NULL" : $global['db']->qstr((string)$this->Oother_comments);
  }
  
  // expire a person
  function expire($uid = null, $explanation = null) {
    $this->sync();
    // we set the expiration time to now
    $this->expiry_date = date('Y-m-d H:i:s', time() - 1 * 60); // one minute ago
    // lastly, when a person is expired, we randomize the image (full and thumb) urls to make it impossible to find the image of an expired person
    foreach($this->images as $image) { $image->randomize(); }
    // save all changes
    $this->update();
  }
  
  // find if this event is open or closed
  public function isEventOpen() {
    $q = "
      SELECT *
      FROM incident
      WHERE incident_id = '".$this->incident_id."';
    ";
    $result = $this->db->Execute($q);
    if($result === false) { daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $this->db->ErrorMsg(), "person check event open ((".$q."))"); }
    if($result != NULL && !$result->EOF) { $row = $result->FetchRow(); }
    else { return false; }
    if($row['closed'] != 0) { return false; }
    else { return true; }
  }
  
  // check revision permissions for this record
  public function canRevise($uid, $gid) {
    if((int)$gid === 1) { return true; }
    if((int)$gid === 3) { return false; }
    if((int)$this->reporting_user === (int)$uid) { return true; }
    else { return false; }
  }
  
  // is this uid the record owner
  public function isOwner($uid) {
    if((int)$this->reporting_user === (int)$uid) { return true; }
    else { return false; }
  }
  
  // find the user of the user to report this person
  public function getOwner() {
    $q = "
      SELECT *
      FROM `users`
      WHERE uid = '".$this->reporting_user."';
    ";
    $result = $this->db->Execute($q);
    if($result === false) { daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $this->db->ErrorMsg(), "person getOwner ((".$q."))"); }
    if($result != NULL && !$result->EOF) { $row = $result->FetchRow(); }
    else { return false; }
    return $row['user'];
  }
  
  // add image to record
  public function addImage($fileContentBase64, $filename) {
    if(trim($fileContentBase64) != "") {
      $i = new image();
      $i->init();
      $i->p_uuid = $this->p_uuid;
      $i->incident_id = $this->incident_id;
      $i->fileContentBase64 = $fileContentBase64;
      $i->original_filename = $filename;
      $this->images[] = $i;
    }
  }
  
  // add comment to record
  public function addComment($text, $stat=null, $loc=null, $photo=null, $uid=3, $pfif_note_id=null, $authorName=null, $entry_date=null) {
    $pn = new note();
    $pn->init();
    $pn->p_uuid       = $this->p_uuid;
    $pn->note         = $text;
    $pn->stat         = $stat;
    $pn->loc          = $loc;
    $pn->photo        = null;
    $pn->uid          = $uid;
    $pn->pfif_note_id = $pfif_note_id;
    $entry_date == null ? $pn->when = date('Y-m-d H:i:s', time()) : $pn->when = $entry_date;
    $pn->insert();
    /*
    if(($suggested_image != null) && (trim($suggested_image) != "")) {
      $i = new image();
      $i->init();
      $i->p_uuid = $this->p_uuid;
      $i->incident_id = $this->incident_id;
      $i->fileContentBase64 = $suggested_image;
      $i->original_filename = null;
      $i->note_id = $pn->note_id;
      $i->principal = 0;
      $i->insert($this->years_old, $this->opt_gender, $this->minAge, $this->maxAge, $this->animal);
    }
    */
  }
  
  // create a new uuid
  public function createUUID() {
    if($this->p_uuid === null || $this->p_uuid == "") { $this->p_uuid = taupo_create_uuid("record");  }
  }
  
  // set the event id
  public function setEvent($eventShortName) {
    $q = "
      SELECT *
      FROM `incident`
      WHERE shortname = ".$this->db->qstr($eventShortName).";
    ";
    $result = $this->db->Execute($q);
    if($result === false) { daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $this->db->ErrorMsg(), "person get incident ((".$q."))"); }
    $this->incident_id = $result->fields['incident_id'];
  }
  
  // make object for json
  public function makeArrayObject() {
    global $conf;
    $r = array();
    $r['uuid'] = (string)$this->p_uuid;
    $r['name'] = (string)strip_tags(preg_replace('/[\x00-\x1F\x80-\x9F]/u', '', $this->full_name));
    $r['stat'] = (string)$this->opt_status;
    $r['sex']  = (string)$this->opt_gender;
    $r['age']  = (int)$this->years_old;
    $r['lki']  = (string)strip_tags(preg_replace('/[\x00-\x1F\x80-\x9F]/u', '', $this->other_comments));
    $r['latitude']  = (float)$this->latitude;
    $r['longitude'] = (float)$this->longitude;
    $r['editable']  = (boolean)$this->is_editable;
    $r['following'] = (boolean)$this->following;
    $r['animal']    = $this->animal;
    $this->last_updated  == null ? $r['updated'] = null : $r['updated'] = (string)date('c', strtotime($this->last_updated));
    $this->creation_time == null ? $r['created'] = null : $r['created'] = (string)date('c', strtotime($this->creation_time));
    $this->expiry_date   == null ? $r['expires'] = null : $r['expires'] = (string)date('c', strtotime($this->expiry_date));
    if($this->reporter_user !== null) {
      $r['reporter_uid']   = (int)$this->reporting_user;
      $r['reporter_user']  = (string)preg_replace('/[\x00-\x1F\x80-\x9F]/u', '', $this->reporter_user);
      // add revision count
      $q = "
        SELECT count(*)
        FROM `person_updates`
        WHERE p_uuid = ".$this->db->qstr($this->p_uuid).";
      ";
      $result = $this->db->Execute($q);
      if($result === false) { daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $this->db->ErrorMsg(), "hasRecordBeenRevised ((".$q."))"); }
      $r['revisions'] = (int)$result->fields['count(*)'];
    }
    // add image // select first primary image // only one
    $count = 0;
    foreach($this->images as $image) {
      if($image->principal == 1) {
        $ri[] = $image->makeArrayObject();
        $count++;
        break;
      }
    }
    if($count == 0) {
      $r['image_url'] = null;
      $r['thumb_url'] = null;
    } else {
      $r['image_url'] = (string)$ri[0]['image_url'];
      $r['thumb_url'] = (string)$ri[0]['thumb_url'];
    }
    // add comments
    $pn = array();
    foreach($this->person_notes as $person_note) { $pn[] = $person_note->makeArrayObject(); }
    $r['comments'] = $pn;
    return $r;
  }
  
  // process the payload
  public function process($revise = false) {
    global $global, $conf;
    // PA1 format // default
    $pa        = isset($global['jsin']['pa'])        ? (string)$global['jsin']['pa']        : 0;
    $stat      = isset($global['jsin']['stat'])      ? (string)$global['jsin']['stat']      : 'mis';
    $sex       = isset($global['jsin']['sex'])       ? (string)$global['jsin']['sex']       : 'unk';
    $age       = isset($global['jsin']['age'])       ? (string)$global['jsin']['age']       : '18';
    $name      = isset($global['jsin']['name'])      ? (string)$global['jsin']['name']      : '';
    $photo     = isset($global['jsin']['photo'])     ? (string)$global['jsin']['photo']     : null;
    $latitude  = isset($global['jsin']['latitude'])  ? (double)$global['jsin']['latitude']  : 0;
    $longitude = isset($global['jsin']['longitude']) ? (double)$global['jsin']['longitude'] : 0;
    $lki       = isset($global['jsin']['lki'])       ? (string)$global['jsin']['lki']       : '';
    $buddy     = isset($global['jsin']['buddy'])     ? (string)$global['jsin']['buddy']     : '';
    // initial report
    if(!$revise) {
      $this->createUUID();
      $this->arrival_website = true;
      $creationDate = date('Y-m-d H:i:s', time()); // now
      $expiryDate   = date('Y-m-d H:i:s', time()+(60*60*24*365)); // now + 1 year
      $this->creation_time = $creationDate;
      $this->expiry_date = $expiryDate;
    }
    // validate pa
    if((int)$pa !== 0 && (int)$pa !== 1) { return (int)1005; }
    // validate status
    if($stat !== 'mis' && $stat !== 'fnd' && $stat !== 'ali' && $stat !== 'inj' && $stat !== 'dec' && $stat !== 'unk') { return (int)1002; }
    // validate gender
    if($sex !== 'mal' && $sex !== 'fml' && $sex !== 'cpx' && $sex !== 'unk') { return (int)1003; }
    // validate age
    if($age == 'unk' || (int)$age < MINIMUM_AGE || (int)$age > MAXIMUM_AGE) { $age = '-1'; }
    // validate lat/lon
    if($latitude < -90 || $latitude > 90 || $longitude < -180 || $longitude > 180) { return (int)1004; }
    // assign object values
    $this->full_name      = $name;
    $this->opt_status     = $stat;
    $this->last_updated   = date('Y-m-d H:i:s', (time()+1)); // add 1s to separate from creation time
    $this->opt_gender     = $sex;
    $this->years_old      = $age;
    $this->other_comments = $lki;
    $this->latitude       = $latitude;
    $this->longitude      = $longitude;
    if($photo != null) {
      // demote all previous images so this new image is the primary // PL-1853
      if($revise) { $this->demoteAllImages(); }
      // start new image
      $i = new image();
      $i->p_uuid = $this->p_uuid;
      $i->init();
      $i->incident_id = $this->incident_id;
      $i->fileContentBase64 = $photo;
      $i->decode();
      $i->principal = 1;
      $this->images[] = $i;
    }
    // animal
    if($pa == 1) {
      $animal = array();
      $animal['buddy'] = $buddy;
      $this->animal = json_encode($animal);
    }
    return (int)0;
  }
}
