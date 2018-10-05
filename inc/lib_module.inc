<?
/**
 * @name     module lib
 * @author   pl@miernicki.com
 * @about    Developed by the U.S. National Library of Medicine
 * @link     https://gitlab.com/tehk/people-locator
 * @license  https://gitlab.com/tehk/people-locator/blob/master/LICENSE
 */

// handle rest calls
function handleRest() {
  global $global, $conf;
  if(isset($_GET['mod']) && (string)$_GET['mod'] === 'rest_endpoint') {
    require_once($conf['approot'].'mod/rest/main.inc');
    rest_endpoint();
    die(); // finished
  }
}

// handle batch uploads
function handleBatch() {
  global $global, $conf;
  if(isset($_GET['load']) && $_GET['load'] == 'biupload')    { require_once($conf['approot'].'mod/load/biu.php'); }
  elseif(isset($_GET['load']) && $_GET['load'] == 'breport') { require_once($conf['approot'].'mod/load/br.php');  }
}

// module check
function initModule() {
  global $global, $conf;
  // set default module / action / event
  $global['module'] = isset($_GET['mod']) ? (string)$_GET['mod'] : $conf['default_mod'];
  $global['action'] = isset($_GET['act']) ? (string)$_GET['act'] : 'default';
  // detect invalid php function name characters
  if(!preg_match('/^[a-zA-Z_\x7f-\xff][a-zA-Z0-9_\x7f-\xff]*$/', $global['module']) ||
     !preg_match('/^[a-zA-Z_\x7f-\xff][a-zA-Z0-9_\x7f-\xff]*$/', $global['action'])) {
    $global['module'] = $conf['default_mod'];
    $global['action'] = 'default';
  }
  $gm = $global['module'];
  // invalid module permission
  if(!is_array($conf['acl'][$gm]) || !array_key_exists($gm, $conf['acl']) || !in_array($_SESSION['gid'], $conf['acl'][$gm])) {
    $global['module'] = $conf['default_mod'];
    $global['action'] = 'default';
    $global['toast']  = 'moduleAccess';
  }
  // load module
  require_once($conf['approot'].'mod/'.$global['module'].'/main.inc');
}
