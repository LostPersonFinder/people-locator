<?
/**
 * @name     include lib
 * @author   pl@miernicki.com
 * @about    Developed by the U.S. National Library of Medicine
 * @link     https://gitlab.com/tehk/people-locator
 * @license  https://gitlab.com/tehk/people-locator/blob/master/LICENSE
 */
 
// load libraries
require_once($conf['approot'].'inc/lib_acl.inc');
require_once($conf['approot'].'inc/lib_cache.inc');
require_once($conf['approot'].'inc/lib_constants.inc');
require_once($conf['approot'].'inc/lib_database.inc');
require_once($conf['approot'].'inc/lib_debug.inc');
require_once($conf['approot'].'inc/lib_image.inc');
require_once($conf['approot'].'inc/lib_fail2ban.inc');
require_once($conf['approot'].'inc/lib_framework.inc');
require_once($conf['approot'].'inc/lib_facematch.inc');
require_once($conf['approot'].'inc/lib_messaging.inc');
require_once($conf['approot'].'inc/lib_module.inc');
require_once($conf['approot'].'inc/lib_session.inc');
require_once($conf['approot'].'inc/lib_solr.inc');
require_once($conf['approot'].'inc/lib_stream.inc');
require_once($conf['approot'].'inc/lib_xajax.inc');
require_once($conf['approot'].'mod/rest/lib.inc');
// load classes
require_once($conf['approot'].'inc/class.person.php');
require_once($conf['approot'].'inc/class.image.php');
require_once($conf['approot'].'inc/class.note.php');
require_once($conf['approot'].'inc/class.email.php');
require_once($conf['approot'].'inc/class.search.php');
// load 3rd
require_once($conf['approot'].'3rd/htmlpurifier/library/HTMLPurifier.auto.php');
