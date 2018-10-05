/**
 * @name     Admin JS
 * @author   pl@miernicki.com
 * @about    Developed by the U.S. National Library of Medicine
 * @link     https://gitlab.com/tehk/people-locator
 * @license  https://gitlab.com/tehk/people-locator/blob/master/LICENSE
 */

function editUser(uid,md5) {
  enableEditButtons();
  admin_edit_user(uid,md5);
  $('#rollup').fadeTo(300,0);
  $('#rollup').animate({ height: '0' }, 300);
  setTimeout(function() { 
    $('#editOneUser').animate({ height: '500px'}, 300);
    $('#editOneUser').fadeTo(300,1);
  }, 310);
}
function saveUser() {
  disableEditButtons();
  var r = new Object();
  r.uid      = $('#uid'   ).val();
  r.user     = $('#user'  ).val();
  r.pass     = $('#pass'  ).val();
  r.gid      = $('#gid'   ).val();
  r.ustatus  = $('#status').val();
  var rj = JSON.stringify(r);
  admin_save_edit(rj);
  toast2('Saving user...');
}
function refreshUsers() {
  $('#usersDiv').html('loading...');
  var term = $('#userSearchBox').val();
  admin_show_users(term, $('#selector').prop('selected'), $('#limiter').prop('checked'));
}
function closeEdit(){
  $('#editOneUser'   ).fadeTo(300,0);
  $('#editOneUser'   ).animate({ height: '0' }, 300);
  refreshUsers();
  setTimeout(function() { 
    $('#rollup').animate({ height: '100%'}, 300);
    $('#rollup').fadeTo(300,1);
  }, 310);  
}
function delUser(pid) {
  window.deluser_pid = pid;
  $("#deleteUserOk").removeClass('none');
  document.querySelector('#deleteUserOk').open();
}
function delUser2() {
  disableEditButtons();
  toast2('Deleting user...');
  admin_del_user_do(window.deluser_pid, 0);
}

function disableEditButtons() {
  $('#editUserSave'  ).prop('disabled', true);
  $('#editUserCancel').prop('disabled', true);
  $('#editUserDelete').prop('disabled', true);
}
function enableEditButtons() {
  $('#editUserSave'  ).prop('disabled', false);
  $('#editUserCancel').prop('disabled', false);
  $('#editUserDelete').prop('disabled', false);
}
function showAll() {
  var term = $('#userSearchBox').val();
  admin_show_users(term, 0, $('#limiter').prop('checked'));
}
function showPending() {
  var term = $('#userSearchBox').val();
  admin_show_users(term, 1, $('#limiter').prop('checked'));
}
function showInactive() {
  var term = $('#userSearchBox').val();
  admin_show_users(term, 3, $('#limiter').prop('checked'));
}
function showAdmin() {
  var term = $('#userSearchBox').val();
  admin_show_users(term, 2, $('#limiter').prop('checked'));
}
