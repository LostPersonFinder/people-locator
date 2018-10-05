<?
/**
 * @name     Facematch Library
 * @author   pl@miernicki.com
 * @about    Developed by the U.S. National Library of Medicine
 * @link     https://gitlab.com/tehk/people-locator
 * @license  https://gitlab.com/tehk/people-locator/blob/master/LICENSE
 */

// Get face locations from Facefinder.
function taupo_facematch_find_faces($image_url, $incident_id=1) {
  global $global,$conf;
  $uploadDir = $conf['approot']."www/tmp/fm_cache/";
  $challengeDir = $conf['approot']."www/tmp/challenge_cache/";
  $http = isset($_SERVER['HTTPS'])? 'https://' : 'http://';
  $full_image_url = $http . $conf['base_uuid'] . $image_url;
  if(isset($conf['facematch_version']) && $conf['facematch_version'] == '2') {
    $key = urlencode($conf['facematch_key']);
    $url = $conf['facematch_server']."/ffind/getfaces?key=$key&url=$full_image_url";
    fm_log("FaceFinder: request=$url");
    $time0 = microtime(true);
    $ch = curl_init();
    curl_setopt($ch, CURLOPT_URL, $url);
    curl_setopt($ch, CURLOPT_PORT, $conf['facematch_port']);
    curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
    $time1 = microtime(true);
    $data = curl_exec($ch);
    $time2 = microtime(true);
    curl_close($ch);
    if ($data) {
      $decode = json_decode($data);
      fm_log("FaceFinder: result=".print_r($decode, true));
      if ($decode->statusCode != 1) {
        $message = "FaceFinder: ".$decode->statusMessage;
        $isError = ($decode->statusCode != NO_FACES_IN_IMAGE)? true:false;
        if ($isError) {
          fm_log($message, true);
        }
        throw new RuntimeException($message);
      }
    } else {
      // FaceMatch down.
      $message = "FaceFinder: No response from FaceMatch!";
      fm_log($message, true);
      throw new RuntimeException($message);
    }
    $setup = round(($time1-$time0), 2);
    $call = round(($time2-$time1),2);
    // Log to file.
    fm_log("FaceFinder: setup=$setup secs, call=$call secs");

    $bboxes = [];
    if (!empty($decode->faceRegions)) {
      $regions = str_replace(';', ',', $decode->faceRegions);
      $i = 0;
      foreach ($regions as $region) {
        $regex = '/{\[([^\]]*)\]/';
        if (preg_match($regex, $region, $box)) {
          $bboxes[$i++] = explode(",", $box[1]);
          // Return just one face for now.
          break;
        }
      }
    }
    return $bboxes;
  } else {
    $params = ['appKey'=>$conf['facematch_key'],
      'eventID' =>$incident_id,
      'url'=>$full_image_url,
      'queryRegions'=>'',
      'displayRegions'=>'',
      'inflatePct'=>'',
      'errorString'=>'',
      'webServiceMilliseconds'=>'',
      'imageCoreMilliseconds'=>'',
      'comMilliseconds'=>'',
      'urlFetchMilliseconds'=>'',
      'requestsOutstanding'=>'',
      'useGPU'=>true];
    fm_log("FaceFinder: request=".print_r($params,true));
    $wsdl = $conf['facefinder_service'].'?wsdl';
    try {
      $time0 = microtime(true);
      $client = @new SoapClient($wsdl);
      $time1 = microtime(true);
      $result = $client->getFacesForUI($params);
      $time2 = microtime(true);
    } catch (SoapFault $fault) {
      $error = "FaceFinder SOAP Fault: faultcode: {$fault->faultcode}, faultstring: {$fault->faultstring}";
      fm_log($error, true);
      throw new RuntimeException($error);
    }
    fm_log("FaceFinder: result=".print_r($result, 1));
    if ($result->errorString != 'SUCCESS') {
      $error = "FaceFinder: {$result->errorString}";
      $isError = ($result->errorCode != NO_FACE_DETECTED)? true:false;
      if (!$isError) {
        // No face detected, so save it as a challenge image.
        // Add identifying information where possible to challenge filename.
        $name = isset($_SESSION, $_SESSION['user'])? "_".$_SESSION['user']:"";
        $ip = get_client_ip();
        $internal = is_internal($ip)? "INTERNAL":$ip;
        $filename = date('m-d-Y-His')."_".$internal.$name;
        rename($uploadDir.basename($image_url), $challengeDir.$filename);
      }
      if ($isError) {
        fm_log($error, $isError);
      }
      throw new RuntimeException($error);
    }
    fm_log("FaceFinder: connection setup=".($time1-$time0)." secs, call=".($time2-$time1)." secs");
    $bboxes = [];
    if (!empty($result->queryRegions)) {
      $regions = str_replace(';', ',', $result->queryRegions);
      $regions_array = explode("\t", substr($regions, 1));   // skip leading tab
      $i = 0;
      foreach ($regions_array as $region) {
        $regex = '/{\[([^\]]*)\]/';
        if (preg_match($regex, $region, $box)) {
          $bboxes[$i++] = explode(",", $box[1]);
          // Return just one face for now.
          break;
        }
      }
    }
    return $bboxes;
  }
}

