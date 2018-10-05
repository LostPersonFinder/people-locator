<?
/**
 * @name     search class
 * @author   pl@miernicki.com
 * @about    Developed by the U.S. National Library of Medicine
 * @link     https://gitlab.com/tehk/people-locator
 * @license  https://gitlab.com/tehk/people-locator/blob/master/LICENSE
 */
 
class SearchDB {

  public
    $incident,
    $searchTerm,
    $searchMode,
    $pageStart,
    $perPage,
    $sortBy,
    $statusString,
    $missing,
    $alive,
    $injured,
    $deceased,
    $unknown,
    $found,
    $bhgreen,
    $red,
    $yellow,
    $gray,
    $green,
    $black,
    $genderString,
    $male,
    $female,
    $complex,
    $genderUnk,
    $ageString,
    $child,
    $adult,
    $ageUnk,
    $image,
    $since,
    $animal,
    $results,
    $numRowsFound,
    $allCount,
    $lastUpdated,
    $SOLRqueryTime,
    $SOLRfacetResults,
    $SOLRjson;

  private
    $SOLRfq,
    $SOLRroot,
    $SOLRport,
    $SOLRquery,
    $searchUnknown,
    $searchImageUrl,
    $searchImageRegion,
    $searchImageAttrs  = array(),
    $searchByImageResult = array(),
    $searchSource,
    $db,
    $conf;


  /**
  *  Constructor
  *
  * Params:
  * $searchMode = "solr" or "sql"
  * $sStatus = "missing;alive;injured;deceased;unknown;found"
  * $sGender = gender choices imploded
  * $sAge = age choices imploded
  * $imageOnly = only return records with images
  * $sPageControls = "pageStart;perPage;sortBy"
  * $since = everything since specified date in 'Y-m-d' format (used by web services)
  * $animal = 1:return only animals, 0:return only humans; 2:return both
  *
  */
  public function __construct($searchMode, $incident, $searchTerm="", $sStatus = "true;true;true;true;true;true;true", $sGender="true;true;true;true", $sAge="true;true;true", $imageOnly="false", $sHospital="", $sPageControls="0;-1;;true", $source=UNKNOWN_SOURCE, $since="", $animal=2) {
    global $conf;
    $this->incident = $incident;
    $this->searchMode = $searchMode;
    $this->searchSource = $source;
    $this->animal = $animal;
    if (preg_match("/^tmp/", $searchTerm)) {
      // Search-by-image. Format is:
      //
      //    url:bbox:gender:age:distance
      //
      $terms = explode(":", $searchTerm);
      // Required url field beginning with "tmp/".
      $this->searchImageUrl = $terms[0];
      if ((count($terms) > 1) && (strpos($terms[1], ";") !== false)) {
        // Optional face bounding box (x,y;width,height)
        $this->searchImageRegion = "f[".$terms[1]."]";
      }
      if (count($terms) > 2) {
        // Optional gender ('Female' or 'Male').
        $this->searchImageAttrs['gender'] = $terms[2];
      }
      if (count($terms) > 3) {
        // Optional age (years).
        $this->searchImageAttrs['age'] = $terms[3];
      }
      if (count($terms) > 4) {
        // Optional max distance (0 = perfect match, 9.999 = everything).
        $this->searchImageAttrs['distance'] = $terms[4];
      }
    } else {
      // Search string "unknown" means return records with no names (PL-225).
      $this->searchUnknown = (strpos($searchTerm, "unknown") !== false)? true : false;
      // Removed a number of symbols to allow power users to exploit SOLR syntax (PL-265).
      $toReplace = array(",", "\\", "@", "$", "%", "^", "&", "#");
      // Lowercasing needs multi-byte aware function (PL-383). 
      // Lowercasing needed because on wildcard and fuzzy searches, no text analysis is performed on the search word. 
      $this->searchTerm = mb_strtolower(str_replace($toReplace, "", $searchTerm), 'UTF-8');
    }
    $this->setStatusFilters($sStatus);
    $this->setPageControls($sPageControls);
    $this->setGenderFilters($sGender);
    $this->setAgeFilters($sAge);
    $this->setImageFilters($imageOnly);
    // Convert since to UTC (SOLR requires it).
    $this->since = ($since == '')? $since: gmdate('Y-m-d\TH:i:s\Z', strtotime($since));
    $this->hospitalString = '';
    $this->numRowsFound = -1;
    if (strpos($this->sortBy, "full_name") !== false) {
      // Sort on last name first, first name last (PL-237).
      $this->sortBy = str_replace("full_name", "family_name", $this->sortBy) . ",given_name asc";
    }
    // Accommodate age ranges in sort (PL-260).
    $this->sortBy = ($searchMode == "solr") ?
      str_replace("years_old", "max(max(years_old,0),div(sum(max(minAge,0),max(maxAge,0)),2))", $this->sortBy) :
      str_replace("years_old", "greatest(coalesce(years_old,0), (coalesce(minAge,0)+coalesce(maxAge,0))/2)", $this->sortBy);
    if ($searchMode == "sql") {
      // Set sql mode sort-by default.
      if ($this->sortBy == "") {
        $this->sortBy = "updated desc";
      }
      $this->buildFiltersClause();
      if ($this->searchImageUrl) {
        $this->buildSearchTerm();
      }
    } else if ($searchMode == "solr") {
      if ($this->searchImageUrl) {
        $this->sortBy = "score desc";
      }
      $this->SOLRroot = $conf["SOLR_root"];
      $this->SOLRport = $conf["SOLR_port"];
      $this->buildSOLRFilters();
      $this->buildSOLRQuery();
    }
  }


