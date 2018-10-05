<?
/**
 * @name     Taupo Database Library
 * @author   pl@miernicki.com
 * @about    Developed by the U.S. National Library of Medicine
 * @link     https://gitlab.com/tehk/people-locator
 * @license  https://gitlab.com/tehk/people-locator/blob/master/LICENSE
 */

global $global, $conf;
require_once($conf['approot'].'3rd/adodb/adodb.inc.php');
require_once($conf['approot'].'3rd/adodb/session/crypt.inc.php');
$dsn = 'mysqli://'.$conf['db_user'].':'.$conf['db_pass'].'@'.$conf['db_host'].'/'.$conf['db_name'].'?persist&port='.$conf['db_port']; 
$global['db'] = ADONewConnection($dsn);
$global['db']->EXECUTE("set names 'utf8'");

// performance monitoring
/*
  $global['perf'] = NewPerfMonitor($global['db']);
  $global['perf']->CreateLogTable();
  $global['db']->logSql(true);
*/
