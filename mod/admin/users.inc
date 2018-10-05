<?
/**
 * @name     Admin X users
 * @author   pl@miernicki.com
 * @about    Developed by the U.S. National Library of Medicine
 * @link     https://gitlab.com/tehk/people-locator
 * @license  https://gitlab.com/tehk/people-locator/blob/master/LICENSE
 */

 // save user edit
function admin_save_edit($rj) {
  global $global, $conf;
  $errors = 0;
  $changes = 0;
  $msg = '';
  $r = json_decode($rj, true);
  // load previous user data
  $q = "
    SELECT *
    FROM   `users`
    WHERE  `uid` = ".$global['db']->qstr($r['uid'])."
    LIMIT  1;
  ";
  $result = $global['db']->Execute($q);
  if($result === false) { daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "admin save user 1 >> ".$q); }
  $old_uid    = $result->fields['uid'];
  $old_user   = $result->fields['user'];
  $old_gid    = $result->fields['gid'];
  $old_status = $result->fields['status'];
  $save_user   = false;
  $save_pass   = false;
  $save_gid    = false;
  $save_status = false;
  // validate user
  if($r['user'] != $old_user) {
    $q = "
      SELECT count(*)
      FROM   `users`
      WHERE  `user` = ".$global['db']->qstr($r['user']).";
    ";
    $result = $global['db']->Execute($q);
    if($result === false) { daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "admin save user 3 >> ".$q); }
    $row = $result->FetchRow();
    $count = $row['count(*)'];
    if($count > 0 || !validateAddress($r['user'])) {
      $global['xajax_res']->addScript('$("#email").removeClass("valid");');
      $global['xajax_res']->addScript('$("#email").addClass("invalid");');
      $errors++;
      $msg .= 'Invalid email or email address in use by another user. ';
    } else {
      $global['xajax_res']->addScript('$("#email").removeClass("invalid");');
      $global['xajax_res']->addScript('$("#email").addClass("valid");');
      $save_user = true;
    }
  }
  // validate pass
  if($r['pass'] != '') {
    $heyTHATScool = taupo_pass_policy($r['pass'], $r['user']);
    if($heyTHATScool !== true) {
      $global['xajax_res']->addScript('$("#pass").removeClass("valid");');
      $global['xajax_res']->addScript('$("#pass").addClass("invalid");');
      $errors++;
      $msg .= taupo_pass_policy_message($heyTHATScool)." ";
    } else {
      $global['xajax_res']->addScript('$("#pass").removeClass("invalid");');
      $global['xajax_res']->addScript('$("#pass").addClass("valid");');
      $save_pass = true;
    }
  }
  // validate role
  if($r['gid'] != $old_gid) { $save_gid = true; }
  // validate status
  if($r['ustatus'] != $old_status) { $save_status = true; }
  // error
  if($errors > 0) {
    $global['xajax_res']->addScript("toast('".$msg."');");
    $global['xajax_res']->addScript("enableEditButtons();");
  // everything validates
  } else {
    // save user
    if($save_user) {
      $changes++;
      $q = "
        UPDATE `users`
        SET    `user` = ".$global['db']->qstr($r['user'])."
        WHERE  `uid`  = ".$global['db']->qstr($r['uid']).";
      ";
      $result = $global['db']->Execute($q);
      if($result === false) { daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "save user 2 ((".$q."))"); }
    }
    // save pass
    if($save_pass) {
      $changes++;
      taupo_change_pass($r['user'], $r['pass']);
    }
    // save gid
    if($save_gid) {
      $changes++;
      $q = "
        UPDATE `users`
        SET    `gid` = ".$global['db']->qstr($r['gid'])."
        WHERE  `uid` = ".$global['db']->qstr($r['uid']).";
      ";
      $result = $global['db']->Execute($q);
      if($result === false) { daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "save user 4 ((".$q."))"); }
      if($r['gid'] == 1) {
        // message admins
        taupo_message_admins('User '.$r['user'].' was promoted to admin by '.$_SESSION['user'], null, $conf['base_url'].'/users');
        // audit
        taupo_acl_log('USER_MOD', 'user '.$_SESSION['user'].' promoted '.$r['user'].' to admin');
      }
    }
    // save status
    if($save_status) {
      $changes++;
      $q = "
        UPDATE `users`
        SET    `status` = ".$global['db']->qstr($r['ustatus'])."
        WHERE  `uid`    = ".$global['db']->qstr($r['uid']).";
      ";
      $result = $global['db']->Execute($q);
      if($result === false) { daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "save user 5 ((".$q."))"); }
    }
    if($changes > 0) {
      $global['xajax_res']->addScript("toast3('Changes saved.');");
      // admin event log
      taupo_acl_log('USER_MOD', 'user '.$_SESSION['user'].' modified user '.$r['user']);
      // update user timestamp
      $q = "
        UPDATE `users`
        SET    `changed` = ".$global['db']->qstr(time())."
        WHERE  `uid`     = ".$global['db']->qstr($r['uid']).";
      ";
      $result = $global['db']->Execute($q);
      if($result === false) { daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "save user 6 ((".$q."))"); }
    }
    $global['xajax_res']->addScript("closeEdit();");
  }
  return $global['xajax_res']->getXML();
}