  private function setStatusFilters($sStatus) {
    global $conf;
    $tempArray = explode(";", $sStatus);
    $this->missing   = $tempArray[0];
    $this->alive     = $tempArray[1];
    $this->injured   = $tempArray[2];
    $this->deceased  = $tempArray[3];
    $this->unknown   = $tempArray[4];
    $this->found     = $tempArray[5];
  }


  private function setPageControls($sPageControls) {
    $tempArray = explode(";", $sPageControls);
    $this->pageStart = $tempArray[0];
    $this->perPage   = $tempArray[1];
    $this->sortBy    = $tempArray[2];
  }


  private function setGenderFilters($sGender) {
    $tempArray = explode(";", $sGender);
    //error_log('>>>>'.$sGender);
    $this->complex   = $tempArray[0];
    $this->male      = $tempArray[1];
    $this->female    = $tempArray[2];
    $this->genderUnk = $tempArray[3];
  }


  private function setAgeFilters($sAge) {
    $tempArray = explode(";", $sAge);
    $this->child     = $tempArray[0];
    $this->adult     = $tempArray[1];
    $this->ageUnk    = $tempArray[2];
  }

  private function setImageFilters($imageOnly) {
    $tempArray = explode(";", $imageOnly);
    $this->image     = $tempArray[0];
  }


  private function initDBConnection() {
    global $global;
    $this->db = $global["db"];
  }


  private function buildFiltersClause() {
    global $conf;
    // Prefix statusString with ";" to help SQL procedure disambiguate two green's.
    $this->statusString = ";";
    if ($this->missing == "true")   $this->statusString .= "mis;";
    if ($this->alive == "true")     $this->statusString .= "ali;";
    if ($this->injured == "true")   $this->statusString .= "inj;";
    if ($this->deceased == "true")  $this->statusString .= "dec;";
    if ($this->unknown == "true")   $this->statusString .= "unk;";
    if ($this->found == "true")     $this->statusString .= "fnd;";
    $this->genderString = "";
    if ($this->male == "true")      $this->genderString .= "mal;";
    if ($this->female == "true")    $this->genderString .= "fml;";
    if ($this->complex == "true")   $this->genderString .= "cpx;";
    if ($this->genderUnk == "true") $this->genderString .= "unk;";
    $this->ageString = "";
    if ($this->child == "true")     $this->ageString .= "youth;";
    if ($this->adult == "true")     $this->ageString .= "adult;";
    if ($this->ageUnk == "true")    $this->ageString .= "unknown;";
  }

