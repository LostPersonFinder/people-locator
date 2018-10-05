<?
/**
 * @name     QUICK
 * @author   pl@miernicki.com
 * @about    Developed by the U.S. National Library of Medicine
 * @link     https://gitlab.com/tehk/people-locator
 * @license  https://gitlab.com/tehk/people-locator/blob/master/LICENSE
 */

taupo_xajax_init();
require_once($conf['approot'].'mod/quick/xajax.inc');
taupo_xajax_registerFunction('quick_list');

function taupo_quick_default() {
  global $global;
  ?>
  <html>
  <script src="assets/bower_components/jquery/dist/jquery.min.js"></script>
  <style>
    td {
      border: none;
      padding: 5px 20px;
    }
    body {
      margin: 1em;
    }
    #s {
      border: 1px solid #808080;
      padding: 0.5em 2em;
      border-radius: 2px;
      margin: 0 0 1em;
      width: 600px;
    }
  </style>
  <input id="s" value="" autocomplete=off>
  <div id="list"></div>
  <script>
    $("#s").keypress(function() {
      quick_list(document.getElementById("s").value);
    });
  </script>
  </html>
  <?
  taupo_xajax_printJavascript();
}