/**
 * Submit the image to FaceMatch for indexing.
 *
 * @param string $image_url
 * @param string $incident_id
 * @param int $source
 * @param array $attrs demographic data
 * @param string $region optional face boundary in 'f[left,top;width,height]' format
 * @return void
 * @throws RuntimeException on FaceMatch error
 */

function taupo_facematch_ingest($image_url, $incident_id, $source, $attrs=array(), $region=NULL) {
  global $conf, $global;
  $challengeDir = $conf['approot']."www/tmp/challenge_cache/";
  $http = isset($_SERVER['HTTPS'])? 'https://' : 'http://';
  $full_image_url = $http . $conf['base_uuid'] . $image_url;
  if(isset($conf['facematch_version']) && $conf['facematch_version'] == '2') {
    $key = urlencode($conf['facematch_key']);
    $extent = $incident_id; 
    $tag = extractTag($image_url);
    $reg = ($region)? "&region=".urlencode($region) : '';
    $gender = isset($attrs['gender'])? "&gender=".$attrs['gender'] : '';
    $age = isset($attrs['age'])? "&age=".$attrs['age'] : '';
    $url = $conf['facematch_server']."/reg/ingest?key=$key&extent=$extent&url=$full_image_url&tag=$tag$reg$age$gender";
    fm_log("FaceMatch Ingest: request=$url");
    $time0 = microtime(true);
    $ch = curl_init();
    curl_setopt($ch, CURLOPT_URL, $url);
    curl_setopt($ch, CURLOPT_PORT, $conf['facematch_port']);
    curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
    $time1 = microtime(true);
    $data = curl_exec($ch);
    $time2 = microtime(true);
    curl_close($ch);
    if ($data) {
      $decode = json_decode($data);
      fm_log("FaceMatch Ingest: result=".print_r($decode, true));
      if ($decode->statusCode != 1) {
        $message = "FaceMatch Ingest: ".$decode->statusMessage;
        $isError = ($decode->statusCode != NO_FACES_IN_IMAGE)? true:false;
        if (!$isError && (!array_key_exists('animal', $attrs) || !$attrs['animal']) && !isCommandLineInterface()) {
          // No face detected, so save it as a challenge image.
          // Add identifying information where possible to challenge filename.
          $name = isset($_SESSION, $_SESSION['user'])? "_".$_SESSION['user']:"";
          $ip = get_client_ip();
          $internal = is_internal($ip)? "INTERNAL":$ip;
          $filename = date('m-d-Y-His')."_".$internal.$name;
          copy($conf['approot']."www/".$image_url, $challengeDir.$filename);
        }
        if ($isError) {
          fm_log($message, true);
        }
        throw new RuntimeException($message);
      }
    } else {
      // FaceMatch down.
      $message = "FaceMatch Ingest: No response from FaceMatch!";
      fm_log($message, true);
      throw new RuntimeException($message);
    }
    $setup = round(($time1-$time0), 2);
    $call = round(($time2-$time1),2);
    $total = round(($time2-$time0),2);
    // Update FM stats.
    fm_db_log(INGEST_SERVICE, $source, $incident_id, $total);
    // Log to file.
    fm_log("FaceMatch Ingest: setup=$setup secs, call=$call secs");
  } else {
    $reg = ($region)? "\t".$region : '';
    $gender = isset($attrs['gender'])? $attrs['gender'] : 'GenderUnknown';
    $age = isset($attrs['age'])? $attrs['age'] : -1;
    $params = ['appKey'=>$conf['facematch_key'],
      'eventID' =>$incident_id,
      'url'=>$full_image_url,
      'gender'=>$gender,
      'age'=>$age,
      'IDRegs'=>extractTag($image_url).$reg,
      'errorString'=>'',
      'webServiceMilliseconds'=>'',
      'imageCoreMilliseconds'=>'',
      'comMilliseconds'=>'',
      'urlFetchMilliseconds'=>'',
      'requestsOutstanding'=>''];
    fm_log("FaceMatch Ingest: request=".print_r($params,true));
    $wsdl = $conf['facematch_service'].'?wsdl';
    try {
      $time0 = microtime(true);
      $client = @new SoapClient($wsdl);
      $time1 = microtime(true);
      $result = $client->ingest($params);
      $time2 = microtime(true);
    } catch (SoapFault $fault) {
      $error = "FaceMatch Ingest: SOAP fault: faultcode: {$fault->faultcode}, faultstring: {$fault->faultstring}";
      fm_log($error, true);
      throw new RuntimeException($error);
    }
    fm_log("FaceMatch Ingest: result=".print_r($result, 1));
    if ($result->errorString != 'SUCCESS') {
      $error = "Ingest: {$result->errorString}";
      $isError = ($result->errorCode != NO_FACE_DETECTED)? true:false;
      if (!$isError && (!array_key_exists('animal', $attrs) || !$attrs['animal'])) {
        // No face detected, so save it as a challenge image.
        // Add identifying information where possible to challenge filename.
        $name = isset($_SESSION, $_SESSION['user'])? "_".$_SESSION['user']:"";
        $ip = get_client_ip();
        $internal = is_internal($ip)? "INTERNAL":$ip;
        $filename = date('m-d-Y-His')."_".$internal.$name;
        copy($conf['approot']."www/".$image_url, $challengeDir.$filename);
      }
      if ($isError) {
        fm_log($error, true);
      }
      throw new RuntimeException($error);
    }
    $setup = round(($time1-$time0), 2);
    $call = round(($time2-$time1),2);
    $total = round(($time2-$time0),2);
    // Update FM stats.
    fm_db_log(INGEST_SERVICE, $source, $incident_id, $total);
    // Log to file.
    fm_log("FaceMatch Ingest: setup=$setup secs, call=$call secs");
  }
}