  public function executeSearch() {
    if ($this->searchMode == "solr" )   $this->executeSOLRQuery();
    elseif ($this->searchMode == "sql") $this->executeSQLQuery();
  }

  public function getAllCount() {
    if ($this->searchMode == "solr" )   $this->getSOLRallCount();
    elseif ($this->searchMode == "sql") $this->getSQLAllCount();
  }

  private function executeSQLQuery() {
    global $conf, $global;
    $this->initDBConnection();
    $this->getSQLAllCount();
    //COMMENTED OUT SINCE NOT CURRENTLY USED IN HOME4.
    //$this->getSQLFacetCount();
    $mysqli = new mysqli( $conf["db_host"], $conf["db_user"], $conf["db_pass"], $conf["db_name"], $conf["db_port"] );
    $procname = "PLSearch";
    $proc = "CALL $procname('$this->searchTerm', '$this->statusString', '$this->genderString', '$this->ageString', '$this->incident', '$this->sortBy', $this->pageStart, $this->perPage, $this->image, '$this->since', $this->animal, @allCount)";
    $res = $mysqli->multi_query( "$proc; SELECT @allCount AS numRowsFound;" );
    $this->numRowsFound = 0;
    if ($res) {
      $results = 0;
      $c = 0;
      do {
        if ($result = $mysqli->store_result()) {
          if ($c == 0) {
            while ($row = $result->fetch_assoc()) {
              $this->results[] = array(
                'p_uuid'             => $row["p_uuid"],
                'full_name'          => htmlspecialchars($row["full_name"]),
                'given_name'         => htmlspecialchars($row["given_name"]),
                'family_name'        => htmlspecialchars($row["family_name"]),
                'opt_status'         => str_replace("\"", "", $row["opt_status"]),
                'imageUrl'           => htmlspecialchars($row["url_thumb"]),
                'imageWidth'         => (int)$row["image_width"],
                'imageHeight'        => (int)$row["image_height"],
                'years_old'          => isset($row["years_old"]) ? $row["years_old"] : null,
                'minAge'             => isset($row["minAge"]) ? $row["minAge"] : null,
                'maxAge'             => isset($row["maxAge"]) ? $row["maxAge"] : null,
                'statusUpdated'      => (string)$row["updated"],
                'last_seen'          => htmlspecialchars($row["last_seen"]),
                'comments'           => nl2br(htmlspecialchars(strip_tags($row["comments"]))),
                'gender'             => $row["opt_gender"]
              );
            }
          } else {
            $row = $result->fetch_assoc();
            $this->numRowsFound = $row["numRowsFound"];
          }
          $result->close();
          if ($mysqli->more_results()) {
            $c += 1;
          }
        }
      } while($mysqli->more_results() && $mysqli->next_result());
    }
    $mysqli->close();
    //error_log(print_r($this->results, true));
  }


  public function getSQLAllCount() {
    global $global;
                
    $sql = "SELECT COUNT(p.p_uuid) AS all_count FROM person_uuid p JOIN incident i ON p.incident_id = i.incident_id" .
      " WHERE i.shortname = '$this->incident'" .
      " AND (p.expiry_date > NOW() OR p.expiry_date is NULL);";
                $res = $global['db']->GetRow($sql);
                $this->allCount = $res['all_count'];
  }


  public function getLastUpdate() {

    if($this->searchMode == "solr") {
      $this->getLastUpdateSOLR();

    } elseif($this->searchMode == "sql") {
      $this->getLastUpdateSQL();
    }
  }