// edit user
function admin_edit_user($uid, $md5) {
  global $global, $conf;
  $randpass = '';
  // adduser
  if(((int)$uid === (int)9999) && ((int)$md5 === (int)9999)) {
    $randpass = randomPass();
    $randuser = 'user'.rand(100000,999999);
    // add new user find uid
    if(taupo_add_user($randuser.'@example.com', $randpass)) {
      $q = "
        SELECT *
        FROM   `users`
        ORDER  BY `changed` DESC
        LIMIT  1;
      ";
      $result = $global['db']->Execute($q);
      if($result === false) { daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "admin edit user x >> ".$q); }
      $uid = $result->fields['uid'];
    } else {
      return;
    }
    // admin event log
    taupo_acl_log('USER_NEW', 'user '.$_SESSION['user'].' created a new user');
  }
  $q = "
    SELECT *
    FROM   `users`
    WHERE  `uid` = ".$global['db']->qstr($uid)."
    LIMIT  1;
  ";
  $result = $global['db']->Execute($q);
  if($result === false) { daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "admin edit user 1 >> ".$q); }
  $out = '
    <h1>Edit User</h1>
    <input class="usertext" name="stamp" id="stamp"  value="'.date('Y-m-d H:i:s', $result->fields['changed']).'" READONLY DISABLED> &nbsp; Last Modified<br>
    <input class="usertext" name="uid"   id="uid"    value="'.$result->fields['uid'].'" READONLY DISABLED> &nbsp; User ID<br>
    <input class="usertext" name="user"  id="user"   value="'.$result->fields['user'].'"> &nbsp; Email<br>
    <input class="usertext" name="pass"  id="pass"   value="'.$randpass.'"> &nbsp; Password <i>(enter to change)</i><br>
  ';
  $out .= '<select name="gid" id="gid">';
  $out .= '<option value="1" '.($result->fields['gid'] == 1 ? 'SELECTED' : '').'>Admin</option>';
  $out .= '<option value="2" '.($result->fields['gid'] == 2 ? 'SELECTED' : '').'>User</option>';
  if($result->fields['status'] == 'pending') {
    $active   = '';
    $pending  = 'SELECTED';
    $inactive = '';
  } elseif($result->fields['status'] == 'inactive') {
    $active   = '';
    $pending  = '';
    $inactive = 'SELECTED';
  } else {
    $active   = 'SELECTED';
    $pending  = '';
    $inactive = '';
  }
  $out .= '
    </select> &nbsp; Role<br>
    <select name="status" id="status">
      <option value="active"   '.$active  .'>Active</option>
      <option value="pending"  '.$pending .'>Pending</option>
      <option value="inactive" '.$inactive.'>Inactive</option>
    </select> &nbsp; Status<br>
    <input type="hidden" id="uid" name="pid" value="'.$uid.'">
  ';
  $out .= '<div style="clear:both"></div>';
  $out .= '<div id="editButtons">';
  $out .= '<paper-button raised id="editUserSave"   class="buttonEM bgGreen"  onclick="saveUser();">Save Changes</paper-button>';
  $out .= '<paper-button raised id="editUserCancel" class="buttonEM bgYellow" onclick="closeEdit();">Cancel</paper-button>';
  $out .= '<paper-button raised id="editUserDelete" class="buttonEM bgRed"    onclick="delUser(\''.$uid.'\');">Delete User</paper-button>';
  $out .= '</div>';
  $global['xajax_res']->addAssign('editOneUser', 'innerHTML', $out);
  return $global['xajax_res']->getXML();
}

