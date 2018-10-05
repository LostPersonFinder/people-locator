<?
/**
 * @name     social daily tally
 * @author   pl@miernicki.com
 * @about    Developed by the U.S. National Library of Medicine
 * @link     https://gitlab.com/tehk/people-locator
 * @license  https://gitlab.com/tehk/people-locator/blob/master/LICENSE
 */

function taupo_tally_default() {
  global $global, $conf;
  $q = "
    SELECT *,
    (
      SELECT count(*)
      FROM person_uuid p
      WHERE p.incident_id = i.incident_id 
      AND (expiry_date IS NULL OR expiry_date > CURDATE())
    ) AS unexpired,
    (
      SELECT count(*)
      FROM person_uuid p
      WHERE p.incident_id = i.incident_id 
      AND expiry_date < CURDATE()
    ) AS expired,
    (
      SELECT count(*)
      FROM person_uuid p
      WHERE p.incident_id = i.incident_id
      AND (
        p_uuid LIKE '%nih.gov%'
      )
    ) AS originated
    FROM incident i
    WHERE `private_group` IS NULL
    AND `unlisted` IS FALSE
    ORDER BY date DESC;
  ";
  $count = 0;
  $lan = $out = '';
  $res = $global['db']->Execute($q);
  if($res === false) { daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "tally 1 ((".$q."))"); }
  while($row = $res->FetchRow() ) {
    $decoded  = json_decode($row['translations']);
    $lan = '';
    foreach($decoded as $key => $value) {
      $obj = json_decode(file_get_contents($conf['approot'].'www/assets/locales/'.$key.'.json'));
      $lan .= $obj->thisLanguageDescription->message.' ';
    }
    $out .= '<div style="background-color: #eee; padding: 1em; border-radius: 2px; margin-bottom: 1em; border: 1px solid #ddd;">';
    $out .=   '<a style="color:#ff4081 !important; text-decoration: none;" href="'.$conf['base_url'].'/en/'.$row['shortname'].'"><b>'.$decoded->en->name.'</b></a><br>';
    $out .=   'public records: <b>'.$row['unexpired'].'</b><br>';
    $out .=   'expired records: <b>'.$row['expired'].'</b><br>';
    $out .=   'originated records: <b>'.$row['originated'].'</b><br>';
    $out .=   'translations: '.$lan;
    $out .= '</div>';
    $count++;
  }
  if($count == 0) { $out = 'currently, there are no public events.'; }
  //
  $out = '<body leftmargin="0" topmargin="0" marginwidth="0" marginheight="0"><table width="100%" height="100%" border="0" cellpadding="0" cellspacing="0"><tr><td width="650" valign="top" align="center"><div style="text-align: left; color: black; margin: 1em;">'.$out.'</div></td></tr></table></body>';
  // key required to email
  if(isset($_REQUEST['key']) && $_REQUEST['key'] == $conf['service_key']) {
    $subject  = $conf['site_name']." :: Daily Update :: Internal Use Only";
    $bodyHTML = $out;
    $bodyAlt  = $out;
    $p = new email();
    $p->sendMessage('@', $subject, $bodyHTML, $bodyAlt, 'daily@');
    $s = new email();
    $s->sendMessage('@', $subject, $bodyHTML, $bodyAlt, 'daily@');
  // admin
  } elseif((int)$_SESSION['gid'] === 1) { echo $out; }
  else {
    http_response_code(404);
    echo '404';
    die();
  }
}