/**
 * Have FaceMatch re-index an image. (Necessary in cases where attributes have changed.)
 *
 * @param string $image_url
 * @param string $incident_id
 * @param array $attrs demographic data
 * @return void
 * @throws RuntimeException on FaceMatch error
 */
function taupo_facematch_reingest($image_url, $incident_id, $attrs=array()) {
  fm_log("FaceMatch Re-Ingest: Start"); 
  try {
    taupo_facematch_remove($image_url, $incident_id, UNKNOWN_SOURCE);
    taupo_facematch_ingest($image_url, $incident_id, UNKNOWN_SOURCE, $attrs);
  } catch (Exception $e) {
    fm_log("Re-Ingest: ".$e->getMessage(), false);
    throw new RuntimeException($error);
  }
  fm_log("FaceMatch Re-Ingest: Done"); 
}

/**
 * Ask Facematch for matches with this image in this incident. Supply an optional set of attributes and a
 * face boundary in 'f[left,top;width,height]' format. Returns an array with p_uuids as keys and their 
 * distances as values.
 *
 * @param string $image_url
 * @param string $incident_id
 * @param int $source
 * @param array $attrs demographic data
 * @param string $region optional face boundary in 'f[left,top;width,height]' format
 * @return void
 * @throws RuntimeException on FaceMatch error
 */
