<?
/**
 * @name     QUICK
 * @author   pl@miernicki.com
 * @about    Developed by the U.S. National Library of Medicine
 * @link     https://gitlab.com/tehk/people-locator
 * @license  https://gitlab.com/tehk/people-locator/blob/master/LICENSE
 */


global $global, $conf;
// register accessible xajax funtions
$global['xajax_functions'] = array();
array_push($global['xajax_functions'], 'quick_list');
require_once($conf['approot'].'mod/quick/main.inc');

// quick list
function quick_list($term) {
  global $global, $conf;
  $out = '<table><tr><td>Record Link</td><td>Event</td></tr>';
  $term2 = preg_replace("/[^A-Za-z]/", '', $term);
  $q = "
    select *
    from person_uuid p, incident i
    where p.full_name like '%".$term2."%'
    and i.incident_id = p.incident_id
    and (p.expiry_date > NOW() or p.expiry_date is NULL)
    limit 100;
  ";
  $result = $global['db']->Execute($q);
  if($result === false) { daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "quick list ((".$q."))"); }
  while($row = $result->FetchRow() ){
    $url = makeRecordUrl($row['p_uuid']);
    $out .= '<tr><td><a target="_blank" href="'.$url.'">'.$url.'</a></td><td>'.$row['shortname'].'</td></tr>';
  }
  $out .= "</table>";
  $global['xajax_res']->addAssign('list', 'innerHTML', $out);
  return $global['xajax_res']->getXML();
}
