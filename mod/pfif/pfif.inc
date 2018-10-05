<?
/**
 * @name     Person Finder Interchange Format
 * @author   pl@miernicki.com
 * @about    Developed by the U.S. National Library of Medicine
 * @link     https://gitlab.com/tehk/people-locator
 * @license  https://gitlab.com/tehk/people-locator/blob/master/LICENSE
 */


$failed_images = null;
require_once('lib_pfif.inc');
require_once($conf['approot'].'mod/pfif/util.inc');

class Pfif_Person {

   //Meta data
   public $person_record_id;
   public $entry_date;
   public $expiry_date;
   public $author_name;
   public $author_email;
   public $author_phone;
   public $source_name;
   public $source_date;
   public $source_url;
   //Static identifying information
   public $full_name;
   public $given_name;
   public $family_name;
   public $alternate_names;
   public $profile_urls;
   public $home_city;
   public $home_state;
   public $home_neighborhood;
   public $home_country;
   public $home_street;
   public $home_postal_code;
   public $photo_url;
   public $sex;
   public $date_of_birth;
   public $age;
   public $description;
}

class Pfif_Note {

   public $note_record_id;
   public $person_record_id;
   public $linked_person_record_id;
   public $entry_date;
   public $author_name;
   public $author_email;
   public $author_phone;
   public $source_date;
   public $author_made_contact;
   public $status;
   public $email_of_found_person;
   public $phone_of_found_person;
   public $last_known_location;
   public $text;
   public $photo_url;

}

class Pfif {

   public $personArray = array();
   public $noteArray = array();
   private $pfif_version = null;    // PFIF version
   private $incident_id = null;
   private $service = null;
   private $service_name = null;
   private $dom_pfif;
   private $loadFilters = array();

   public function __construct() {

   }

   // Set service.
   public function setService($service_name, $service) {
      $this->service_name = $service_name;
      $this->service = $service;
      $this->incident_id = $service['incident_id'];
   }

   // Set incident ID.
   public function setIncidentId($incident_id) {
      $this->incident_id = $incident_id;
   }

   public function setVersion($v) {
      $this->pfif_version = $v;
   }

   public function getVersion() {
      return $this->pfif_version;
   }

   public function setSourceReposId($id) {
      $this->service['repository']->id = $id;
   }

   public function getSourceReposId() {
      if (!isset($this->service)) return null;
      return $this->service['repository']->id;
   }

   public function setPerson(Pfif_Person $person) {
      $this->personArray[] = $person;
      //pfif_error_log("Set $person->person_record_id ...");
   }

   public function setNote(Pfif_Note $note) {
      $this->noteArray[] = $note;
      //pfif_error_log("Set $note->note_record_id ...");
   }

   public function setNotes($notes) {
      $this->noteArray = array_merge($this->noteArray, $notes);
   }

   public function getPersons() {
      return $this->personArray;
   }

   // Return all notes or notes for a specified person.
   public function getNotes($person_record_id='') {
      if (empty($person_record_id)) {
         // Return all notes.
         $notes = $this->noteArray;
      } else {
         // Return notes for this person_record_id.
         $notes = array();
         foreach ($this->noteArray as $note) {
            if ($note->person_record_id == $person_record_id) {
               $notes[] = $note;
            }
         }
      }
      return $notes;
   }

   private function createXmlElement($domElement, $property, $value) {
      if (!empty($value)) {
         $child = $this->dom_pfif->createElement($property);
         $text = $this->dom_pfif->createTextNode($value);  // escapes text
         $child->appendChild($text);
         $domElement->appendChild($child);
      }
   }

   private function check_init() {
      if (empty($this->service)) {
         throw new RuntimeException("PFIF service not set");
      }

      return true;
   }

   // Reset log first entry date to first entry date from source repository.
   private function reset_log_dates($entry_date) {
      global $first_entry_date, $last_entry_date, $last_entry_count;

      $first_entry_date = $entry_date;
      $last_entry_date = $first_entry_date;
      $last_entry_count = 0;
   }

   // Count number of entries with same entry_date
   // This is needed when performing incremental harvests so
   // that previously harvested records can be skipped.
   private function update_log_dates($entry_date) {
      global $first_entry_date, $last_entry_date, $last_entry_count;

      if ($entry_date == $last_entry_date) {
         $last_entry_count += 1;
      } else {
         $last_entry_date = $entry_date;
         $last_entry_count = 1;
      }
   }

   private function save_log_dates() {
      global $first_entry_date, $last_entry_date, $last_entry_count;

      $_SESSION['pfif_info']['first_entry'] = $first_entry_date;
      $_SESSION['pfif_info']['last_entry'] = $last_entry_date;
      $_SESSION['pfif_info']['last_entry_count'] = $last_entry_count;
   }

   /**
    *  Returns a PFIF-compliant XML document containing person and note records loaded into this PFIF instance.
    *  Used primarily for export, but also used for debugging import. If filtering on dates has already occurred at
    *  search time, but if only original records are desired now, reuse the original date constraints for refiltering.
    */
   public function storeInXML($embed=true, $original=false, $max=0, $skip=0) {
      global $first_entry_date, $last_entry_date, $last_entry_count, $conf;

      $this->dom_pfif = new DomDocument('1.0', 'utf-8');
      $this->dom_pfif->formatOutput = true;  // pretty-print
      $dom_pfif = $this->dom_pfif->createElement('pfif:pfif');
      $dom_pfif->setAttribute('xmlns:pfif', 'http://zesty.ca/pfif/1.4');
      $dom_pfif->setAttribute('xmlns:xsi', 'http://www.w3.org/2001/XMLSchema-instance');
      $ns = 'http://zesty.ca/pfif/1.4';
      $dom_pfif->setAttribute('xsi:schemaLocation', "$ns $ns/pfif-1.4.xsd");
      $this->dom_pfif->appendChild($dom_pfif);

      $maxcount = 0;
      $skipcount = 0;
      if ($original && !$embed) {
         // Do original records only. Non-embedded format. (Used by Google PF automated exports.)
         // Reuse original date constraints, if any, for filtering the original records. Handle empty dates.
         // NOTE: Currently ignoring skip and max here.
         $start = (empty($this->loadFilters['since']))? 0 : strtotime($this->loadFilters['since']);
         $end = (empty($this->loadFilters['before']))? strtotime('2036') : strtotime($this->loadFilters['before']);

         $pfif_persons = $this->getPersons();
         if (count($pfif_persons) != 0) {
            // Do log housekeeping since this is an automated export (i.e. from cronexport.php).
            $this->reset_log_dates($pfif_persons[0]->entry_date);
            foreach ($pfif_persons as $person) {
               // Snag entry_date for log since this indicates new information (could be from a new Note).
               $this->update_log_dates($person->entry_date);
               // Is this an original record meeting the date constraint?
               if ((strtotime($person->source_date) > $start) &&
                   (strtotime($person->source_date) < $end) &&
                   (strncmp($person->person_record_id, $conf['base_uuid'], strlen($conf['base_uuid']))==0)) {
                  $personElement = $this->createPersonElement($person);
                  $this->dom_pfif->firstChild->appendChild($personElement);
               }
            }
            unset($person);
         }
         $pfif_notes = $this->getNotes();
         if (count($pfif_notes) != 0) {
            foreach ($pfif_notes as $note) {
               // Is this an original record meeting the date constraint?
               if ((strtotime($note->source_date) > $start) &&
                   (strtotime($note->source_date) < $end) &&
                   (strncmp($note->note_record_id, $conf['base_uuid'], strlen($conf['base_uuid']))==0)) {
                  $noteElement = $this->createNoteElement($note);
                  $this->dom_pfif->firstChild->appendChild($noteElement);
               }
            }
            unset($note);
         }
         // Save person and note counts.
         // Fixme: getElementsbyTagNameNS not working for some reason.
         $persons = $this->dom_pfif->getElementsByTagName('pfif:person');
         $notes = $this->dom_pfif->getElementsByTagName('pfif:note');
         $_SESSION['pfif_info']['pfif_person_count'] = $persons->length;
         $_SESSION['pfif_info']['pfif_note_count'] = $notes->length;
         $this->save_log_dates();
      } else if (!$embed) {
         // Serialize persons and notes separately. (Used by cronimport.php debug.)
         $pfif_persons = $this->getPersons();
         if (count($pfif_persons) != 0) {
            foreach ($pfif_persons as $person) {
               $personElement = $this->createPersonElement($person);
               $this->dom_pfif->firstChild->appendChild($personElement);
            }
            unset($person);
         }
         $pfif_notes = $this->getNotes();
         if (count($pfif_notes) != 0) {
            foreach ($pfif_notes as $note) {
               $noteElement = $this->createNoteElement($note);
               $this->dom_pfif->firstChild->appendChild($noteElement);
            }
            unset($note);
         }
      } else {
         // Serialize persons and embed notes. (Used by data feed and export GUI.)
         // Date filters already applied during search, but honor original, max and skip.
         $pfif_persons = $this->getPersons();
         foreach ($pfif_persons as $person) {
        if ($original) {
          // Must be original.
               if (strncmp($person->person_record_id, $conf['base_uuid'], strlen($conf['base_uuid']))!=0) {
             continue;
          }
        }
        $skipcount++;
        if ($skipcount <= $skip) continue;
            $personElement = $this->createPersonElement($person);
            // Embed all notes associated with this person.
            $pfif_notes = $this->getNotes($person->person_record_id);
            foreach ($pfif_notes as $note) {
               $noteElement = $this->createNoteElement($note);
               $personElement->appendChild($noteElement);
            }
            $this->dom_pfif->firstChild->appendChild($personElement);
            unset($note);
        $maxcount++;
        if ($maxcount >= $max) break;
         }
         unset($person);
      }

      if ($this->dom_pfif->firstChild->hasChildNodes()) {
         return $this->dom_pfif->saveXML();
      } else {
         return null;
      }
   }