function taupo_facematch_query($image_url, $incident_id, $source, $attrs=array(), $region=NULL) {
  global $conf;
  $http = isset($_SERVER['HTTPS'])? 'https://' : 'http://';
  $full_image_url = $http . $conf['base_uuid'] . $image_url;
  if(isset($conf['facematch_version']) && $conf['facematch_version'] == '2') {
    $key = urlencode($conf['facematch_key']);
    $distance = isset($attrs['distance'])? $attrs['distance'] : $conf['facematch_max_distance'];
    $max_matches = isset($attrs['max_matches'])? $attrs['max_matches'] : $conf['facematch_max_matches'];
    $extent = $incident_id;
    $reg = ($region)? "&region=".urlencode($region) : '';
    $gender = isset($attrs['gender'])? "&gender=".$attrs['gender'] : '';
    $age = isset($attrs['age'])? "&age=".$attrs['age'] : '';
    $url = $conf['facematch_server']."/reg/query?key=$key&extent=$extent&maxmatches=$max_matches$reg&tolerance=$distance&url=$full_image_url$age$gender";
    fm_log("FaceMatch Query: request=$url");
    $time0 = microtime(true);
    $ch = curl_init();
    curl_setopt($ch, CURLOPT_URL, $url);
    curl_setopt($ch, CURLOPT_PORT, $conf['facematch_port']);
    curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
    $time1 = microtime(true);
    $data = curl_exec($ch);
    $time2 = microtime(true);
    curl_close($ch);
    if ($data) {
      $decode = json_decode($data);
      fm_log("FaceMatch Query: result=".print_r($decode, true));
      if ($decode->statusCode != 1) {
        $message = "FaceMatch Query: ".$decode->statusMessage;
        $isError = ($decode->statusCode != NO_FACES_IN_IMAGE)? true:false;
        if ($isError) {
          fm_log($message, true);
        }
        throw new RuntimeException($message);
      }
    } else {
      // FaceMatch down.
      $message = "FaceMatch Query: No response from FaceMatch!";
      fm_log($message, true);
      throw new RuntimeException($message);
    }
    $setup = round(($time1-$time0), 2);
    $call = round(($time2-$time1),2);
    $total = round(($time2-$time0),2);
    // Update FM stats.
    fm_db_log(QUERY_SERVICE, $source, $incident_id, $total);
    // Log to file.
    fm_log("FaceMatch Query: setup=$setup secs, call=$call secs");

    $rets = array();
    // Get all matching tags for all regions. Since max_matches is *per region*, we
    // could have many more matches than that when there are multiple regions. So we
    // arbitrarily cut things off at max_matches overall. 
    foreach ($decode->allMatches as $match) {
      foreach ($match->regionMatches as $regionMatch) {
        // Get p_uuid from filename.
        $temp = explode("_", $regionMatch->tag, 2);
        $p_uuid = str_replace(array("SLASH","TILDE"), array("/","~"), $temp[0]);
        if (!array_key_exists($p_uuid, $rets)) {
          $rets[$p_uuid] = $regionMatch->distance;
        }
        if (count($rets) >= $max_matches) break 2;
      }
    }
    fm_log("FaceMatch Query: return p_uuids=".print_r($rets, true));
    return $rets;
  } else {
    $reg = ($region)? "\t".$region : '';
    $gender = isset($attrs['gender'])? $attrs['gender'] : 'GenderUnknown';
    $age = isset($attrs['age'])? $attrs['age'] : -1;
    $distance = isset($attrs['distance'])? $attrs['distance'] : $conf['facematch_max_distance'];
    $max_matches = isset($attrs['max_matches'])? $attrs['max_matches'] : $conf['facematch_max_matches'];
    // Get matching image names from FaceMatch.
    $params = ['appKey'=>$conf['facematch_key'],
                  'eventID'=>$incident_id,
                  'urlRegs'=>$full_image_url.$reg,
                  'result'=>'',
                  'tolerance'=>$distance,
                  'gender'=>$gender,
                  'age'=>$age,
                  'errorString'=>'',
                  'matches'=>'',
                  'maxMatchesRequested'=>$max_matches,
                  'webServiceMilliseconds'=>'',
                  'imageCoreMilliseconds'=>'',
                  'comMilliseconds'=>'',
                  'urlFetchMilliseconds'=>'',
                  'requestsOutstanding'=>''];
    fm_log("FaceMatch Query: request=".print_r($params,true));
    $wsdl = $conf['facematch_service'].'?wsdl';
    try {
      $time0 = microtime(true);
      $client = @new SoapClient($wsdl);
      $time1 = microtime(true);
      $result = $client->query($params);
      $time2 = microtime(true);
    } catch (SoapFault $fault) {
      $error = "FaceMatch Query: SOAP Fault: faultcode: {$fault->faultcode}, faultstring: {$fault->faultstring}";
      fm_log($error, true);
      throw new RuntimeException($error); 
    }
    fm_log("FaceMatch Query: result=".print_r($result, 1));
    if ($result->errorString != 'SUCCESS') {
      $error = "Facematch Query: {$result->errorString}";
      $isError = ($result->errorCode == NO_FACE_DETECTED)? false:true;
      $isError = ($result->errorCode == EVENT_ID_NOT_FOUND)? false:$isError;
      if ($isError) fm_log($error, $isError);
      throw new RuntimeException($error);
    }
    $setup = round(($time1-$time0), 2);
    $call = round(($time2-$time1),2);
    $total = round(($time2-$time0),2);
    // Update FM stats.
    fm_db_log(QUERY_SERVICE, $source, $incident_id, $total);
    // Log to file.
    fm_log("FaceMatch Query: setup=$setup secs, call=$call secs");
  
    $p_uuids = array();
    $distances = array();
    if ($result->matches > 0) {
      // Get p_uuids (from the image tags) and distances.
      preg_match_all("/(.*?)\t(.*?)__/", $result->result, $matches);
      // Filter duplicate p_uuids while preserving original keys.
      $ids = array_unique($matches[2]);
      // Use the keys to pull out their associated distances.
      $p_uuid_keys = array_keys($ids);
      foreach ($p_uuid_keys as $key) {
        $distances[] = $matches[1][$key];
      }
      // Remove SLASH (and possible TILDE) from p_uuid and reset p_uuid keys.
      foreach ($ids as $id) {
        $p_uuids[] = str_replace(array("SLASH","TILDE"), array("/","~"), $id);
      }
    }
    fm_log("FaceMatch Query: return p_uuids=".print_r($p_uuids, true));
    return array_combine($p_uuids,$distances);
  }
}

