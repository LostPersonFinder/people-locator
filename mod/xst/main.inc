<?
/**
 * @name     XAJAX Steamer
 * @author   pl@miernicki.com
 * @about    Developed by the U.S. National Library of Medicine
 * @link     https://gitlab.com/tehk/people-locator
 * @license  https://gitlab.com/tehk/people-locator/blob/master/LICENSE
 */

// 404
function taupo_xst_default() {
  http_response_code(404);
  echo '404';
  die();
}
function taupo_ajax_xst_xajax(){} // streamer