   private function createPersonElement($person) {
      $personElement = $this->dom_pfif->createElement('pfif:person');
      $this->createXmlElement($personElement, 'pfif:person_record_id', $person->person_record_id);
      $this->createXmlElement($personElement, 'pfif:entry_date', utc_date($person->entry_date));
      $this->createXmlElement($personElement, 'pfif:expiry_date', utc_date($person->expiry_date));
      $this->createXmlElement($personElement, 'pfif:author_name', $person->author_name);
      // TODO: Condition output of email and phone on author's consent.
      // $this->createXmlElement($personElement, 'pfif:author_email' , $person->author_email);
      // $this->createXmlElement($personElement, 'pfif:author_phone' , $person->author_phone);
      $this->createXmlElement($personElement, 'pfif:source_name', $person->source_name);
      $this->createXmlElement($personElement, 'pfif:source_date', utc_date($person->source_date));
      $this->createXmlElement($personElement, 'pfif:source_url', $person->source_url);
      $this->createXmlElement($personElement, 'pfif:full_name', $person->full_name);
      $this->createXmlElement($personElement, 'pfif:given_name', $person->given_name);
      $this->createXmlElement($personElement, 'pfif:family_name', $person->family_name);
      $this->createXmlElement($personElement, 'pfif:alternate_names', $person->alternate_names);
      $this->createXmlElement($personElement, 'pfif:profile_urls', $person->profile_urls);
      $this->createXmlElement($personElement, 'pfif:sex', $person->sex);
      $this->createXmlElement($personElement, 'pfif:age', $person->age);
      $this->createXmlElement($personElement, 'pfif:date_of_birth', $person->date_of_birth);
      $this->createXmlElement($personElement, 'pfif:home_street', $person->home_street);
      $this->createXmlElement($personElement, 'pfif:home_neighborhood', $person->home_neighborhood);
      $this->createXmlElement($personElement, 'pfif:home_city', $person->home_city);
      $this->createXmlElement($personElement, 'pfif:home_state', $person->home_state);
      $this->createXmlElement($personElement, 'pfif:home_country', $person->home_country);
      $this->createXmlElement($personElement, 'pfif:home_postal_code', $person->home_postal_code);
      $this->createXmlElement($personElement, 'pfif:photo_url', $person->photo_url);
      $this->createXmlElement($personElement, 'pfif:description', $person->description);
      return $personElement;
   }

   private function createNoteElement($note) {
      $noteElement = $this->dom_pfif->createElement('pfif:note');
      $this->createXmlElement($noteElement, 'pfif:note_record_id', $note->note_record_id);
      $this->createXmlElement($noteElement, 'pfif:person_record_id', $note->person_record_id);
      $this->createXmlElement($noteElement, 'pfif:linked_person_record_id', $note->linked_person_record_id);
      $this->createXmlElement($noteElement, 'pfif:entry_date', utc_date($note->entry_date));
      $this->createXmlElement($noteElement, 'pfif:author_name', $note->author_name);
      // A policy decision is required in order to allow these to be emitted without the author's consent,
      // or a software/db mechanism is needed to allow the author to provide consent.
      // $this->createXmlElement($noteElement, 'pfif:author_email' , $note->author_email);
      // $this->createXmlElement($noteElement, 'pfif:author_phone' , $note->author_phone);
      $this->createXmlElement($noteElement, 'pfif:source_date', utc_date($note->source_date));
      $this->createXmlElement($noteElement, 'pfif:author_made_contact', $note->author_made_contact);
      $this->createXmlElement($noteElement, 'pfif:status', $note->status);
      $this->createXmlElement($noteElement, 'pfif:last_known_location', $note->last_known_location);
      // These should only be emitted to trusted parties.
      // $this->createXmlElement($noteElement, 'pfif:email_of_found_person' , $note->email_of_found_person);
      // $this->createXmlElement($noteElement, 'pfif:phone_of_found_person' , $note->phone_of_found_person);
      $this->createXmlElement($noteElement, 'pfif:text', $note->text);
      $this->createXmlElement($noteElement, 'pfif:photo_url', $note->photo_url);
      return $noteElement;
   }

   public function postToService($xml) {
      $this->check_init();
      //print "posting to service $this->service_name \n";
      // Harcoding Google service as it's the only game in town right now.
      $status = $this->postGoogleItem($this->service, $xml);
      return $status;
   }

   function postGoogleItem($service_config, $item, $item_type='data') {
      // DEBUG:  print "PFIF Configuration:<pre>";print_r($service_config);print "</pre>";
      $service_uri = $service_config['post_url'];

      // TODO: may need to add logic to support services other than Google.
      $subdomain = empty($service_config['subdomain']) ? '' : '?subdomain=' . $service_config['subdomain'];
      $auth_key = empty($service_config['auth_key']) ? '' : 'key=' . $service_config['auth_key'];
      if (!empty($auth_key)) {
         if (empty($subdomain)) {
            $auth_key = '?'.$auth_key;
         } else {
            $auth_key = '&'.$auth_key;
         }
      }
      $pfif_uri = $service_uri.$subdomain.$auth_key;
      $response = $this->do_post_request($pfif_uri, $item, 'Content-Type: application/xml');
      // The old curl approach should still work if necessary.
      //$response = $this->do_post_request_with_curl($pfif_uri, $item);
      return $response;
   }

   private function do_post_request($url, $data, $optional_headers = null) {
      ini_set('track_errors', 1);
      $params = array('http' => array(
              'method' => 'POST',
              'content' => $data
            ));
      if ($optional_headers !== null) {
        $params['http']['header'] = $optional_headers;
      }
      $ctx = stream_context_create($params);
      $fp = @fopen($url, 'rb', false, $ctx);
      ini_set('track_errors', 0);
      if (!$fp) {
        pfif_error_log("Error in do_post_request with $url, $php_errormsg");
        return -1;
      }
      $response = @stream_get_contents($fp);
      if ($response === false) {
        pfif_error_log("Error in do_post_request reading data from $url, $php_errormsg");
        return -1;
      }
      return $response;
   }

   /*
    * Note that command-line curl comes in handy for debugging.
    * curl -X POST -H 'Content-type: application/xml' --data-binary @your_file.xml \
      http://haiticrisis.appspot.com/api/write?key=your_auth_key
    */
   private function do_post_request_with_curl($pfif_uri, $item, $item_type='data') {
      $ch = curl_init();    /* Create a CURL handle. */
      //print "cURL init " . ($ch ? "true" : "false") . "\n";
      /* Set cURL options. */
      // Set temporarily when Google had SSL problems.
      $status = $ch ? curl_setopt($ch, CURLOPT_SSL_VERIFYPEER, false) : false;
      $status = $ch ? curl_setopt($ch, CURLOPT_SSL_VERIFYHOST, false) : false;

      $status = $ch ? curl_setopt($ch, CURLOPT_URL, $pfif_uri) : false;
      print "URL $pfif_uri set " . ($status ? "true" : "false") . "\n";

      $status &= $status ? curl_setopt($ch, CURLOPT_POST, true) : false;
      //print "POST set " . ($status ? "true" : "false") . "\n";

      $status &= $status ? curl_setopt($ch, CURLOPT_RETURNTRANSFER, true) : false;
      //print "RETURNTRANSFER set " . ($status ? "true" : "false") . "\n";

      $status &= $status ? curl_setopt($ch, CURLOPT_FAILONERROR, true) : false;
      //print "FAILONERROR set " . ($status ? "true" : "false") . "\n";

      $status &= $status ? curl_setopt($ch, CURLOPT_HTTPHEADER, array(
                  'Content-Type: application/xml'
              )) : false;
      //print "HTTPHEADER set " . ($status ? "true" : "false") . "\n";

      $status &= $status ? curl_setopt($ch, CURLOPT_POSTFIELDS, $item) : false; // TODO: if item_type= 'file' use @ syntax
      //print "POSTFIELDS set " . ($status ? "true" : "false") . "\n";

      //$status=false; //debug-skip exec

      $result = $status ? curl_exec($ch) : "POST not executed";  /* Execute the HTTP request. */
      if (!$result) {
         $result = curl_error($ch) . "\n";
      }
      $result .= print_r(curl_getinfo($ch), true);

      curl_close($ch);           /* Close the cURL handle. */

      return $result;
   }