/**
 * Have FaceMatch remove this image.
 *
 * @param string $image_url
 * @param string $incident_id
 * @param int $source
 * @return void
 * @throws RuntimeException on FaceMatch error
 */
function taupo_facematch_remove($image_url, $incident_id, $source) {
  global $conf;
  if(isset($conf['facematch_version']) && $conf['facematch_version'] == '2') {
    $key = urlencode($conf['facematch_key']);
    $extent = $incident_id;
    $tag = extractTag($image_url);
    $url = $conf['facematch_server']."/reg/remove?key=$key&extent=$extent&tag=$tag";
    fm_log("FaceMatch Remove: request=$url");
    $time0 = microtime(true);
    $ch = curl_init();
    curl_setopt($ch, CURLOPT_URL, $url);
    curl_setopt($ch, CURLOPT_PORT, $conf['facematch_port']);
    curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
    $time1 = microtime(true);
    $data = curl_exec($ch);
    $time2 = microtime(true);
    curl_close($ch);
    if ($data) {
      $decode = json_decode($data);
      fm_log("FaceMatch Remove: result=".print_r($decode, true));
      if ($decode->statusCode != 1) {
        $message = "FaceMatch Remove: ".$decode->statusMessage;
        fm_log($message, true);
        throw new RuntimeException($message);
      }
    } else {
      // FaceMatch down.
      $message = "FaceMatch Ingest: No response from FaceMatch!";
      fm_log($message, true);
      throw new RuntimeException($message);
    }
    $setup = round(($time1-$time0), 2);
    $call = round(($time2-$time1),2);
    $total = round(($time2-$time0),2);
    // Update FM stats.
    fm_db_log(REMOVE_SERVICE, $source, $incident_id, $total);
    // Log to file.
    fm_log("FaceMatch Remove: setup=$setup secs, call=$call secs");
  } else {
    $params = ['appKey'=>$conf['facematch_key'],
      'eventID' =>$incident_id,
      'ID'=>extractTag($image_url),
      'errorString'=>'',
      'webServiceMilliseconds'=>'',
      'imageCoreMilliseconds'=>'',
      'comMilliseconds'=>'',
      'urlFetchMilliseconds'=>'',
      'requestsOutstanding'=>''];
    fm_log("FaceMatch Remove: request=".print_r($params,true));
    $wsdl = $conf['facematch_service'].'?wsdl';
    try {
      $time0 = microtime(true);
      $client = @new SoapClient($wsdl);
      $time1 = microtime(true);
      $result = $client->remove($params);
      $time2 = microtime(true);
    } catch (SoapFault $fault) {
      $error = "FaceMatch Remove: SOAP fault: faultcode: {$fault->faultcode}, faultstring: {$fault->faultstring}";
      fm_log($error, true);
      throw new RuntimeException($error);
    }
    fm_log("FaceMatch Remove: result=".print_r($result, 1));
    if ($result->errorString != 'SUCCESS') {
       $error = "FaceMatch Remove: {$result->errorString}";
       fm_log($error, true);
       throw new RuntimeException($error);
    }
    $setup = round(($time1-$time0), 2);
    $call = round(($time2-$time1),2);
    $total = round(($time2-$time0),2);
    // Update FM stats.
    fm_db_log(REMOVE_SERVICE, $source, $incident_id, $total);
    // Log to file.
    fm_log("FaceMatch Remove: setup=$setup secs, call=$call secs");
  }
}

