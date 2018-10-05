<?
/**
 * @name     xajax lib
 * @author   pl@miernicki.com
 * @about    Developed by the U.S. National Library of Medicine
 * @link     https://gitlab.com/tehk/people-locator
 * @license  https://gitlab.com/tehk/people-locator/blob/master/LICENSE
 */

// init xajax
function taupo_xajax_init() {
  global $global, $conf;
  require_once($conf['approot']."3rd/xajax/xajax.inc.php");
  $global['xajax'] = new xajax('index.php?stream=ajax&mod=xst&act=xajax','');
}

// add function to global js space
function taupo_xajax_registerFunction($name){
  global $global, $conf;
  $global['xajax']->registerFunction($name);
}

// called in main.inc to empty js space to client
function taupo_xajax_printJavascript(){
  global $global, $conf;
  static $i; // only allow to be called once
  if($i === 0 || $i === null) {
    $i = 0;
    $sRequestURI='index.php?stream=ajax&mod=xst&act=xajax';
    $global['xajax']->setRequestURI($sRequestURI);
    $global['xajax']->printJavascript("res/js");
    $i++;
  }
}

// ajax stream start
function taupo_ajax_start() {
  global $global, $conf;
  $global['xajax']           = new xajax();
  $global['xajax_res']       = new xajaxResponse();
  $global['xajax_functions'] = array();
}

// ajax stream end
function taupo_ajax_end() {
  global $global, $conf;
  if(!isset($_REQUEST['xajax'])) { http_response_code(404); die(); } // die on bad request
  $module = explode('_',$_REQUEST['xajax']);
  $module = $module[0];
  $module = str_replace('/','_', $module);
  if(file_exists($conf['approot'].'mod/'.$module.'/xajax.inc')) {
    require_once($conf['approot'].'mod/'.$module.'/xajax.inc');
    foreach($global['xajax_functions'] as $function) { $global['xajax']->registerFunction($function); }
  } else { http_response_code(404); die(); } // bad request
  $global['xajax']->processRequests();
}