   // Convenience function: Load PFIF persons AND notes XML data into Pfif class.
   // Returns total items (persons + notes) loaded or -1.
   public function loadFromXML($capxml) {
      $numPersons = $this->loadPersonsFromXML($capxml);
      if ($numPersons < 0) return $numPersons;
      $numNotes = $this->loadNotesFromXML($capxml);
      if ($numNotes < 0) {
         return $numNotes;
      } else {
         return $numPersons + $numNotes;
      }
   }

   // Load PFIF XML data into Pfif_Person class.
   public function loadPersonsFromXML($capxml) {
      $this->dom_pfif = new DOMDocument('1.0', 'utf-8');
      if (isset($capxml)) {
         if (stripos($capxml, '<' . PFIF_NS_PREFIX . ':') === FALSE) {
            //print "loading from file/url ...\n>".$capxml."<\n with length ".strlen($capxml)."\n";
            libxml_use_internal_errors(true);
            $lst = @$this->dom_pfif->load($capxml); // load from file or URL
            //print "loaded\n";
         } else {
            //print "loading from string ...\n";
            $lst = $this->dom_pfif->loadXML($capxml); // load from string
         }
         //print "load status = " . ($lst ? "TRUE" : "FALSE") . "\n";
         if (!$lst) {
            $msg = 'libxml error start: ';
            // Transient warning about some kind of server error. Just send to stdout.
            foreach (libxml_get_errors() as $error) {
               $msg .= $error->message . "\n";
            }
            print "$msg :libxml error end";
            libxml_clear_errors();
            return 0; 
         }
      }

      $ns = $this->dom_pfif->documentElement->lookupnamespaceURI(PFIF_NS_PREFIX); // 'http://zesty.ca/pfif/1.4';
      // DEBUG: print('loadPersonsFromXML: ns='.$ns.'\n');
      $this->setVersion(($ns == PFIF_1_3_NAMESPACE) ? PFIF_V_1_3 : PFIF_V_1_4);

      $persons = $this->dom_pfif->getElementsByTagNameNS($ns, 'person');
      // $dump = var_export($persons, true);
      // pfif_error_log("got persons:".$dump." length = ".$persons->length);
      if ($persons->length > 0) {
         $this->process_persons($persons, $ns);
      }

      return $persons->length;
   }

   // Load PFIF XML data into Pfif_Note class.
   public function loadNotesFromXML($capxml) {
      $this->dom_pfif = new DOMDocument('1.0', 'utf-8');
      if (isset($capxml)) {
         if (stripos($capxml, '<' . PFIF_NS_PREFIX . ':') === FALSE) {
            // print "loading from file/url ...\n>".$capxml."<\n with length ".strlen($capxml)."\n";
            libxml_use_internal_errors(true);
            $lst = @$this->dom_pfif->load($capxml); // load from file or URL
         } else {
            //print "loading from string ...\n";
            $lst = $this->dom_pfif->loadXML($capxml); // load from string
         }
         //print "load status = " . ($lst ? "TRUE" : "FALSE") . "\n";
         if (!$lst) {
            // Transient warning about some kind of server error. Just send to stdout.
            $msg = 'libxml error start: ';
            foreach (libxml_get_errors() as $error) {
               $msg .= $error->message . "\n";
            }
            print "$msg :libxml error end";
            libxml_clear_errors();
            return 0; 
         }
      }

      $ns = $this->dom_pfif->documentElement->lookupnamespaceURI(PFIF_NS_PREFIX); // 'http://zesty.ca/pfif/1.4';
      // DEBUG: print('loadNotesFromXML: ns='.$ns.'\n');
      $this->setVersion(($ns == PFIF_1_3_NAMESPACE) ? PFIF_V_1_3 : PFIF_V_1_4);

      $notes = $this->dom_pfif->getElementsByTagNameNS($ns, 'note');
      if ($notes->length > 0) {
         $this->process_notes($notes, $ns);
      }

      return $notes->length;
   }

   // Extract DOM person data into pif_person.
   // Drop any embedded notes on the floor and process them as a separate feed.
   private function process_persons($persons, $ns) {
      foreach ($persons as $person) {
         $pfif_person = new Pfif_Person();
         $pfif_person->person_record_id = $this->getValueFromXmlElementTag($person, 'person_record_id', $ns);
         $pfif_person->entry_date = local_date($this->getValueFromXmlElementTag($person, 'entry_date', $ns));
         $pfif_person->expiry_date = local_date($this->getValueFromXmlElementTag($person, 'expiry_date', $ns));
         $pfif_person->author_name = $this->getValueFromXmlElementTag($person, 'author_name', $ns);
         $pfif_person->author_email = $this->getValueFromXmlElementTag($person, 'author_email', $ns);
         $pfif_person->author_phone = $this->getValueFromXmlElementTag($person, 'author_phone', $ns);
         $pfif_person->source_name = $this->getValueFromXmlElementTag($person, 'source_name', $ns);
         $pfif_person->source_date = local_date($this->getValueFromXmlElementTag($person, 'source_date', $ns));
         $pfif_person->source_url = $this->getValueFromXmlElementTag($person, 'source_url', $ns);
         // Static identifying information
         $pfif_person->full_name = $this->getValueFromXmlElementTag($person, 'full_name', $ns);
         $pfif_person->given_name = $this->getValueFromXmlElementTag($person, 'given_name', $ns);
         $pfif_person->family_name = $this->getValueFromXmlElementTag($person, 'family_name', $ns);
         $pfif_person->alternate_names = $this->getValueFromXmlElementTag($person, 'alternate_names', $ns);
         $pfif_person->profile_urls = $this->getValueFromXmlElementTag($person, 'profile_urls', $ns);
         $pfif_person->home_city = $this->getValueFromXmlElementTag($person, 'home_city', $ns);
         $pfif_person->home_state = $this->getValueFromXmlElementTag($person, 'home_state', $ns);
         $pfif_person->home_neighborhood = $this->getValueFromXmlElementTag($person, 'home_neighborhood', $ns);
         $pfif_person->home_street = $this->getValueFromXmlElementTag($person, 'home_street', $ns);
         $pfif_person->home_country = $this->getValueFromXmlElementTag($person, 'home_country', $ns);
         $pfif_person->home_postal_code = $this->getValueFromXmlElementTag($person, 'home_postal_code', $ns);
         $pfif_person->photo_url = $this->getValueFromXmlElementTag($person, 'photo_url', $ns);
         $pfif_person->description = $this->getValueFromXmlElementTag($person, 'description', $ns);
         $pfif_person->age = $this->getValueFromXmlElementTag($person, 'age', $ns);
         $pfif_person->date_of_birth = $this->getValueFromXmlElementTag($person, 'date_of_birth', $ns);
         $pfif_person->sex = $this->getValueFromXmlElementTag($person, 'sex', $ns);
         // If empty record without expiration date, expire it. (JIRA PL-183)
         if (empty($pfif_person->expiry_date)
                 && empty($pfif_person->full_name)
                 && empty($pfif_person->given_name)
                 && empty($pfif_person->family_name)
                 && empty($pfif_person->alternate_names)
                 && empty($pfif_person->profile_urls)
                 && empty($pfif_person->description)
                 && empty($pfif_person->photo_url)) {
            $pfif_person->expiry_date = date("Y-m-d H:i:s");
         }
         $this->setPerson($pfif_person);
      }
      unset($person);
   }

   // Extract DOM note data into pfif_notes.
   private function process_notes($notes, $ns) {
      foreach ($notes as $note) {
         $pfif_note = new Pfif_Note();
         $pfif_note->note_record_id = $this->getValueFromXmlElementTag($note, 'note_record_id', $ns);
         $pfif_note->entry_date = local_date($this->getValueFromXmlElementTag($note, 'entry_date', $ns));
         $pfif_note->author_name = $this->getValueFromXmlElementTag($note, 'author_name', $ns);
         $pfif_note->author_email = $this->getValueFromXmlElementTag($note, 'author_email', $ns);
         $pfif_note->author_phone = $this->getValueFromXmlElementTag($note, 'author_phone', $ns);
         $pfif_note->source_date = local_date($this->getValueFromXmlElementTag($note, 'source_date', $ns));
         $pfif_note->status = $this->getValueFromXmlElementTag($note, 'status', $ns);
         $pfif_note->author_made_contact = $this->getValueFromXmlElementTag($note, 'author_made_contact', $ns);
         $pfif_note->email_of_found_person = $this->getValueFromXmlElementTag($note, 'email_of_found_person', $ns);
         $pfif_note->phone_of_found_person = $this->getValueFromXmlElementTag($note, 'phone_of_found_person', $ns);
         $pfif_note->last_known_location = $this->getValueFromXmlElementTag($note, 'last_known_location', $ns);
         $pfif_note->text = $this->getValueFromXmlElementTag($note, 'text', $ns);
         $pfif_note->photo_url = $this->getValueFromXmlElementTag($note, 'photo_url', $ns);
         $pfif_note->person_record_id = $this->getValueFromXmlElementTag($note, 'person_record_id', $ns);
         $pfif_note->linked_person_record_id = $this->getValueFromXmlElementTag($note, 'linked_person_record_id', $ns);
         $this->setNote($pfif_note);
      }
      unset($note);
   }