// Add an extent to FM for the specified event ID.
function taupo_facematch_add_event($incident_id) {
  global $conf;
  if (!isset($conf['facematch_version']) || $conf['facematch_version'] != '2') return;
  $key = urlencode($conf['facematch_key']);
  $url = $conf['facematch_server']."/extent/add?key=$key&performance=optimal&name=$incident_id";
  fm_log("FaceMatch Add Event: request=$url");
  $ch = curl_init();
  curl_setopt($ch, CURLOPT_URL, $url);
  curl_setopt($ch, CURLOPT_PORT, $conf['facematch_port']);
  curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
  $data = curl_exec($ch);
  curl_close($ch);
  if ($data) {
    $decode = json_decode($data);
    fm_log("FaceMatch Add Event: result=".print_r($decode, true));
    if ($decode->statusCode != 1) {
      $message = "FaceMatch Add Event: ".$decode->statusMessage;
      fm_log($message, true);
      throw new RuntimeException($message);
    }
  } else {
    // FaceMatch down.
    $message = "FaceMatch Add Event: No response from FaceMatch!";
    fm_log($message, true);
    throw new RuntimeException($message);
  }
}

/**
 * Have FaceMatch delete the event.
 *
 * @param string $incident_id
 * @return void
 * @throws RuntimeException on FaceMatch error
 */
