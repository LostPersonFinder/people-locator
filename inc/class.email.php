<?
/**
 * @name     taupo emailer
 * @author   pl@miernicki.com
 * @about    Developed by the U.S. National Library of Medicine
 * @link     https://gitlab.com/tehk/people-locator
 * @license  https://gitlab.com/tehk/people-locator/blob/master/LICENSE
 */

class email {
  public function __construct() {}
  public function sendMessage($toEmail, $subject, $bodyHTML, $bodyAlt, $email_reply='no-reply@') {
    global $global, $conf;
    $email_user  = null;
    $email_pass  = null;
    $email_host  = 'mailfwd.host';
    $email_port  = 25;
    $email_ssl   = null;
    $email_auth  = null;
    require_once($conf['approot'].'3rd/phpmailer/PHPMailerAutoload.php');
    try {
      $bodyAlt  .= "\n\n\n - ".$conf['site_name'];
      if($bodyHTML != null){ $bodyHTML .= '<div style="width: 15em;font-size:24px;color:#2196F3;border-top:1px solid #2196F3;margin:1em 0 1em;padding-top:0.5em;">'.$conf['site_name'].'</div>'; }
      $mail = new PHPMailer(true); // true=enable exceptions
      $mail->CharSet    = 'UTF-8';
      $mail->Encoding   = "base64";
      $mail->IsSMTP();
      $mail->SMTPAuth   = ($email_auth == 1) ? true  : false;
      $mail->Port       = $email_port;
      $mail->Host       = $email_host;
      $mail->Username   = $email_user;
      $mail->Password   = $email_pass;
      $mail->SMTPDebug  = false;
      $mail->SMTPSecure = ($email_ssl == 1) ? 'ssl' : '';
      $mail->AddReplyTo(   $email_reply, $conf['site_name']);
      $mail->From       =  $email_reply;
      $mail->FromName   = $conf['site_name'];
      $mail->AddAddress($toEmail, null);
      $mail->Subject = $subject;
      $mail->WordWrap = 80;
      if($bodyHTML != null) {
        $mail->isHTML(true);
        $mail->MsgHTML($bodyHTML);
        $mail->AltBody = $bodyAlt;
      } else {
        $mail->isHTML(false);
        $mail->Body = $bodyAlt;
      }
      $mail->Send();
      $sendStatus = "SUCCESS";
      $messageLog = "";
    } catch (phpmailerException $e) {
      $sendStatus = "ERROR";
      $messageLog = $e->errorMessage();
    } catch (Exception $e) {
      $sendStatus = "ERROR";
      $messageLog = $e->getMessage();
    }
    $mod = isset($global['module']) ? $global['module'] : '---';
    $q = "
      INSERT INTO emails (
        `mod_accessed`,
        `time_sent`,
        `send_status`,
        `error_message`,
        `email_subject`,
        `email_from`,
        `email_recipients` )
      VALUES (
        '".$mod."',
        CURRENT_TIMESTAMP,
        '".$sendStatus."',
        '".$messageLog."',
        '".$subject."',
        '".$email_reply."',
        '".$toEmail."' ) ;
    ";
    $result = $global['db']->Execute($q);
    if($result === false) { daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $this->db->ErrorMsg(), "email send message ((".$q."))"); }
  }
}