   private function getValueFromXmlElementTag($elementNode, $elementName, $ns) {
      // Default value.
      $return_value = "";
      $nsValue = $elementNode->getElementsByTagNameNS($ns, $elementName);
      if ($nsValue->length != 0) {
         $return_value = $nsValue->item(0)->nodeValue;
      } else {
         // Try to get the element without namespace just in case that helps.
         $nonNsValue = $elementNode->getElementsByTagName($elementName);
         if ($nonNsValue->length != 0) {
            $return_value = $nonNsValue->item(0)->nodeValue;
         }
      }

      return $return_value;
   }

   // Convenience function: Store persons AND notes in DB.
   public function storeInDatabase($incident_id) {
      global $global;

      $this->incident_id = $incident_id;
      $results = array();
      $numPersons = $this->storePersonsInDatabase();
      $results['person'] = $numPersons;
      $numNotes = $this->storeNotesInDatabase();
      $results['note'] = $numNotes;
      return $results;
   }

  // Store persons in database and update with any existing notes. PFIF 1.3 and beyond allows for persons to
  // be reimported when expiration date changes or other data is corrected. They should be given a new source_date
  // and entry_date.
  public function storePersonsInDatabase() {
    global $conf, $global, $failed_images, $first_entry_date, $last_entry_date, $last_entry_count;

    // Temporarily turn off strict mode while loading unconstrained Google PF data to allow unvalidated data entry. 
    $global['db']->execute('SET SESSION sql_mode = "NO_ENGINE_SUBSTITUTION"');

    $failed_images = array();
    $_SESSION['pfif_info']['images_in'] = 0;
    $_SESSION['pfif_info']['images_retried'] = 0;
    $_SESSION['pfif_info']['images_failed'] = 0;
    $_SESSION['pfif_info']['person_insert_errors'] = 0;
    $_SESSION['pfif_info']['person_insert_dups'] = 0;
    $pfif_persons = $this->getPersons();
    $this->reset_log_dates($pfif_persons[0]->entry_date);

    //var_dump("first/last/count at start: ", $first_entry_date, $last_entry_date, $last_entry_count);
    foreach ($pfif_persons as $person) {
      // Update last_entry_count.
      $this->update_log_dates($person->entry_date);

      // Skip original records. (Might be worthwhile for batch-loading test records, but we would
      // need a way to generate unique record IDs.)
      if (strncmp($person->person_record_id, $conf['base_uuid'], strlen($conf['base_uuid']))==0) {
         $_SESSION['pfif_info']['person_insert_errors'] += 1;
         continue;
      }
      // Check for duplicate.
      $lookup = $this->checkForReportedPerson($person->person_record_id);
      //var_dump('lookup returns:',$lookup);
      $newPerson = (count($lookup) == 0);
      if (!$newPerson) {
         $_SESSION['pfif_info']['person_insert_dups'] += 1;
         if (!empty($person->expiry_date) && strtotime($person->expiry_date) < strtotime("+1 hour")) {   // +1 hour fudge factor
           // When a record is deleted, Google sends out an expunged record to expire the person.
           // These records are supposed to have a matching source and expiry date but that test
           // is unreliable (PL-463). Instead look for records that have expired or are about to 
           // expire and skip these records except to update the expiration date.
           $this->taupo_update_expiry_date($person);
           continue;
         } else {
           // Replace duplicate person (PFIF says it should have a new source date, but in
           // practice Google doesn't follow this and overwrites anything in PF.)
           $this->deleteReportedPerson($person->person_record_id);
         }
      }

      // Save
      try {
         $this->taupo_pfif_addperson_commit($person);
      } catch (Exception $e) {
         pfif_error_log("Error in storePersonsInDatabase: taupo_pfif_addperson_commit failed for "
                 . $person->person_record_id . ":" . $e->getMessage());
         $_SESSION['pfif_info']['person_insert_errors'] += 1;
         continue;  // stop processing this person
      }

      // Update stats if new person.
      if ($newPerson) {
        updateArrivalRate($person->person_record_id, $this->incident_id, false, false, false, true, false);
      }

      // Save to pfif_person table.
      try {
        $this->taupo_pfif_addpfifperson_commit($person);
      } catch (Exception $e) {
        pfif_error_log("Error in storePersonsInDatabase: taupo_pfif_addpfifperson_commit failed for "
                . $person->person_record_id . ":" . $e->getMessage());
      }

      // Update with dynamic data from all notes since some may have arrived before this record.
      $this->_pfif_update_person_from_notes($person->person_record_id);
      // Do same for person_notes (which will have been deleted if we've seen this p_uuid before).
      $this->_pfif_add_person_notes_from_notes($person->person_record_id);
      // Do the same for images.
      $this->_pfif_add_images($person->person_record_id);

      // Flag person record insertion as complete.
      $sql = "UPDATE person_uuid SET complete=1" .
             " WHERE p_uuid='".$person->person_record_id."'";
      $ret = $global['db']->Execute($sql);
      if ($ret === false) {
        $errchk = $global['db']->ErrorMsg();
        pfif_error_log("Error in storePersonsinDatabase marking record as complete: " . $errchk);
      }

      // Update SOLR indexes.
      taupo_solr_add_person($person->person_record_id);
    }
    unset($person);
    //var_dump("first/last/count at end: ", $first_entry_date, $last_entry_date, $last_entry_count);
    // Update status info
    $this->save_log_dates();

    if (!empty($failed_images)) {
      $_SESSION['pfif_info']['images_failed'] = count($failed_images);
      print "The following photos were not retrieved:\n";
      print_r($failed_images);
    }

    // Save number of new persons.
    $_SESSION['pfif_info']['pfif_person_count'] =
      count($pfif_persons) - ($_SESSION['pfif_info']['person_insert_errors'] + $_SESSION['pfif_info']['person_insert_dups']);

    $global['db']->execute('SET SESSION sql_mode = "NO_ENGINE_SUBSTITUTION,STRICT_TRANS_TABLES"');

    return $_SESSION['pfif_info']['pfif_person_count'];
   }

   public function storeNotesInDatabase() {
      global $conf, $global, $first_entry_date, $last_entry_date, $last_entry_count;

      // Temporarily turn off strict mode while loading unconstrained Google PF data to allow unvalidated data entry. 
      $global['db']->execute('SET SESSION sql_mode = "NO_ENGINE_SUBSTITUTION"');

      $_SESSION['pfif_info']['images_in'] = 0;
      $_SESSION['pfif_info']['images_retried'] = 0;
      $_SESSION['pfif_info']['images_failed'] = 0;
      $_SESSION['pfif_info']['note_insert_errors'] = 0;
      $pfif_notes = $this->getNotes();
      if (count($pfif_notes) > 0) $this->reset_log_dates($pfif_notes[0]->entry_date);

      //var_dump("first/last/count at start: ", $first_entry_date, $last_entry_date, $last_entry_count);
      foreach ($pfif_notes as $note) {
         // Update last_entry_count.
         $this->update_log_dates($note->entry_date);

         // Check for duplicates.
         $lookup = $this->checkForReportedNote($note->note_record_id);
         //var_dump('lookup returns:',$lookup);
         if (count($lookup) > 0) {
            // This skips Notes reported in PL.
            // TODO: Should other duplicates be passed through, so error can be reported at DB insert time?
            $_SESSION['pfif_info']['note_insert_errors'] += 1;
            continue; // stop processing this note
         }

         // Save to pfif_note table.
         try {
            $this->taupo_pfif_addpfifnote_commit($note);
         } catch (Exception $e) {
            $_SESSION['pfif_info']['note_insert_errors'] += 1;
             pfif_error_log("Error in storeNotesInDatabase: taupo_pfif_addpfifnote_commit failed for "
                    . $note->note_record_id . ":" . $e->getMessage());
            continue; // stop processing this note
         }

         // Update with dynamic data if the person exists and this is not an original note.
         if (strncmp($note->note_record_id, $conf['base_uuid'], strlen($conf['base_uuid']))!=0) {
            $lookup = $this->checkForReportedPerson($note->person_record_id);
            if (count($lookup) > 0) {
               // Can't rely on notes coming in in source date order. (Only entry date order is assured.)
               // So for each note, reprocess all existing notes for the associated person.
               $this->_pfif_update_person_from_notes($note->person_record_id);
               // Also add a person_note from this note. (Note: This updates SOLR indexes.)
               $this->_pfif_add_person_note_from_note(
                  $note->person_record_id,
                  $note->text,
                  taupo_map_status_from_pfif($note->status, $note->author_made_contact), 
                  $note->last_known_location, 
                  null,
                  null, 
                  $note->note_record_id,
                  $note->author_name,
                  date("Y-m-d H:i:s"));
               // Finally, fetch any image.
               $this->_pfif_fetch_image($note->person_record_id, $note->note_record_id, $note->photo_url);
            }
         }
      }
      unset($note);
      //var_dump("first/last/count at end: ", $first_entry_date, $last_entry_date, $last_entry_count);
      // Update status info
      $this->save_log_dates();

      // Update number of notes actually processed.
      $_SESSION['pfif_info']['pfif_note_count'] = count($pfif_notes) - $_SESSION['pfif_info']['note_insert_errors'];

      $global['db']->execute('SET SESSION sql_mode = "NO_ENGINE_SUBSTITUTION,STRICT_TRANS_TABLES"');

      return $_SESSION['pfif_info']['pfif_note_count'];
   }