  // Get concatenated last update time and record count (to detect deleted SOLR docs [PL-499]).
  public function getLastUpdateSOLR() {

    $solrQuery = $this->SOLRquery . "&sort=updated desc&rows=1&start=0";
    $solrQuery = str_replace(" ", "%20", $solrQuery);
    $ch = curl_init();
    curl_setopt($ch, CURLOPT_URL, $solrQuery . "&wt=json");
    curl_setopt($ch, CURLOPT_RETURNTRANSFER, 1);
    //curl_setopt($ch, CURLOPT_VERBOSE, 1);
    curl_setopt($ch, CURLOPT_PORT, $this->SOLRport);

    $temp = json_decode(curl_exec($ch));
    curl_close($ch);

    if ($temp->response->numFound == 0) {
      $this->lastUpdated = '0001-01-01 01:01:01@0';
    } else {
      $date = new DateTime($temp->response->docs[0]->updated);
      $this->lastUpdated = $date->format("Y-m-d H:i:s") . "@" . $temp->response->numFound;
    }
    //error_log("Last Updated: " . $this->lastUpdated);
  }


  // Get last update time for SQL.
  private function getLastUpdateSQL() {
    global $conf;

    $mysqli = new mysqli( $conf["db_host"], $conf["db_user"], $conf["db_pass"], $conf["db_name"], $conf["db_port"] );

    $this->pageStart = 0;
    $this->perPage = 1;
    $this->sortBy = 'updated desc';

    // NOTE: The allCount below is not used. (It would just be 1.) If we want to account for deleted records
    // (like SOLR method above), we'd have to issue a separate DB call to get total # of records. 
    $procname = "PLSearch";
    $proc = "CALL $procname('$this->searchTerm', '$this->statusString', '$this->genderString', '$this->ageString', '$this->incident', '$this->sortBy', $this->pageStart, $this->perPage, $this->image, '', @allCount)";

    $res = $mysqli->multi_query("$proc; SELECT @allCount;");

    $this->lastUpdated = '0001-01-01 01:01:01';
    if ($res) {
      $results = 0;
      $c = 0;
      do {
        if ($result = $mysqli->store_result()) {
          if ($c == 0) {
            while ($row = $result->fetch_assoc()) {
              $this->lastUpdated = $row["updated"];
            }
          }
          $result->close();
          if($mysqli->more_results()) {
            $c += 1;
          }
        }
      } while($mysqli->more_results() && $mysqli->next_result());
    }
    $mysqli->close();

    $date = new DateTime($this->lastUpdated);
    $this->lastUpdated = $date->format('Y-m-d H:i:s');
    //error_log("Last Updated: " . $this->lastUpdated);
  }


  public function executeSOLRQuery() {

    $this->getSOLRallCount();  // there has to be a way to include this in the 1 query, still looking
    //COMMENTED OUT SINCE NOT CURRENTLY USED IN HOME4.
    //$this->getSOLRFacetCount(); // (PL-234) any way to avoid doing a separate query?

    $displayParams = '';
    if($this->perPage != "-1") {
      $displayParams .= "&start=" . $this->pageStart . "&rows=" . $this->perPage;
    }

    if ($this->sortBy != "") {
      if ($this->sortBy == "score desc") {
        $displayParams .= "&sort=score desc";
      } else {
        $displayParams .= "&sort=" . $this->sortBy . ",score desc";
      }
    } else {
      $displayParams .= "&sort=updated desc,score desc";
    }

    $this->SOLRquery = str_replace(" ", "%20", $this->SOLRquery);
    $displayParams = str_replace(" ", "%20", $displayParams);

    $ch = curl_init();
    curl_setopt($ch, CURLOPT_URL, $this->SOLRquery . $displayParams . "&wt=json");
    curl_setopt($ch, CURLOPT_RETURNTRANSFER, 1);
    //curl_setopt($ch, CURLOPT_VERBOSE, 1);
    curl_setopt($ch, CURLOPT_PORT, $this->SOLRport);

    $this->SOLRjson = curl_exec($ch);
    curl_close($ch);

    $this->processSOLRjson();
  }