// show users
function admin_show_users($term = '', $show=0, $limit=true) {
  global $global, $conf;
  $out = '';
  if($term == '') { $and = ''; }
  else { $and = "AND `user` LIKE ".$global['db']->qstr("%$term%"); }
  if($show == 2) { $and2 = "AND `gid` = 1"; }
  elseif($show == 3) { $and2 = "AND `status` = 'inactive'"; }
  elseif($show == 1) { $and2 = "AND `status` = 'pending'"; }
  else { $and2 = ''; }
  if($limit == 1 || $limit == 'true') { $limiter = 'LIMIT 100;'; }
  else { $limiter = ';'; }
  $q = "
    SELECT *
    FROM   `users`
    WHERE  `uid` != '3'
    ".$and."
    ".$and2."
    ORDER BY changed DESC
    ".$limiter."
  ";
  $result = $global['db']->Execute($q);
  if($result === false) { daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "admin deluser 1"); }
  $tr = '<thead><tr class="thr"><th>&nbsp;</th><th>User</th><th>Modified &nbsp; ▼</th><th>Group</th><th class="opake">&nbsp;</th></tr></thead>';
  $count = 1;
  while(!$result == NULL && !$result->EOF) {
    if($result->fields['status'] == 'pending' || $result->fields['status'] == 'inactive') { $pending   = 'class="pending"'; }
    else { $pending   = ''; }
    if((int)$result->fields["gid"] == 1) { $groop = 'Admin'; }
    else { $groop = 'User'; }
    $tr .= '
      <tr '.$pending.' id="id-'.md5($result->fields["uid"]).'">
        <td>&nbsp;'.$count.'</td>
        <td>'.$result->fields["user"].'</td>
        <td>'.date('Y-m-d H:i:s', $result->fields["changed"]).'</td>
        <td>'.$groop.'</td>
        <td class="edituser emColumn" data-md5="id-'.md5($result->fields["uid"]).'"
          data-uid="'.$result->fields["uid"].'"
          onclick="editUser($(this).attr(\'data-uid\'), $(this).attr(\'data-md5\'));">
          <paper-button class="bgGreen">Edit</paper-button>
        </td>
      </tr>
    ';
    $count++;
    $result->MoveNext();
  }
  $out = '<table id="rt" style="border-spacing: 0 5px; border-collapse: separate;">'.$tr.'</table>';
  $global['xajax_res']->addAssign('usersDiv', 'innerHTML', $out);
  return $global['xajax_res']->getXML();
}

// deluser
function admin_del_user_do($uid, $md5) {
  global $global, $conf;
  // count 
  $q = "
    SELECT count(*), user
    FROM   `users`
    WHERE  `uid` = ".$global['db']->qstr((string)$uid).";
  ";
  $result = $global['db']->Execute($q);
  if($result === false) { daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "admin deluser ((".$q."))"); }
  $row = $result->FetchRow();
  $count = (int)$row['count(*)'];
  $user = (string)$row['user'];
  // audit
  if((int)$count !== 1 || (int)$_SESSION['gid'] !== 1) { die(); }
  $q = "
    DELETE
    FROM   `users`
    WHERE  `uid` = ".$global['db']->qstr((string)$uid).";
  ";
  $result = $global['db']->Execute($q);
  if($result === false) { daoErrorLog(__FILE__, __LINE__, __METHOD__, __CLASS__, __FUNCTION__, $global['db']->ErrorMsg(), "admin deluser 3 "); }
  // message all admins
  taupo_message_admins('User '.$user.' was deleted by '.$_SESSION['user'], null, $conf['base_url'].'/users');
  // audit
  taupo_acl_log('USER_DELETE', 'user '.$_SESSION['user'].' deleted user '.$user);
  // show message and remove row from gui
  $global['xajax_res']->addScript("toast3('User deleted.');");
  $global['xajax_res']->addScript("closeEdit();");
  return $global['xajax_res']->getXML();
}