   // Update with relevant information from this person's notes (if any).
   // NOTE: Person may not exist yet, so we also come here when we insert a person.
   private function _pfif_update_person_from_notes($p_uuid) {
      global $global;
      // Get all notes for this person.
      $sql = "SELECT pn.note_record_id, pn.source_date,pn.status, pn.author_made_contact, pn.last_known_location" .
              " FROM pfif_note pn WHERE pn.p_uuid = '" . $p_uuid . "' ORDER BY pn.source_date";
      $note_result = $global['db']->Execute($sql);
      if ($note_result === false) {
         $errchk = $global['db']->ErrorMsg();
         pfif_error_log("Error in _pfif_update_person_from_notes getting later notes: " . $errchk);
      }
      // Initialize to least specificity.
      $old_status = 'unk';
      $last_known_location = '';
      $source_date = '';
      while ($note_row = $note_result->FetchRow()) {
         // Update last_seen if present.
         if (!empty($note_row['last_known_location'])) {
            $last_known_location = $note_row['last_known_location'];
         }
         // Determine summary PFIF status.
         $status = $note_row['status'];
         $found = $note_row['author_made_contact'];
         $mapped_status = taupo_map_status_from_pfif($status, $found, $old_status);
         $old_status = $mapped_status;
         // Update last_updated.
         $source_date = $note_row['source_date'];
      }
      // Update DB with notes, if any.
      if (!empty($source_date)) {
         // Only update last known location if not empty.
         if (empty($last_known_location)) {
            $sql = "UPDATE person_status ps SET" .
                " ps.last_updated_db='" . date("Y-m-d H:i:s") . "'" .
                ", ps.last_updated='" . $source_date . "'" .
                ", ps.opt_status='" . $mapped_status . "'" .
                " WHERE ps.p_uuid='$p_uuid'";
         } else {
            $sql = "UPDATE person_status ps,person_details pd SET" .
                " ps.last_updated_db='" . date("Y-m-d H:i:s") . "'" .
                ", ps.last_updated='" . $source_date . "'" .
                ", ps.opt_status='" . $mapped_status . "'" . 
                ", pd.last_seen=" . $global['db']->qstr($last_known_location) .
                " WHERE ps.p_uuid='$p_uuid' AND ps.p_uuid = pd.p_uuid";
         }
         $ret = $global['db']->Execute($sql);
         if ($ret === false) {
            $errchk = $global['db']->ErrorMsg();
            pfif_error_log("Error in _pfif_update_person_from_notes performing update: " . $errchk);
         }
         
      }
   }

   // Make sure 'person_notes' are up to date with notes. (We now use these for the full record page.)
   private function _pfif_add_person_notes_from_notes($p_uuid) {
      global $global;
      // Add all pre-existing notes, if any.
      $sql = "
        SELECT pn.note_record_id, pn.status, pn.author_made_contact, pn.last_known_location, pn.text, pn.author_name, pn.entry_date
        FROM pfif_note pn
        WHERE pn.p_uuid = '$p_uuid'
      ";
      $note_result = $global['db']->Execute($sql);
      if ($note_result === false) {
         $errchk = $global['db']->ErrorMsg();
         pfif_error_log("Error in _pfif_add_person_notes_from_notes getting notes: " . $errchk);
      }
      while ($note_row = $note_result->FetchRow()) {
         $authorName          = $note_row['author_name'];
         $suggested_location  = (empty($note_row['last_known_location']))? null : $note_row['last_known_location'];
         $suggested_status    = taupo_map_status_from_pfif($note_row['status'], $note_row['author_made_contact']);
         $comment = (empty($note_row['text']))? null : $note_row['text'];
         $note_record_id      = $note_row['note_record_id'];
         $base64imageData     = null;
         $authorUuid          = null;
         $entry_date          = $note_row['entry_date'];
         $this->_pfif_add_person_note_from_note($p_uuid, $comment, $suggested_status, $suggested_location, $base64imageData, $authorUuid, $note_record_id, $authorName, $entry_date);
      }
   }

   // Create a 'person_note' for the specified note information.
   private function _pfif_add_person_note_from_note($p_uuid, $comment, $suggested_status, $suggested_location, $base64imageData, $authorUuid, $note_record_id, $authorName, $entry_date) {
      global $global;
      $p = new person();
      $p->p_uuid = $p_uuid;
      $p->load();
      $p->addComment($comment, $suggested_status, $suggested_location, $base64imageData, 3, $note_record_id, $authorName, $entry_date);
      $p->update(true); // true so we skip the person_status update
   }

   // Make sure all note images for this person have been fetched.
   private function _pfif_add_images($person_record_id) {
      global $global;
      $sql = "SELECT pn.note_record_id, pn.photo_url FROM pfif_note pn" .
           " WHERE pn.p_uuid = '$person_record_id' AND pn.photo_url IS NOT NULL";
      $note_result = $global['db']->Execute($sql);
      if ($note_result === false) {
         $errchk = $global['db']->ErrorMsg();
         pfif_error_log("Error in _pfif_add_images getting notes: " . $errchk);
      }
      while ($note_row = $note_result->FetchRow()) {
         $this->_pfif_fetch_image($person_record_id, $note_row['note_record_id'], $note_row['photo_url']);
      }
   }

   // Fetch an image and store in database.
   private function _pfif_fetch_image($person_record_id, $note_record_id, $photo_url) {
      global $conf;
      // Fetch image, if any.
      $image_info = fetch_image($photo_url, $person_record_id);
      // Save image info.
      if (!empty($image_info)) {
         pfif_image_to_db($person_record_id, $note_record_id, $image_info['image_type'], $image_info['image_height'], $image_info['image_width'], $image_info['channels'], $image_info['url'], $image_info['url_thumb'], $image_info['original_filename']);
         if($conf['image_search']) {
            // Submit the image to Facematch for indexing.
            try {
               taupo_facematch_ingest($image_info['url'], $this->incident_id, LOCAL_SOURCE, taupo_facematch_get_attrs($person_record_id));
               //print "FaceMatch ingest success: image '".$image_info['url']."', incident '".$this->incident_id."'\n";
            } catch (Exception $e) {
               // FaceMatch log will capture it.
            }
         }
      }
   }

