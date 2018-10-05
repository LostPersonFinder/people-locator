<?
/**
 * @name     cache library
 * @author   pl@miernicki.com
 * @about    Developed by the U.S. National Library of Medicine
 * @link     https://gitlab.com/tehk/people-locator
 * @license  https://gitlab.com/tehk/people-locator/blob/master/LICENSE
 */

use phpFastCache\CacheManager;
use phpFastCache\Core\phpFastCache;

// start fast cache
function initCache() {
  global $global, $conf;
  // init cache
  require_once($conf['approot'].'3rd/phpfastcache/src/autoload.php');
  // set cache path
  CacheManager::setDefaultConfig(["path" => $conf['approot'].'www/tmp/fast_cache']);
  // globalize cache
  $global['cache'] = CacheManager::getInstance('sqlite');
}