  // ugly but we'd like to have clean json responses.
  private function cleanUpFacets() {
    global $conf;
    $temp["child"]         = $this->SOLRfacetResults->{"ageGroup:youth"} + $this->SOLRfacetResults->{"ageGroup:both"};
    $temp["adult"]         = $this->SOLRfacetResults->{"ageGroup:adult"} + $this->SOLRfacetResults->{"ageGroup:both"};
    $temp["otherAge"]      = $this->SOLRfacetResults->{"ageGroup:unknown"};
    $temp["missing"]    = $this->SOLRfacetResults->{"opt_status:mis"};
    $temp["injured"]    = $this->SOLRfacetResults->{"opt_status:inj"};
    $temp["deceased"]   = $this->SOLRfacetResults->{"opt_status:dec"};
    $temp["unknown"]    = $this->SOLRfacetResults->{"opt_status:unk"};
    $temp["found"]      = $this->SOLRfacetResults->{"opt_status:fnd"};
    $temp["male"]          = $this->SOLRfacetResults->{"opt_gender:mal"};
    $temp["female"]        = $this->SOLRfacetResults->{"opt_gender:fml"};
    $temp["complex"]       = $this->SOLRfacetResults->{"opt_gender:cpx"};
    $temp["otherGender"]   = $this->SOLRfacetResults->{"opt_gender:unk"};
    $temp["hasImage"]         = $this->SOLRfacetResults->{"url_thumb:[* TO *]"};
    $this->SOLRfacetResults = $temp;
  }


  private function processSOLRjson() {
    global $conf;
    $tempObject = json_decode($this->SOLRjson);
    // set rows found
    $this->numRowsFound = $tempObject->response->numFound;
    // get query time
    $this->SOLRqueryTime = $tempObject->responseHeader->QTime;
    // If this is an image search sorted by similarity, verify results order using original query (PL-977).
    $similarity = false;
    if ($this->searchImageUrl && ($tempObject->responseHeader->params->sort == "score desc")) {
      // This is a similarity sort. Get p_uuids from original FM query result.
      $p_uuids = array_keys($this->searchByImageResult);
      //error_log("p_uuids used in SOLR query " . print_r($this->searchByImageResult,true));
      $similarity = true;
    }
    $last_key = -1;
    foreach ($tempObject->response->docs as $doc) {
      if ($similarity) {
        // Get the query key.
        $key = array_search($doc->p_uuid, $p_uuids);
        if ($last_key > $key) {
          error_log("Incorrect order in search results for p_uuid: ".$doc->p_uuid);
        } else {
          $last_key = $key;
        }
        
      }
      $this->results[] = array(
        'p_uuid' => $doc->p_uuid,
        'full_name'           => isset($doc->full_name)        ? htmlspecialchars($doc->full_name) : null,
        'given_name'          => isset($doc->given_name)       ? htmlspecialchars($doc->given_name) : null,
        'family_name'         => isset($doc->family_name)      ? htmlspecialchars($doc->family_name) : null,
        'opt_status'          => isset($doc->opt_status)       ? $doc->opt_status : null,
        'imageUrl'            => isset($doc->url_thumb)        ? $doc->url_thumb : null,
        'imageWidth'          => isset($doc->image_width)      ? $doc->image_width : null,
        'imageHeight'         => isset($doc->image_height)     ? $doc->image_height : null,
        'years_old'           => isset($doc->years_old)        ? $doc->years_old : null,
        'minAge'              => isset($doc->minAge)           ? $doc->minAge : null,
        'maxAge'              => isset($doc->maxAge)           ? $doc->maxAge : null,
        'id'                  => isset($doc->personId)         ? $doc->personId : null,
        'statusUpdated'       => isset($doc->updated)          ? str_replace('Z', '', $doc->updated) : null,
        'statusTriage'        => isset($doc->triage_category)  ? $doc->triage_category : null,
        'peds'                => isset($doc->peds)             ? $doc->peds : null,
        'orgName'             => isset($doc->orgName)          ? $doc->orgName : null,
        'last_seen'           => isset($doc->last_seen)        ? htmlspecialchars($doc->last_seen) : null,
        'comments'            => isset($doc->comments)         ? nl2br(strip_tags(htmlspecialchars($doc->comments))) : null,
        'gender'              => isset($doc->opt_gender)       ? $doc->opt_gender : null,
        'searchImageDistance' => $this->searchImageDistance($doc->p_uuid)
      );
    }
  }