   /**
    * Save
    */
   private function taupo_pfif_addperson_commit($person) {
      // person_uuid table
      $insert_array['p_uuid'] = $person->person_record_id;
      if (!empty($person->full_name)) {
         $insert_array['full_name'] = $person->full_name;
      } else {
         // Try to cobble something together. PL searches on this.
         $full_name = $person->given_name . ' ' . $person->family_name;
         if ($full_name != ' ') {
            $insert_array['full_name'] = $full_name;
         }
      }
      if (!empty($person->given_name)) {
         $insert_array['given_name'] = $person->given_name;
      }
      if (!empty($person->family_name)) {
         $insert_array['family_name'] = $person->family_name;
      }
      // If first and last names are empty, but full_name is present, use it for first name (PL-554).
      if (empty($person->given_name) && empty($person->family_name)  && !empty($person->full_name)) {
         $insert_array['given_name'] = $person->full_name;
      }
      if (!empty($person->alternate_names)) {
         $insert_array['alternate_names'] = $person->alternate_names;
      }
      if (!empty($person->profile_urls)) {
         $insert_array['profile_urls'] = $person->profile_urls;
      }
      $insert_array['incident_id'] = $this->incident_id;
      if (!empty($person->expiry_date)) {
         $insert_array['expiry_date'] = $person->expiry_date;
      }
      pfif_db_insert($insert_array, 'person_uuid');
      $insert_array = null; // reset array
      // contact table
      $insert_array['p_uuid'] = $person->person_record_id;
      $address = '';
      if (!empty($person->home_street)) {
         $address = $person->home_street . ", ";
      }
      if (!empty($person->home_neighborhood)) {
         $address .= $person->home_neighborhood . ", ";
      }
      if (!empty($person->home_city)) {
         $address .= $person->home_city . ", ";
      }
      if (!empty($person->home_state)) {
         $address .= $person->home_state;
      }
      if (!empty($person->home_country)) {
         $address .= ", " . $person->home_country;
      }
      if (!empty($address)) {
         $insert_array['contact_value'] = $address;
         $insert_array['opt_contact_type'] = 'home';
         pfif_db_insert($insert_array, 'contact');
      }
      $insert_array = null; // reset array
      $insert_array['p_uuid'] = $person->person_record_id;
      if (!empty($person->home_postal_code)) {
         $insert_array['contact_value'] = $person->home_postal_code;
         $insert_array['opt_contact_type'] = 'zip';
         pfif_db_insert($insert_array, 'contact');
      }
      $insert_array = null; // reset array
      //person_details table
      $insert_array['p_uuid'] = $person->person_record_id;
      $age_info = taupo_map_age_from_pfif($person->date_of_birth, $person->age, $person->source_date);
      if (array_key_exists('date_of_birth', $age_info) && !empty($age_info['date_of_birth'])) {
         $insert_array['birth_date'] = $age_info['date_of_birth'];
      }
      if (array_key_exists('years_old', $age_info) && is_numeric($age_info['years_old'])) {
         $insert_array['years_old'] = $age_info['years_old'];
      } else {
         if (array_key_exists('minAge', $age_info) && is_numeric($age_info['minAge'])) {
            $insert_array['minAge'] = $age_info['minAge'];
         }
         if (array_key_exists('maxAge', $age_info) && is_numeric($age_info['maxAge'])) {
            $insert_array['maxAge'] = $age_info['maxAge'];
         }
      }
      $gender = taupo_map_gender_from_pfif($person->sex);
      if (!empty($gender)) {
         $insert_array['opt_gender'] = $gender;
      }
      // Save "description" field to person details comments.
      if (!empty($person->description)) {
         $insert_array['other_comments'] = $person->description;
      }
      pfif_db_insert($insert_array, 'person_details');
      $insert_array = null; // reset array
      // Image table
      // Fetch image and save. Then index it.
      $this->_pfif_fetch_image($person->person_record_id, null, $person->photo_url);

      //Save reporter information
      $rep_uuid = taupo_create_uuid('record');

      //person_uuid table
      $insert_array['p_uuid'] = $rep_uuid;
      //$insert_array['incident_id'] = $this->incident_id; //reporters have null incident id
      if (!empty($person->author_name)) {
         $insert_array['full_name'] = $person->author_name;
      }
      pfif_db_insert($insert_array, 'person_uuid');
      $insert_array = null; // reset array
      //contact table
      $insert_array['p_uuid'] = $rep_uuid;
      if (!empty($person->author_phone)) {
         $insert_array['contact_value'] = $person->author_phone;
         $insert_array['opt_contact_type'] = 'curr';
         pfif_db_insert($insert_array, 'contact');
      }
      $insert_array = null; // reset array
      $insert_array['p_uuid'] = $rep_uuid;
      if (!empty($person->author_email)) {
         $insert_array['contact_value'] = $person->author_email;
         $insert_array['opt_contact_type'] = 'email';
         pfif_db_insert($insert_array, 'contact');
      }
      $insert_array = null; // reset array
      // person_to_report table
      $insert_array['p_uuid'] = $person->person_record_id;
      $insert_array['rep_uuid'] = $rep_uuid;
      pfif_db_insert($insert_array, 'person_to_report');
      $insert_array = null; // reset array
      // person_status table  Do person_status last so last_updated is accurate (PL-711).
      $insert_array['p_uuid'] = $person->person_record_id;
      $insert_array['opt_status'] = 'unk'; //default
      // Set creation_time and last updated.
      if (!empty($person->source_date)) {
         // If date is in the future, set to current date-time (PL-549).
         $now = date('Y-m-d H:i:s');
         if (strtotime($person->source_date) > strtotime($now)) {  
        $insert_array['creation_time'] = $now;
        $insert_array['last_updated'] = $now;
    } else {
        $insert_array['creation_time'] = $person->source_date;
        $insert_array['last_updated'] = $person->source_date;
         }
      } else {
         // Pick an arbitrary early date.
         $insert_array['creation_time'] = '2000-01-01 00:00:00';
         $insert_array['last_updated'] = '2000-01-01 00:00:00';
      }
      $insert_array['last_updated_db'] = date('Y-m-d H:i:s');
      pfif_db_insert($insert_array, 'person_status');
      $insert_array = null; // reset array
   }

   // Load from database based on optional date window.
   public function loadFromDatabase($since_entry_date=null, $before=null) {
      global $conf, $global;

      // Save filters since they may be needed at XML generation time.
      $this->loadFilters['since'] = $since_entry_date;
      $this->loadFilters['before'] = $before;

      $baseUuid = $conf['base_uuid'];
      $baseUrl = "https://" . $baseUuid;
      $pfifDomain = substr($baseUuid, 0, -1);  // omit trailing "/"

      // Get person records based on filters.
      $personRecords = $this->getPersonsFromDb($since_entry_date, $before);
      $record_count = count($personRecords);

      foreach ($personRecords as $personRecord) {
         // Get the PFIF record if any.
         $sourcePfif = $this->getSourcePfif($personRecord);

         // Populate person.
         $person = new Pfif_Person();

         $person->person_record_id = $personRecord['p_uuid'];
         $person->entry_date = $personRecord['update_time'];
         $person->expiry_date = $sourcePfif ? $sourcePfif->expiry_date : $personRecord['expiry_date'];

         $reporter = $this->getReportedPerson($personRecord['reporter']);
         if (count($reporter) > 0) {
            // pfif_error_log("reporter_record is set!");
            $personRecord['reporter_record'] = $reporter;
            $person->author_name = $personRecord['reporter_record']['full_name'];
            $person->author_email = $personRecord['reporter_record']['email'];
            $person->author_phone = $personRecord['reporter_record']['phone'];
         }

         // Spec defines source_ info is for home repository. Use local info only for our own records.
         $person->source_name = $sourcePfif ? $sourcePfif->source_name : $pfifDomain;
         $person->source_date = $sourcePfif ? $sourcePfif->source_date : $personRecord['creation_time'];
         $person->source_url = $sourcePfif ? $sourcePfif->source_url : $this->getLocalSourceUrl($personRecord['p_uuid']);

         $person->full_name = $sourcePfif ? $sourcePfif->full_name : $personRecord['full_name'];
         $person->given_name = $sourcePfif ? $sourcePfif->given_name : $personRecord['given_name'];
         $person->family_name = $sourcePfif ? $sourcePfif->family_name : $personRecord['family_name'];
         $person->alternate_names = $sourcePfif ? $sourcePfif->alternate_names : $personRecord['alternate_names'];
         $person->profile_urls = $sourcePfif ? $sourcePfif->profile_urls : $personRecord['profile_urls'];

         // Note: For when no source PFIF, we could try to parse $personRecord['address'] better.
         $person->home_city = $sourcePfif ? $sourcePfif->home_city : '';
         $person->home_state = $sourcePfif ? $sourcePfif->home_state : '';
         $person->home_neighborhood = $sourcePfif ? $sourcePfif->home_neighborhood : '';
         // PL uses a textarea, so might be newlines present. Google PF will reject if so.
         $newline = array("\r\n", "\r", "\n");
         $person->home_street = $sourcePfif ? $sourcePfif->home_street : str_replace($newline, ' ', $personRecord['address']);
         $person->home_country = $sourcePfif ? $sourcePfif->home_country : '';
         $person->home_postal_code = $sourcePfif ? $sourcePfif->home_postal_code : $personRecord['zip'];

         if ($sourcePfif) {
            $person->photo_url = $sourcePfif->photo_url;
         } else {
            if (!empty($personRecord['image_url'])) {
               $photo_url = $personRecord['image_url'];
               $person->photo_url = $baseUrl . $photo_url;
            }
         }

         // Since our gender mapping is not lossless (PFIF has "other" category), use the PFIF if it exists.
         if ($sourcePfif) {
            $person->sex = $sourcePfif->sex;
         } else {
            $person->sex = taupo_map_gender_to_pfif($personRecord, $sourcePfif);
         }

         // Use the PFIF age if it exists since we only have to copy it.
         if ($sourcePfif) {
            $person->age = $sourcePfif->age;
         } else {
            $person->age = taupo_map_age_to_pfif($personRecord);
         }
         $person->date_of_birth = $sourcePfif ? $sourcePfif->date_of_birth : $personRecord['dob'];
         if ($sourcePfif) {
            $person->description = $sourcePfif->description;
         } else {
            $details = array();
            if (!empty($personRecord['height'])) {
               $details['height'] = 'height: ' . $personRecord['height'];
            }
            if (!empty($personRecord['weight'])) {
               $details['weight'] = 'weight: ' . $personRecord['weight'];
            }
            if (!empty($personRecord['physical_comments'])) {
               $details['physical_comments'] = 'other features: ' . $personRecord['physical_comments'];
            }
            if (!empty($personRecord['other_comments'])) {
               $details['other_comments'] = 'other comments: ' . $personRecord['other_comments'];
            }
            if (count($details) > 0) {
               $person->description = "description: ";
               foreach ($details as $detail) {
                  $person->description .= $detail . ', ';
               }
               $person->description = substr($person->description, 0, -2);
            }
         }
         $this->setPerson($person);
         $notes = $this->getPfifNotes($person->person_record_id);
         $this->setNotes($notes);
      }
      unset($personRecord);
      if ($personRecords !== false && $record_count >= 0) {
         return $record_count;
      } else {
         return -1;
      }
   }

  // Source url for exported local record should look like this:
  // https://pl.nlm.nih.gov/en/testpf/record#3487205
  private function getLocalSourceUrl($p_uuid) {
    global $conf, $global;

    $baseUuid = $conf['base_uuid'];
    // Get incident short name for this record.
    $sql = "SELECT i.shortname FROM incident i, person_uuid p WHERE p.p_uuid = '".$p_uuid."' AND p.incident_id = i.incident_id";
    $row = $global['db']->GetRow($sql);
    $shortname = $row['shortname'];
    $uuid = str_replace(".", "#", basename($p_uuid));
    return "https://" . $baseUuid . "en/" . $shortname. "/" . $uuid . "/view";
  }

