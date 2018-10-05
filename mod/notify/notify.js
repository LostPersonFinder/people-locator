/**
 * @name    notify
 * @author  pl@miernicki.com
 * @about   Developed by the U.S. National Library of Medicine
 * @link    https://gitlab.com/tehk/people-locator
 * @license	https://gitlab.com/tehk/people-locator/blob/master/LICENSE
 */

function ask_fcm() {
  window.msg  = $('#not-msg' ).val();
  window.goto = $('#not-goto').val();
  $("#fcmOk").removeClass('none');
  document.querySelector('#fcmOk').open();
}

function ask_fcma() {
  window.msg  = $('#not-msg' ).val();
  window.goto = $('#not-goto').val();
  send_fcma();
}

function send_fcm() {
  // toast for a very long time
  $('#toast2').prop('duration', 10000000);
  toast2('Sending notification to all users...');
  // disable button
  $('#askfcm' ).prop('raised', false);
  $('#askfcma').prop('raised', false);
  $('#askfcm' ).addClass('opacityTwenty');
  $('#askfcma').addClass('opacityTwenty');
  $(function() { $("#askfcm").unbind('click'); });
  // push
  notify_send(window.msg, 0, window.goto);
}

function send_fcma() {
  // toast for a very long time
  $('#toast2').prop('duration', 10000000);
  toast2('Sending notification to all ADMIN users...');
  // disable button
  $('#askfcm' ).prop('raised', false);
  $('#askfcma').prop('raised', false);
  $('#askfcm' ).addClass('opacityTwenty');
  $('#askfcma').addClass('opacityTwenty');
  $(function() { $("#askfcm" ).unbind('click'); });
  $(function() { $("#askfcma").unbind('click'); });
  // push
  notify_send(window.msg, 1, window.goto);
}

$(function() { $("#askfcm" ).click(function(e){ ask_fcm();  }); });
$(function() { $("#askfcma").click(function(e){ ask_fcma(); }); });