  private function buildSOLRQuery() {
    global $global;
    if ($this->searchImageUrl) {
      // Get incident id from shortname.
      $sql = "SELECT incident_id from incident where shortname='$this->incident'";
      $res = $global['db']->GetRow($sql);
      $incident_id = $res['incident_id'];
      try {
        $this->searchByImageResult = taupo_facematch_query($this->searchImageUrl, $incident_id, $this->searchSource, $this->searchImageAttrs, $this->searchImageRegion);
        $this->searchTerm = $this->prep_for_solr($this->searchByImageResult);
      } catch (Exception $e) {
        //error_log("Search Facematch query error: " . $e->getMessage());
        $this->searchTerm = 'p_uuid:none';
      }
      $queryType = "";  // use default
    } else {
      $this->searchTerm = ($this->searchTerm == "" || $this->searchTerm == "*") ? "*:*" : $this->searchTerm;
      $queryType = "&qt=edismax";
    }
    $this->SOLRquery =  $this->SOLRroot."select?fl=*,score"
                  .$queryType
                  ."&q=".trim(urlencode($this->searchTerm))
                  .$this->SOLRfq;
  }

  private function buildSearchTerm() {
    global $global;
    // Get incident id from shortname.
    $sql = "SELECT incident_id from incident where shortname='$this->incident'";
    $res = $global['db']->GetRow($sql);
    $incident_id = $res['incident_id'];
    try {
      $this->searchByImageResult = taupo_facematch_query($this->searchImageUrl, $incident_id, $this->searchSource, $this->searchImageAttrs, $this->searchImageRegion);
      $this->searchTerm = $this->prep_for_sql($this->searchByImageResult);
    } catch (Exception $e) {
      //error_log("Search Facematch query error: " . $e->getMessage());
      $this->searchTerm = 'p_uuid:none';
    }
  }


  private function buildSOLRFilters() {
    global $conf, $global;
    // opt_status filters
    $temp = $base = "&fq=opt_status:(*";
    if ($this->missing != "true")  $temp .= " -mis";
    if ($this->alive != "true")    $temp .= " -ali";
    if ($this->injured != "true")  $temp .= " -inj";
    if ($this->deceased != "true") $temp .= " -dec";
    if ($this->unknown != "true")  $temp .= " -unk";
    if ($this->found != "true")    $temp .= " -fnd";
    if (strcmp($temp, $base) != 0) {
      $this->SOLRfq .= $temp . ')';
    }
    // opt_gender filters
    $temp = $base = "&fq=opt_gender:(*";
    if($this->male != "true")      $temp .= " -mal";
    if($this->female != "true")    $temp .= " -fml";
    if($this->complex != "true")   $temp .= " -cpx";
    if($this->genderUnk != "true") $temp .= " -unk";
    if (strcmp($temp, $base) != 0) {
      $this->SOLRfq .= $temp . ')';
    }
    // years_old filters
    $temp = $base = "&fq=ageGroup:(*";
    if($this->child != "true")  $temp .= " -youth ";
    if($this->adult != "true")  $temp .= " -adult ";
    if($this->child != "true" && $this->adult != "true") $temp .= " -both ";
    if($this->ageUnk != "true") $temp .= " -unknown";
    if (strcmp($temp, $base) != 0) {
      $this->SOLRfq .= $temp . ')';
    }
    // image only filter
    if($this->image == "true") {
      $this->SOLRfq .= "&fq=url_thumb:[* TO *]";
    }
    //incident shortname filter (always applied)
    $this->SOLRfq .= "&fq=shortname:" . $this->incident;
    // NULL full_name filter if searching for "unknown"
    if ($this->searchUnknown) {
      $this->searchTerm = '';
      $this->SOLRfq .= "&fq=-full_name:[* TO *]";
    }
    // since filter
    if ($this->since != "") {
      $this->SOLRfq .= "&fq=updated:[".$this->since." TO NOW]";
    }
    // animal filter
    if ($this->animal != 2) {
      $this->SOLRfq .= "&fq=animal:".$this->animal;
    }
  }