function taupo_facematch_delete_event($incident_id) {
  global $conf;
  if(isset($conf['facematch_version']) && $conf['facematch_version'] == '2') {
    $key = urlencode($conf['facematch_key']);
    $url = $conf['facematch_server']."/extent/remove?key=$key&name=$incident_id";
    fm_log("FaceMatch Delete Event: request=$url");
    $ch = curl_init();
    curl_setopt($ch, CURLOPT_URL, $url);
    curl_setopt($ch, CURLOPT_PORT, $conf['facematch_port']);
    curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
    $data = curl_exec($ch);
    curl_close($ch);
    if ($data) {
      $decode = json_decode($data);
      fm_log("FaceMatch Delete Event: result=".print_r($decode, true));
      if ($decode->statusCode != 1) {
        $message = "FaceMatch Delete Event: ".$decode->statusMessage;
        fm_log($message, true);
        throw new RuntimeException($message);
      }
    } else {
      // FaceMatch down.
      $message = "FaceMatch Delete Event: No response from FaceMatch!";
      fm_log($message, true);
      throw new RuntimeException($message);
    }
  } else {
    fm_log("FaceMatch Delete Event: Event Id=$incident_id"); 
    $params = ['appKey'=>$conf['facematch_key'],
      'eventID' =>$incident_id,
      'errorString'=>'',
      'webServiceMilliseconds'=>'',
      'comMilliseconds'=>''];
    $wsdl = $conf['facematch_service'].'?wsdl';
    try {
      $time0 = microtime(true);
      $client = @new SoapClient($wsdl);
      $time1 = microtime(true);
      $result = $client->deleteEvent($params);
      $time2 = microtime(true);
    } catch (SoapFault $fault) {
      $error = "FaceMatch Delete Event: SOAP fault: faultcode: {$fault->faultcode}, faultstring: {$fault->faultstring}";
      fm_log($error, true);
      throw new RuntimeException($error);
    }
    fm_log("FaceMatch Delete Event: result=".print_r($result, 1));
    if ($result->errorString != 'SUCCESS') {
      $error = "Delete Event: {$result->errorString}";
      fm_log($error, true);
       throw new RuntimeException($error);
    }
    $setup = round(($time1-$time0), 2);
    $call = round(($time2-$time1),2);
    $total = round(($time2-$time0),2);
    // Log to file.
    fm_log("FaceMatch Delete Event: setup=$setup secs, call=$call secs");
  }
}

// Get a person's selected demographic information from the DB for use in facematch ingest. 
// Returns unknowns or an empty array (if the person doesn't exist). Used by pfif import.
// Note sure of the general usefulness of this, because usually during ingest this information is at hand.
function taupo_facematch_get_attrs($person_id) {
  global $global;
  $sql = "SELECT pd.opt_gender, pd.years_old, pd.minAge, pd.maxAge FROM person_uuid pu, person_details pd" .
        " where pu.p_uuid='".$person_id."'" .
        " and pu.p_uuid = pd.p_uuid";
  $result = $global['db']->Execute($sql);
  if ($result === false) {
    $errchk = $global['db']->ErrorMsg();
    die("Error getting attributes: " . $errchk);
  }
  $attrs = array();
  while ($row = $result->FetchRow()) {
    if ($row['opt_gender'] == 'fml') {
      $attrs['gender'] = 'female';
    } else if ($row['opt_gender'] == 'mal') {
            $attrs['gender'] = 'male';
    } else {
            $attrs['gender'] = 'unknown';
    }
    if ($row['years_old'] != '') {
       $attrs['age'] = $row['years_old'];
    } else if ($row['minAge'] != ''  && $row['maxAge'] != '') {
       $attrs['age'] = floor(($row['minAge']  + $row['maxAge']) / 2);
    } else {
       $attrs['age'] = -1;
    }
  }
  // Note: This is currently only used by import, so no animals.
  $attrs['animal'] = null;
  return $attrs;
}