   // Load notes from database based on date, limit and number of records to skip.
   public function loadNotesFromDatabase($since_entry_date, $max_results, $skip) {
      // Get note records based on params.
      $rows = $this->getNotesFromDb($since_entry_date, $max_results, $skip);
      foreach ($rows as $row) {
         $this->setNote($this->createPfifNote($row));
      }
      unset($row);
   }

   // Use threshhold date and number of records to skip.
   private function getPersonsFromDb($since_entry_date, $before) {
      global $global, $conf;

      $whereSql = '';
      $where = array();
      if (!empty($since_entry_date)  && $since_entry_date != -1) {
         $where[] = " pul.update_time >= '$since_entry_date' ";
      }
      if (!empty($before)) {
         $where[] = " pul.update_time <= '$before' ";
      }
      $where[] = " (a.expiry_date IS NULL OR a.expiry_date > now()) ";
      $where[] = " a.complete = 1 ";
      $where[] = " a.incident_id = " . $this->incident_id;

      foreach ($where as $whereElement) {
         $whereSql .= ( $whereSql == null ? '' : ' AND ') . $whereElement;
      }
      $whereSql = $whereSql != null ? ' WHERE ' . $whereSql : '';
      $max = 700000;  // some big number to avoid catastrophe
      $whereSql .= " ORDER BY ps.status_id ASC LIMIT $max";

      $sql = "SELECT DISTINCT a.p_uuid, a.full_name, a.family_name, a.given_name, a.alternate_names, a.profile_urls, a.expiry_date,
              e.birth_date as dob, e.opt_gender, e.years_old, e.minAge, e.maxAge, 
              f.comments as physical_comments, f.height, f.weight, f.opt_eye_color, f.opt_skin_color, f.opt_hair_color,
              g.contact_value as phone, h.contact_value as mobile,
              i.contact_value as email, j.contact_value AS address, k.contact_value AS zip, e.last_seen,
              e.other_comments, pr.rep_uuid as reporter, ps.status_id, ps.last_updated, pul.update_time,
              ps.creation_time, ps.opt_status as `status`,
              im.url as image_url
        FROM person_update_log pul
        LEFT JOIN person_details e ON e.p_uuid = pul.p_uuid
        LEFT JOIN person_physical f ON f.p_uuid = pul.p_uuid
        LEFT JOIN contact g ON g.p_uuid = pul.p_uuid AND g.opt_contact_type = 'curr'
        LEFT JOIN contact h ON h.p_uuid = pul.p_uuid AND h.opt_contact_type = 'cmob'
        LEFT JOIN contact i ON i.p_uuid = pul.p_uuid AND i.opt_contact_type = 'email'
        LEFT JOIN contact j ON j.p_uuid = pul.p_uuid AND j.opt_contact_type = 'home'
        LEFT JOIN contact k ON k.p_uuid = pul.p_uuid AND k.opt_contact_type = 'zip'
              LEFT JOIN image im ON im.p_uuid = pul.p_uuid
              LEFT JOIN person_status ps ON ps.p_uuid = pul.p_uuid
              LEFT JOIN person_to_report pr ON pr.p_uuid  = pul.p_uuid
              JOIN person_uuid a ON a.p_uuid = pul.p_uuid
              $whereSql ";
      $array = $global['db']->GetAll($sql);
      if ($array === false) {
         $errchk = $global['db']->ErrorMsg();
         pfif_error_log("Error in getPersonsFromDB: " . $errchk);
      }

      return $array;
   }

   // Use threshhold date and number of records to skip.
   private function getNotesFromDb($since_entry_date, $max_results, $skip) {
      global $global, $conf;

      $whereSql = '';
      $where = array();
      if (!empty($since_entry_date)  && $since_entry_date != -1) {
         $where[] = " pn.entry_date >= '$since_entry_date' ";
      }
      $where[] = " (a.expiry_date IS NULL OR a.expiry_date > now()) ";
      $where[] = " a.incident_id = " . $this->incident_id;

      foreach ($where as $whereElement) {
         $whereSql .= ( $whereSql == null ? '' : ' AND ') . $whereElement;
      }
      $whereSql = $whereSql != null ? ' WHERE ' . $whereSql : '';
      // Put a 200 record limit on max_results data feed.
      $max = ($max_results > 200)? 200 : $max_results;
      if ($since_entry_date == -1) {
         // This is a parameterless feed: Return 10 most recent records in reverse chronological order.
         $whereSql .= " ORDER BY pn.entry_date DESC LIMIT 10";
      } else if ($max > 0) {
         // This is a feed with parameters.
         $whereSql .= " ORDER BY pn.entry_date ASC LIMIT $skip,$max";
      }

      $sql = "SELECT * FROM pfif_note pn
        JOIN person_uuid a ON a.p_uuid = pn.p_uuid
              $whereSql ";
      $array = $global['db']->GetAll($sql);
      if ($array === false) {
         $errchk = $global['db']->ErrorMsg();
         pfif_error_log("Error in getNotesFromDB: " . $errchk);
      }

      return $array;
   }


   private function checkForReportedPerson($p_uuid) {
      global $global;
      $sql = "SELECT creation_time FROM person_status WHERE p_uuid = '$p_uuid'";
      $array = $global['db']->GetRow($sql);
      return $array;
   }

   private function checkForReportedNote($note_record_id) {
      global $global;
      $sql = "SELECT note_record_id FROM pfif_note WHERE note_record_id = '$note_record_id'";
      $array = $global['db']->GetRow($sql);
      return $array;
   }

   private function getReportedPerson($p_uuid) {
      global $global;
      $sql = "SELECT a.p_uuid , a.full_name, a.family_name, e.birth_date AS dob, f.comments AS physical_comments,
              f.height, f.weight, g.contact_value AS phone, h.contact_value AS mobile, i.contact_value AS email
    FROM person_uuid a
    LEFT OUTER JOIN person_details e ON e.p_uuid = a.p_uuid
    LEFT OUTER JOIN person_physical f ON f.p_uuid = a.p_uuid
    LEFT OUTER JOIN contact g ON g.p_uuid = a.p_uuid
    AND g.opt_contact_type = 'curr'
    LEFT OUTER JOIN contact h ON h.p_uuid = a.p_uuid
    AND h.opt_contact_type = 'cmob'
    LEFT OUTER JOIN contact i ON i.p_uuid = a.p_uuid
    AND i.opt_contact_type = 'email'
        WHERE a.p_uuid = '$p_uuid' ";

      //print $sql;
      $array = $global['db']->GetRow($sql);
      return $array;
   }

   // Deletes reported person. Removes image files and all relevant DB entries, including
   // reporter. (Does not delete the the reporter if he/she is registered with PL.)
   private function deleteReportedPerson($p_uuid) {
      global $global, $conf;
      $webroot = $conf['approot']."www/";
      // Delete any associated images.
      $sql = "SELECT url, url_thumb FROM image WHERE p_uuid = '$p_uuid'";
      $image_result = $global['db']->GetRow($sql);
      if ($image_result === false) {
         $errchk = $global['db']->ErrorMsg();
         pfif_error_log("Error in deleteReportedPerson for person '$p_uuid': " . $errchk);
      } else if (count($image_result) > 0) {
         // There is an image so delete it and its thumbnail.
         $file = $webroot . $image_result['url'];
         if (!unlink($file)) {
            pfif_error_log("Error in deleteReportedPerson while deleting file '$file'.");
         }
         $thumb = $webroot . $image_result['url_thumb'];
         if ($thumb != $file) {
            if (!unlink($thumb)) {
               pfif_error_log("Error in deleteReportedPerson while deleting thumb '$thumb'.");
            }
         }
      }
      // Delete the person in question.
      $sql = "CALL delete_reported_person('$p_uuid', 0)";  //Don't delete Notes
      $st = $global['db']->Execute($sql);
      if ($st === false) {
         $errchk = $global['db']->ErrorMsg();
         pfif_error_log("Error in deleteReportedPerson during delete: $errchk");
      }
   }