  private function getSOLRallCount() {

    //$tmpSOLRquery = $this->SOLRroot . "select?rows=0&q=*:*&fq=shortname:".$this->incident."&fq=-expiry_date:[*%20TO%20NOW]";
    $tmpSOLRquery = $this->SOLRroot . "select?rows=0&q=*:*&fq=shortname:".$this->incident;
    $ch = curl_init();
    curl_setopt($ch, CURLOPT_URL, $tmpSOLRquery . "&wt=json"); // ensure the json version is called
    curl_setopt($ch, CURLOPT_RETURNTRANSFER, 1);
    //curl_setopt($ch, CURLOPT_VERBOSE, 1);
    curl_setopt($ch, CURLOPT_PORT, $this->SOLRport);

    $tempSOLRjson = json_decode(curl_exec($ch));
    curl_close($ch);

    $this->allCount = $tempSOLRjson->response->numFound;
  }


  private function getSQLFacetCount() {
    global $global;
    $db = $global['db'];
    // Temporarily hardcode categories.
    // Initialize facet counts so there are zeros for missing counts.
    $facets = ['green'=>0,'bhgreen'=>0,'yellow'=>0,'red'=>0,'gray'=>0,'black'=>0,'unknown'=>0];
    // Get incident id from shortname.
    $sql = "
      SELECT incident_id
      from incident
      where shortname = '$this->incident';
    ";
    $res = $db->GetRow($sql);
    $incident_id = $res['incident_id'];
    // Set facet counts.
    $sql = "
      SELECT COUNT(*) AS count
      FROM person_uuid pu
      WHERE pu.incident_id = '$incident_id'
      AND (
        pu.expiry_date > NOW() 
        OR pu.expiry_date is NULL
      );
    ";
    $res = $db->Execute($sql);
    while ($row = $res->FetchRow()) {
      $facets[] = $row['count'];
    }
    $this->SOLRfacetResults = $facets;
  }

  private function getSOLRFacetCount() {
      if ($this->searchImageUrl) {
         $queryType = "";
      } else {
         $queryType = "&qt=edismax";
      }

    $solrQuery =
      $this->SOLRroot
      . "select?rows=0".$queryType."&q="
      . trim(urlencode($this->searchTerm))
      . "&fq=shortname:" . $this->incident
      . (strpos($this->SOLRfq, "-full_name")? "&fq=-full_name:[*%20TO%20*]" : '')
      . "&facet=true"
      . "&facet.query=ageGroup:youth&facet.query=ageGroup:adult&facet.query=ageGroup:unknown&facet.query=ageGroup:both"
      . "&facet.query=opt_status:mis&facet.query=opt_status:ali&facet.query=opt_status:inj&facet.query=opt_status:dec&facet.query=opt_status:unk&facet.query=opt_status:fnd"
      . "&facet.query=triage_category:%22BH%20Green%22&facet.query=triage_category:Yellow&facet.query=triage_category:Red"
        ."&facet.query=triage_category:Gray&facet.query=triage_category:Green&facet.query=triage_category:Black&facet.query=triage_category:Unknown"
      . "&facet.query=opt_gender:mal&facet.query=opt_gender:fml&facet.query=opt_gender:unk&facet.query=opt_gender:cpx"
      . "&facet.query=url_thumb:[*%20TO%20*]";

    $ch = curl_init();
    curl_setopt($ch, CURLOPT_URL, $solrQuery . "&wt=json"); 
    curl_setopt($ch, CURLOPT_RETURNTRANSFER, 1);
    //curl_setopt($ch, CURLOPT_VERBOSE, 1);
    curl_setopt($ch, CURLOPT_PORT, $this->SOLRport);

    $tempSOLRjson = json_decode(curl_exec($ch));
    curl_close($ch);

    $this->SOLRfacetResults = $tempSOLRjson->facet_counts->facet_queries;
    $this->cleanUpFacets();
  }

