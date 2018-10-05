<?
/**
 * @name     Event Manager 2
 * @author   pl@miernicki.com
 * @about    Developed by the U.S. National Library of Medicine
 * @link     https://gitlab.com/tehk/people-locator
 * @license  https://gitlab.com/tehk/people-locator/blob/master/LICENSE
 */

$global['use_stream'] = true; // use lib_stream

// xajax
taupo_xajax_init();
require_once($conf['approot'].'mod/em2/xajax.inc');
taupo_xajax_registerFunction('em2_show_events');
taupo_xajax_registerFunction('em2_perform_edit');
taupo_xajax_registerFunction('em2_perform_delete');
taupo_xajax_registerFunction('em2_perform_purge');
taupo_xajax_registerFunction('em2_perform_insert');
taupo_xajax_registerFunction('em2_perform_save');
taupo_xajax_registerFunction('em2_image_send');

function taupo_em2_default() {
  global $global, $conf;
  taupo_xajax_printJavascript();
  echo '<script src="//maps-api-ssl.google.com/maps/api/js?v=3&key='.$conf['gmaps_key'].'"></script>';
  echo '<script>'.file_get_contents($conf['approot'].'mod/em2/em2.js').'</script>';
  // load google analytics
  echo "<script>".GOOGLE_UA."</script>";
?>
<link rel="stylesheet" href="assets/bower_components/jquery-ui/themes/smoothness/jquery-ui.min.css">
<script src="assets/bower_components/jquery-textchange/jquery.textchange.js"></script>
<script src="assets/bower_components/jquery-ui/jquery-ui.min.js"></script>
<style>
.emColumn, .rezColumnTd {
    padding: 0.25em 1em !important;
}
.emta {
          box-sizing: border-box;
     -moz-box-sizing: border-box;
  -webkit-box-sizing: border-box;
  width: 100%;
  border: none;
  height: 300px;
  padding: 10px;
  resize: none;
  margin: 6px 0;
}
.emtb {
  border: 1px solid #ddd !important;
}
.emtd {
          box-sizing: border-box;
     -moz-box-sizing: border-box;
  -webkit-box-sizing: border-box;
  padding: 0 0 0 16px !important;
  height: 294px !important;
  overflow: hidden;
}
tr:hover {
    background-color: #555;
}
</style>
<div id="em2Holder" class="four44">
  <paper-dialog id="deleteOk" class="none" modal entry-animation="scale-up-animation" exit-animation="fade-out-animation">
    <h2>DELETE EVENT CONFIRMATION</h2>
    <paper-dialog-scrollable>
      Are you sure you want to delete this event? This action cannot be undone and is audited.
    </paper-dialog-scrollable>
    <div class="buttons">
      <paper-button dialog-dismiss class="hover" onclick="console.log('delete-decline');">Cancel</paper-button>
      <paper-button dialog-confirm class="hover" onclick="em2_delete_event_ok();">Accept</paper-button>
    </div>
  </paper-dialog>
  <paper-dialog id="purgeOk" class="none" modal entry-animation="scale-up-animation" exit-animation="fade-out-animation">
    <h2>PURGE EVENT CONFIRMATION</h2>
    <paper-dialog-scrollable>
      Are you sure you want to purge this event? This action cannot be undone and is audited.
    </paper-dialog-scrollable>
    <div class="buttons">
      <paper-button dialog-dismiss class="hover" onclick="console.log('purge-decline');">Cancel</paper-button>
      <paper-button dialog-confirm class="hover" onclick="em2_purge_event_ok();">Accept</paper-button>
    </div>
  </paper-dialog>
  <div id="rezControl" class="controlArea">&nbsp;</div>
  <div id="rezMain"    class="mainArea"></div>
</div>
<?
}