   /**
    * Stores imported PFIF person records in PFIF tables.
    */
   private function taupo_pfif_addpfifperson_commit(Pfif_Person $person) {
      // Gather $person fields for pfif_person table
      $insert_array = array();
      $insert_array['p_uuid'] = $person->person_record_id;
      $insert_array['source_version'] = $this->getVersion();
      if ($this->getSourceReposId() != null) {
         $insert_array['source_repository_id'] = $this->getSourceReposId();
      }
      $insert_array['entry_date'] = date("Y-m-d H:i:s");
      if (!empty($person->expiry_date)) {
         $insert_array['expiry_date'] = $person->expiry_date;
      }
      if (!empty($person->author_name)) {
         $insert_array['author_name'] = $person->author_name;
      }
      if (!empty($person->author_email)) {
         $insert_array['author_email'] = $person->author_email;
      }
      if (!empty($person->author_phone)) {
         $insert_array['author_phone'] = $person->author_phone;
      }
      if (!empty($person->source_name)) {
         $insert_array['source_name'] = $person->source_name;
      }
      // If source date is missing, use entry date (i.e. current date).
      if (!empty($person->source_date)) {
         $insert_array['source_date'] = $person->source_date;
      } else {
         $insert_array['source_date'] = date("Y-m-d H:i:s");
      }
      if (!empty($person->source_url)) {
        $insert_array['source_url'] = $person->source_url;
      }
      if (!empty($person->full_name)) {
         $insert_array['full_name'] = $person->full_name;
      }
      if (!empty($person->given_name)) {
         $insert_array['given_name'] = $person->given_name;
      }
      if (!empty($person->family_name)) {
         $insert_array['family_name'] = $person->family_name;
      }
      if (!empty($person->alternate_names)) {
         $insert_array['alternate_names'] = $person->alternate_names;
      }
      if (!empty($person->profile_urls)) {
         $insert_array['profile_urls'] = $person->profile_urls;
      }
      if (!empty($person->home_city)) {
         $insert_array['home_city'] = $person->home_city;
      }
      if (!empty($person->home_state)) {
         $insert_array['home_state'] = $person->home_state;
      }
      if (!empty($person->home_country)) {
         $insert_array['home_country'] = $person->home_country;
      }
      if (!empty($person->home_neighborhood)) {
         $insert_array['home_neighborhood'] = $person->home_neighborhood;
      }
      if (!empty($person->home_street)) {
         $insert_array['home_street'] = $person->home_street;
      }
      if (!empty($person->home_postal_code)) {
         $insert_array['home_postal_code'] = $person->home_postal_code;
      }
      if (!empty($person->photo_url)) {
         $insert_array['photo_url'] = $person->photo_url;
      }
      if (!empty($person->sex)) {
         $insert_array['sex'] = $person->sex;
      }
      if (!empty($person->date_of_birth)) {
         $insert_array['date_of_birth'] = $person->date_of_birth;
      }
      if (!empty($person->age)) {
         $insert_array['age'] = $person->age;
      }
      if (!empty($person->description)) {
         $insert_array['description'] = $person->description;
      }
      pfif_db_insert($insert_array, 'pfif_person');
   }

   /**
    * Stores all imported PFIF note records in PFIF tables.
    */
   private function taupo_pfif_addpfifnote_commit(Pfif_Note $note) {
      // Gather note fields for pfif_note table.
      $insert_array = array();
      $insert_array['note_record_id'] = $note->note_record_id;
      $insert_array['p_uuid'] = $note->person_record_id;
      $insert_array['source_version'] = $this->getVersion();
      if ($this->getSourceReposId() != null) {
         $insert_array['source_repository_id'] = $this->getSourceReposId();
      }
      if (!empty($note->linked_person_record_id)) {
         $insert_array['linked_person_record_id'] = $note->linked_person_record_id;
      }
      $insert_array['entry_date'] = date("Y-m-d H:i:s");
      if (!empty($note->author_name)) {
         $insert_array['author_name'] = $note->author_name;
      }
      if (!empty($note->author_email)) {
         $insert_array['author_email'] = $note->author_email;
      }
      if (!empty($note->author_phone)) {
         $insert_array['author_phone'] = $note->author_phone;
      }
      // If source date is missing, use entry date.
      if (!empty($note->source_date)) {
         $insert_array['source_date'] = $note->source_date;
      } else {
         $insert_array['source_date'] = $note->entry_date;
      }
      if (!empty($note->author_made_contact)) {
         $insert_array['author_made_contact'] = $note->author_made_contact;
      }
      if (!empty($note->status)) {
         $insert_array['status'] = $note->status;
      }
      if (!empty($note->email_of_found_person)) {
         $insert_array['email_of_found_person'] = $note->email_of_found_person;
      }
      if (!empty($note->phone_of_found_person)) {
         $insert_array['phone_of_found_person'] = $note->phone_of_found_person;
      }
      if (!empty($note->last_known_location)) {
         $insert_array['last_known_location'] = $note->last_known_location;
      }
      if (!empty($note->text)) {
         $insert_array['text'] = $note->text;
      }
      if (!empty($note->photo_url)) {
         $insert_array['photo_url'] = $note->photo_url;
      }
      pfif_db_insert($insert_array, 'pfif_note');
   }

   /**
    * Gets the source PFIF person
    */
   private function getSourcePfif($person) {
      global $global;
      $pfif_person = false; // The return variable

      $sql = "SELECT * FROM pfif_person WHERE p_uuid = '" . $person['p_uuid'] . "'";
      $rs = $global['db']->GetRow($sql);
      if (count($rs) > 0) {
         $pfif_person = new Pfif_Person();
         $pfif_person->person_record_id = $rs['p_uuid'];
         // pfif_error_log("processing person:".$pfif_person->person_record_id);
         $pfif_person->entry_date = $rs['entry_date'];
         $pfif_person->expiry_date = $rs['expiry_date'];
         $pfif_person->author_name = $rs['author_name'];
         $pfif_person->author_email = $rs['author_email'];
         $pfif_person->author_phone = $rs['author_phone'];
         $pfif_person->source_name = $rs['source_name'];
         $pfif_person->source_date = $rs['source_date'];
         $pfif_person->source_url = $rs['source_url'];
         // pfif_error_log("meta-information done ...");
         // Static identifying information
         $pfif_person->full_name = $rs['full_name'];
         $pfif_person->given_name = $rs['given_name'];
         $pfif_person->family_name = $rs['family_name'];
         $pfif_person->alternate_names = $rs['alternate_names'];
         $pfif_person->profile_urls = $rs['profile_urls'];
         $pfif_person->home_city = $rs['home_city'];
         $pfif_person->home_state = $rs['home_state'];
         $pfif_person->home_neighborhood = $rs['home_neighborhood'];
         $pfif_person->home_street = $rs['home_street'];
         $pfif_person->home_country = $rs['home_country'];
         $pfif_person->home_postal_code = $rs['home_postal_code'];
         $pfif_person->photo_url = $rs['photo_url'];
         $pfif_person->description = $rs['description'];
         $pfif_person->age = $rs['age'];
         $pfif_person->date_of_birth = $rs['date_of_birth'];
         $pfif_person->sex = $rs['sex'];
         // pfif_error_log("Static information done ...");
      }
      unset($person);
      return $pfif_person;
   }

   /**
    * Gets the PFIF notes for a person.
    */
   private function getPfifNotes($person) {
      global $global;

      $notes = array();
      $sql = "SELECT * FROM pfif_note WHERE p_uuid = '$person' ORDER BY source_date";
      $rows = $global['db']->GetAll($sql);
      foreach ($rows as $row) {
         $notes[] = $this->createPfifNote($row);
      }
      unset($row);
      return $notes;
   }

   /**
    * Create a Pfif_Note from a query resultset row.
    */
   private function createPfifNote($row) {
         $pfif_note = new Pfif_Note();
         $pfif_note->note_record_id = $row['note_record_id'];
         $pfif_note->person_record_id = $row['p_uuid'];
         $pfif_note->linked_person_record_id = $row['linked_person_record_id'];
         $pfif_note->entry_date = $row['entry_date'];
         $pfif_note->author_name = $row['author_name'];
         $pfif_note->author_email = $row['author_email'];
         $pfif_note->author_phone = $row['author_phone'];
         $pfif_note->source_date = $row['source_date'];
         $pfif_note->email_of_found_person = $row['email_of_found_person'];
         $pfif_note->phone_of_found_person = $row['phone_of_found_person'];
         $pfif_note->status = $row['status'];
         $pfif_note->author_made_contact = $row['author_made_contact'];
         $pfif_note->last_known_location = $row['last_known_location'];
         $pfif_note->text = $row['text'];
         $pfif_note->photo_url = $row['photo_url'];
         return $pfif_note;
   }

   /**
    * Update the expiration date for a person.
    */
   private function taupo_update_expiry_date(Pfif_Person $person) {
      global $global;
      // Update expiration date in person_uuid.
      $sql = "UPDATE person_uuid SET expiry_date='" . $person->expiry_date . "'" .
             " WHERE p_uuid='".$person->person_record_id."'";
      $ret = $global['db']->Execute($sql);
      if ($ret === false) {
         $errchk = $global['db']->ErrorMsg();
         pfif_error_log("Error in taupo_update_expiry performing person_uuid update: " . $errchk);
      }
      // Update expiration date in pfif_person.
      $sql = "UPDATE pfif_person SET expiry_date='" . $person->expiry_date . "'" .
             " WHERE p_uuid='".$person->person_record_id."'";
      $ret = $global['db']->Execute($sql);
      if ($ret === false) {
         $errchk = $global['db']->ErrorMsg();
         pfif_error_log("Error in taupo_update_expiry performing pfif_person update: " . $errchk);
      }
      // Update last_updated_db in person_status so SOLR picks it up right away (PL-472).
      $sql = "UPDATE person_status SET last_updated_db='" . date("Y-m-d H:i:s") . "'" .
             " WHERE p_uuid='".$person->person_record_id."'";
      $ret = $global['db']->Execute($sql);
      if ($ret === false) {
         $errchk = $global['db']->ErrorMsg();
         pfif_error_log("Error in taupo_update_expiry performing person_status update: " . $errchk);
      }
   }
}