  // Insert fuzzy search operator after each search term (PL-264).
  // NOTE: This is obsolete since we've switched to partial string matching.
  private function fuzzify($searchTerm) {

    // Leave really short strings as is.
    if (strlen($searchTerm) <= 2) {
      return $searchTerm;
    }

    // If search terms start and end with a double-quote, do nothing.
    if ($searchTerm[0] == '"' && substr($searchTerm, -1) == '"') {
                        return $searchTerm;
     }

    // If search term contains a ":" (field-based search), do nothing.
    if (strpos($searchTerm, ":") !== false) {
                        return $searchTerm;
     }

    $tempTerm = '';

    // Take care not to fuzzify boolean terms, or terms w/in double-quoted phrases.
    // SOLR does the right thing if you fuzzify nonsensical stuff (e.g. single quoted term).
    $tempArray = explode(" ", $searchTerm);
                $inQuote = false;

    foreach($tempArray as $token) {
      if(strcasecmp($token, 'and') == 0 || strcasecmp($token, 'or') == 0) {
        $tempTerm .= $token . " ";
      } else {
        if(!$inQuote) {
          // Fixme: Right paren? Place ~ inside it.
                      $tempTerm .= $token . "~0.8 ";
        } else {
          $tempTerm .= $token . " ";
        }
      }

      // Process quote flag.
      if($token[0] == '"' && substr($token, -1) != '"') {
        // First character (but not last character) is a quote.
        $inQuote = true;
      } else if (substr($token, -1) == '"') {
        // Last character is a quote.
        $inQuote = false;
      }
    }
    return trim($tempTerm);
  }

  // Prep p_uuids from search by image results for SOLR search.
  private function prep_for_solr($result) {
    $p_uuids = array_keys($result);
    $count = count($p_uuids);
    if ($count == 0) {
      $query = 'p_uuid:none';
    } else {
      $query = 'p_uuid:(';
      for ($i = 0, $boost = $count; $i < $count; ++$i, --$boost) {
        // Add boosts to maintain sort order.
        $query .= $p_uuids[$i] . '^' . $boost;
        if ($i < $count-1) {
          $query .= ' ';
        }
      }
      $query .= ')';
    }
    return $query;
  }

  // Prep p_uuids from search by image results for SQL search.
  // NOTE: p_uuid order controls display order for sort by "image similarity".
  private function prep_for_sql($result) {
    $p_uuids = array_values(array_unique(array_keys($result)));
    $count = count($p_uuids);
    if ($count == 0) {
      $query = 'p_uuid:none';
    } else {
      $query = '';
      for ($i = 0; $i < $count; ++$i) {
        $query .= $p_uuids[$i];
        if ($i < $count-1) {
          $query .= ',';
        }
      }
    }
    return $query;
  }

  // Get distance from search image if this is a search by image query.
  private function searchImageDistance($p_uuid) {
    global $conf;
    $distance = null;
    if($conf['img_distance']) {
      if(array_key_exists($p_uuid, $this->searchByImageResult)) {
         $distance = $this->searchByImageResult[$p_uuid];
      }
    }
    return $distance;
  }
}


// testing
// $search = new SearchDB("sql", "sendai2011", "Mike", "true;true;true;true;true;true", "true;true;true", "true;true;true", "true;true;true", "0;25;last_updated;true");
// $search->executeSearch();
// echo "<br />";
// echo count($search->results);

// $search2 = new SearchDB("solr", "sendai2011", "Mi*");
// $search2->executeSearch();
// echo count($search2->results);
// $search->getLastUpdateSOLR();

// echo json_encode($search->results);
// echo json_encode($search2->results);