function fm_log($msg, $error=false) {
  global $conf;

  $fm_log = "/pl/facematch/logs/".$conf['db_name']."_fm.log";
  $fm_error_log = "/pl/facematch/logs/".$conf['db_name']."_fm_error.log";
  if(!file_exists($fm_log) || !file_exists($fm_error_log)) {
    shell_exec('touch '.$fm_log);
    shell_exec('touch '.$fm_error_log);
  }
  $date = date("m-d H:i:s");
  $log = (($error)? $fm_error_log:$fm_log);
  // Don't log errors for staging servers during FMStage maintenance (0:00 - 4:00).
  if ($error && dateIsBetween('today', 'today 4:00') && strpos($conf['db_name'], 'stage') !== false) return;
  error_log("$date $msg\n", 3, $log);
}

// Capture visual search stats.
function fm_db_log($service, $source, $incident_id, $total) {
  global $global;

  if ($source == UNKNOWN_SOURCE) return;
  $q = "INSERT INTO facematch_log (service, source, incident_id, duration, call_time)".
    " VALUES ($service, $source, $incident_id, $total, NOW())";
  $result = $global['db']->Execute($q);
  if ($result === false) {
    $errchk = $global['db']->ErrorMsg();
    fm_log("FaceMatch Stats Error: $errchk ($q)", true);
  }
}

// Check whether FaceMatch is up.
function taupo_facematch_status() {
  global $conf;
  if(!isset($conf['image_search']) || $conf['image_search'] != true) {
   return 1;
  }
  if(isset($conf['facematch_version']) && $conf['facematch_version'] == '2') {
    $status = 0;
    $url = $conf['facematch_server'].'/status';
    $ch = curl_init();
    curl_setopt($ch, CURLOPT_URL, $url);
    curl_setopt($ch, CURLOPT_PORT, $conf['facematch_port']);
    curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
    $data = curl_exec($ch);
    curl_close($ch);
    if ($data) {
      $decode = json_decode($data);
      $status = $decode->statusCode;
      if ($status == 1) {
        fm_log("FaceMatch Status: UP");
      } else {
        fm_log("FaceMatch Status: DOWN", true);
        fm_log("FaceMatch Status: result=".print_r($decode, true));
      }
    } else {
        fm_log("FaceMatch Status: DOWN", true);
    }
    return $status;
  } else {
    $params = ['errorString'=>'', 'serviceOn'=>''];
    $wsdl = isset($conf['facematch_monitor']) ? $conf['facematch_monitor'].'?wsdl' : '';
    // Allow 2 seconds to establish a connection.
    $default = ini_get('default_socket_timeout');
    ini_set('default_socket_timeout', 2);
    try {
      $client = @new SoapClient($wsdl);
      $result = $client->isRunning($params);
    } catch (SoapFault $fault) {
      $error = "FaceMatch Monitor: SOAP fault: faultcode: {$fault->faultcode}, faultstring: {$fault->faultstring}";
      fm_log($error, true);
      ini_set('default_socket_timeout', $default);
      return 0;
    }
    fm_log("FaceMatch Monitor: result=".$result->serviceOn);
    ini_set('default_socket_timeout', $default);
    return $result->serviceOn;
  }  
}

// Check whether FaceMatch logs are clear.
function taupo_facematch_log_status() {
  global $conf;
  $fm_error_log = "/pl/facematch/logs/".$conf['db_name']."_fm_error.log";
  return !(file_exists($fm_error_log) && filesize($fm_error_log) > 0);
}

// Utility function to check whether date is between two other dates.
function dateIsBetween($from, $to, $date = 'now') {
  $date = is_int($date) ? $date : strtotime($date); // convert non timestamps
  $from = is_int($from) ? $from : strtotime($from); // ..
  $to = is_int($to) ? $to : strtotime($to);         // ..
  return ($date > $from) && ($date < $to); // extra parens for clarity
}

// is ip internal
function is_internal($ip) {
  return (substr($ip, 0, 7)  === "130.14."
       || substr($ip, 0, 8)  === "156.40."
       || substr($ip, 0, 13) === "2607:f220:41e");
}

// Extract image tag (look for pattern like: ^puuid__imageid_).
function extractTag ($image_url) {
  preg_match('/(.*?__[^_]+)/', basename($image_url), $match);
  return $match[1];
}
