<?
/**
 * @name     people locator main controller
 * @author   pl@miernicki.com
 * @about    Developed by the U.S. National Library of Medicine
 * @link     https://gitlab.com/tehk/people-locator
 * @license  https://gitlab.com/tehk/people-locator/blob/master/LICENSE
 */

// load config; load libs
require_once('../conf/taupo.conf');
require_once('../inc/lib_includes.inc');
initCache();     // start phpFastCache
fail2banCheck(); // check ip ban
cleanPostGet();  // clean post/get vars
handleRest();    // provide rest web services
handleBatch();   // batch uploads handler
initSession();   // php session
initModule();    // begin module
stream();        // stream output
