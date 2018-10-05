<?
/**
 * @name     notify
 * @author   pl@miernicki.com
 * @about    Developed by the U.S. National Library of Medicine
 * @link     https://gitlab.com/tehk/people-locator
 * @license  https://gitlab.com/tehk/people-locator/blob/master/LICENSE
 */

$global['use_stream'] = true; // use lib_stream
taupo_xajax_init();
require_once($conf['approot'].'mod/notify/xajax.inc');
taupo_xajax_registerFunction('notify_send');

function taupo_notify_default() {
  global $global, $conf;
  taupo_xajax_printJavascript();
  
  if($conf['site_name'] == 'NLM PEOPLE LOCATOR') {
    $icon  = $conf['base_url'].'/assets/images/logo/logo-white-128.png';
    $badge = $conf['base_url'].'/assets/images/logo/logo-white-transparent-128.png';
  } elseif($conf['site_name'] == 'PL STAGE') {
    $icon  = $conf['base_url'].'/assets/images/logo/logo-white-128-stage.png';
    $badge = $conf['base_url'].'/assets/images/logo/logo-white-transparent-128-stage.png';
  } else {
    $icon  = $conf['base_url'].'/assets/images/logo/logo-white-128-dev.png';
    $badge = $conf['base_url'].'/assets/images/logo/logo-white-transparent-128-dev.png';
  }
  
  
?><div class="notifyHolder">
    <center><h1 style="font-size: 1.5em; margin: 0;">New Push Notification</h1></center>
    <div style="background-color: white; color: black; padding: 16px; border-radius: 3px; margin: 16px 0; box-shadow: 0 0 5px 0 #777;">
      <? echo $conf['site_name']; ?>
      <paper-input char-counter class="counter" always-float-label label="Message" name="not-msg" id="not-msg"   maxlength="140" value="An event has been started for the recent earthquake in Ecuador."></paper-input>
      <img src="<? echo $icon; ?>">
      <paper-input char-counter class="counter" always-float-label label="Goto URL" name="not-goto" id="not-goto" maxlength="140" value="<? echo $conf['base_url'].'/'; ?>"></paper-input>
    </div>
    <center>
      <paper-button class="buttonEM bgGreen" style="width: 350px; margin: 16px;" raised id="askfcm" >Send to ALL Users (<? echo get_push_count_all(); ?>)</paper-button>
      <paper-button class="buttonEM bgGreen" style="width: 355px; margin: 16px;" raised id="askfcma">Send to Admins only (<? echo get_push_count_admin(); ?>)</paper-button>
    </center>
    <script><? echo file_get_contents($conf['approot']."mod/notify/notify.js"); ?></script>
    <paper-dialog id="fcmOk" class="none" modal entry-animation="scale-up-animation" exit-animation="fade-out-animation">
      <h2>NOTIFICATION CONFIRMATION</h2>
      <paper-dialog-scrollable>
        Are you sure you want to send a notification to <b>all <? echo get_push_count_all(); ?>' users</b>? 
        This action cannot be undone!
      </paper-dialog-scrollable>
      <div class="buttons">
        <paper-button dialog-dismiss class="hover" onclick="console.log(\'fcm-decline\');">Cancel</paper-button>
        <paper-button dialog-confirm class="hover" onclick="send_fcm();">Accept</paper-button>
      </div>
    </paper-dialog>
  </div>
  <div class="notifyHolder" style="margin-top: 1.5em; margin-bottom: 2em;">
  <center><h1 style="font-size: 1.5em; margin: 0;">Previous Push Notifications (last 100)</h1></center><?
  $q = "SELECT * FROM `notifications` ORDER BY `not_time` DESC LIMIT 100;";
  $result = $global['db']->Execute($q);
  if($result === false) { daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "getEventListUser 2"); }
  while (!$result == NULL && !$result->EOF) {
    echo '<div style="background-color: white; margin: 16px; padding: 16px; border-radius: 3px; color: #555;">';
    echo '<b style="font-size: 120%; color: #333;">'.$result->fields["not_message"].'</b><br>';
    echo '<div style="height: 16px;"></div>';
    echo 'Sent <b>'.date('l jS \of F Y h:i:s A', strtotime($result->fields['not_time'])).'</b> to <b>'.$result->fields['sent_to'].'</b> devices.<br>';
    echo 'Delivered to <b>'.$result->fields['success'].'</b> devices and failed for <b>'.$result->fields['failure'].'</b> devices.<br>';
    echo 'Sent by <b>'.$result->fields['user'].'</b>';
    echo '</div>';
    $result->MoveNext();
  }
  echo '</div>';
}
