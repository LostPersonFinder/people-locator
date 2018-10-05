<?
/**
 * @name     home4
 * @author   pl@miernicki.com
 * @about    Developed by the U.S. National Library of Medicine
 * @link     https://gitlab.com/tehk/people-locator
 * @license  https://gitlab.com/tehk/people-locator/blob/master/LICENSE
 */

function taupo_home4_default() {
  global $global, $conf;
  ob_start("ob_gzhandler");
  $buffer = file_get_contents($conf['approot'].'www/assets/app/a.html');
  $buffer .= "<script>";
  $buffer .= "window.token       = '".$_SESSION['token']."';";
  $buffer .= "window.gval        = '9999';";
  $buffer .= "window.baseUrl     = '".$conf['base_url'] ."';";
  $buffer .= "window.baseUuid    = '".$conf['base_uuid']."';";
  $buffer .= "window.gmapsapikey = '".$conf['gmaps_key']."';";
  $buffer .= "window.cloglevel   = ".(int)$conf['clog']    .";";
  $buffer .= "window.solr        = ".(int)$conf['SOLR_on'] .";";
  $buffer .= "window.i18n_off    =  JSON.parse('".json_encode($conf['i18n_off'])."');";
  $buffer .= "window.i18n_on     =  JSON.parse('".json_encode($conf['i18n_on'] )."');";
  $buffer .= "window.toastError  = '".(isset($global['toast']) ? $global['toast'] : "")."';";
  $buffer .= "</script>";
  echo $buffer;
  ob_end_flush();
}
